/* Copyright (c) 2011-2023 Columbia University, System Level Design Group */
/* SPDX-License-Identifier: Apache-2.0 */

#include <stdio.h>
#ifndef __riscv
    #include <stdlib.h>
#endif

#include <esp_accelerator.h>
#include <esp_probe.h>
#include <fixed_point.h>

// typedef int16_t token_t;
typedef uint32_t token_t;

static unsigned DMA_WORD_PER_BEAT(unsigned _st)
{
    return (sizeof(void *) / _st);
}

#define SLD_ELTADD 0x072
#define DEV_NAME   "sld,eltadd_vivado"

/* <<--params-->> */
const int32_t matrix_dim_x  = 24;
const int32_t matrix_dim_y  = 8;
int32_t       state_control = 0;

static unsigned in_words_adj;
static unsigned out_words_adj;
static unsigned in_len;
static unsigned out_len;
static unsigned in_size;
static unsigned out_size;
static unsigned out_offset;
static unsigned mem_size;

/* Size of the contiguous chunks for scatter/gather */
#define CHUNK_SHIFT 20
#define CHUNK_SIZE  BIT(CHUNK_SHIFT)
#define NCHUNK(_sz) ((_sz % CHUNK_SIZE == 0) ? (_sz / CHUNK_SIZE) : (_sz / CHUNK_SIZE) + 1)

/* User defined registers */
/* <<--regs-->> */
#define ELTADD_STATE_CONTROL_REG 0x48
#define ELTADD_MATRIX_DIM_Y_REG   0x44
#define ELTADD_MATRIX_DIM_X_REG   0x40

static int validate_buf(token_t *out, token_t *gold)
{
    int      i;
    int      j;
    int      index;
    unsigned errors = 0;

    printf("-- validate_buf() --\n");

    for (i = 0; i < 1; i++) {
        for (j = 0; j < matrix_dim_x * matrix_dim_y; j++) {
            index = i * out_words_adj + j;
            if (gold[index] != out[index]) {
                errors++;
            }
            printf("gold[%d] = %d\tout[%d] = %d\n", index, gold[index], index, out[index]);
        }
    }

    return errors;
}

static void init_buf(token_t *in, token_t *gold)
{
    int i;
    int j;

    token_t *in_1 = in;
    token_t *in_2 = in + matrix_dim_x * matrix_dim_y;

    for (i = 0; i < 1; i++) {
        for (j = 0; j < matrix_dim_x * matrix_dim_y; j++) {
            in_1[i * in_words_adj + j] = (token_t)(j + 2);
        }
        for (j = 0; j < matrix_dim_x * matrix_dim_y; j++) {
            in_2[i * in_words_adj + j] = (token_t)(j + 7);
        }
    }

    for (i = 0; i < 1; i++) {
        for (j = 0; j < matrix_dim_x * matrix_dim_y; j++) {
            gold[i * out_words_adj + j] =
                (token_t)(in_1[i * out_words_adj + j]) + (token_t)(in_2[i * out_words_adj + j]);
        }
    }

    // for (i = 0; i < matrix_dim_x * matrix_dim_y; i++) {
    //     printf("in_1[%d] = %d\n", i, in_1[i]);
    // }

    // for (i = 0; i < matrix_dim_x * matrix_dim_y; i++) {
    //     printf("in_2[%d] = %d\n", i, in_2[i]);
    // }

    // for (i = 0; i < matrix_dim_x * matrix_dim_y; i++) {
    //     printf("gold[%d] = %d\n", i, gold[i]);
    // }
}

int main(int argc, char *argv[])
{
    int                i;
    int                n;
    int                ndev;
    struct esp_device *espdevs;
    struct esp_device  espdevs_tmp;
    struct esp_device *dev;
    unsigned           done;
    unsigned **        ptable;
    token_t *          mem;
    token_t *          gold;
    unsigned           errors = 0;
    unsigned           coherence;

    if (DMA_WORD_PER_BEAT(sizeof(token_t)) == 0) {
        in_words_adj  = matrix_dim_x * matrix_dim_y * 2;
        out_words_adj = matrix_dim_x * matrix_dim_y;
    } else {
        in_words_adj  = round_up(matrix_dim_x * matrix_dim_y * 2, DMA_WORD_PER_BEAT(sizeof(token_t)));
        out_words_adj = round_up(matrix_dim_x * matrix_dim_y, DMA_WORD_PER_BEAT(sizeof(token_t)));
    }
    in_len     = in_words_adj * (1);
    out_len    = out_words_adj * (1);
    in_size    = in_len * sizeof(token_t);
    out_size   = out_len * sizeof(token_t);
    out_offset = in_len;
    mem_size   = (out_offset * sizeof(token_t)) + out_size;

    printf("Baremetal App of Element-wise Mult\n");

    espdevs_tmp.addr = 0x60010000;
    // Search for the device
    printf("Scanning device tree... \n");
    ndev = 1;
    /*
        ndev = probe(&espdevs, VENDOR_SLD, SLD_ELTADD, DEV_NAME);
        if (ndev == 0) {
            printf("eltadd not found\n");
            return 0;
        }
    */
    for (n = 0; n < ndev; n++) {

        printf("**************** %s.%d ****************\n", DEV_NAME, n);

        // dev = &espdevs[n];
        dev = &espdevs_tmp;
        // Check DMA capabilities
        if (ioread32(dev, PT_NCHUNK_MAX_REG) == 0) {
            printf("  -> scatter-gather DMA is disabled. Abort.\n");
            return 0;
        }

        if (ioread32(dev, PT_NCHUNK_MAX_REG) < NCHUNK(mem_size)) {
            printf("  -> Not enough TLB entries available. Abort.\n");
            return 0;
        }

        // Allocate memory
        gold = aligned_malloc(out_size);
        mem  = aligned_malloc(mem_size);
        printf("  memory buffer base-address = %p\n", mem);

        // Alocate and populate page table
        ptable = aligned_malloc(NCHUNK(mem_size) * sizeof(unsigned *));
        for (i = 0; i < NCHUNK(mem_size); i++)
            ptable[i] = (unsigned *)&mem[i * (CHUNK_SIZE / sizeof(token_t))];

        printf("  ptable = %p\n", ptable);
        printf("  nchunk = %lu\n", NCHUNK(mem_size));

#ifndef __riscv
        for (coherence = ACC_COH_NONE; coherence <= ACC_COH_RECALL; coherence++) {
#else
        {
            /* TODO: Restore full test once ESP caches are integrated */
            coherence = ACC_COH_NONE;
#endif
            printf("  --------------------\n");
            printf("  Generate input...\n");
            init_buf(mem, gold);

            printf("Check mem buff (only check the first 16 --------\n");
            for (i = 0; i < 15; i++) {
                printf("mem[%d] = %d\n", i, mem[i]);
            }
            printf("End Check mem buff--------\n");

            // Pass common configuration parameters

            iowrite32(dev, SELECT_REG, ioread32(dev, DEVID_REG));
            iowrite32(dev, COHERENCE_REG, coherence);

#ifndef __sparc
            iowrite32(dev, PT_ADDRESS_REG, (unsigned long long)ptable);
#else
            iowrite32(dev, PT_ADDRESS_REG, (unsigned)ptable);
#endif
            iowrite32(dev, PT_NCHUNK_REG, NCHUNK(mem_size));
            iowrite32(dev, PT_SHIFT_REG, CHUNK_SHIFT);

            // Use the following if input and output data are not allocated at the default offsets
            iowrite32(dev, SRC_OFFSET_REG, 0x0);
            iowrite32(dev, DST_OFFSET_REG, 0x0);

            // Pass accelerator-specific configuration parameters
            /* <<--regs-config-->> */
            iowrite32(dev, ELTADD_MATRIX_DIM_X_REG, matrix_dim_x);
            iowrite32(dev, ELTADD_MATRIX_DIM_Y_REG, matrix_dim_y);

            state_control = 1; // only load
            iowrite32(dev, ELTADD_STATE_CONTROL_REG, state_control);

            // Flush (customize coherence model here)
            // esp_flush(coherence);

            // Start accelerators
            printf("  --> Start 1: only load\n");
            iowrite32(dev, CMD_REG, CMD_MASK_START);

            // Wait for completion
            done = 0;
            while (!done) {
                done = ioread32(dev, STATUS_REG);
                done &= STATUS_MASK_DONE;
                printf(".");
            }
            iowrite32(dev, CMD_REG, 0x0);

            printf("  Done 1: only load\n");

            state_control = 2; // only compute and store
            iowrite32(dev, ELTADD_STATE_CONTROL_REG, state_control);

            // Flush (customize coherence model here)
            // esp_flush(coherence);

            // Start accelerators
            printf("  --> Start 2: only compute and store\n");
            iowrite32(dev, CMD_REG, CMD_MASK_START);

            // Wait for completion
            done = 0;
            while (!done) {
                done = ioread32(dev, STATUS_REG);
                done &= STATUS_MASK_DONE;
                printf(".");
            }
            iowrite32(dev, CMD_REG, 0x0);

            printf("  Done 2: only compute and store\n");

            printf("  validating...\n");

            /* Validation */
            errors = validate_buf(&mem[out_offset], gold);
            if (errors)
                printf("  --> FAIL\n");
            else
                printf("  --> PASS\n");
        }

        printf("Check mem buff--------\n");
        for (i = 0; i < 15; i++) {
            printf("mem[%d] = %d\n", i, mem[i]);
        }
        printf("End Check mem buff--------\n");

        aligned_free(ptable);
        aligned_free(mem);
        aligned_free(gold);
    }

    return 0;
}
