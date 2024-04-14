/* Copyright (c) 2011-2023 Columbia University, System Level Design Group */
/* SPDX-License-Identifier: Apache-2.0 */

#include <stdio.h>
#ifndef __riscv
    #include <stdlib.h>
#endif

#include <esp_accelerator.h>
#include <esp_probe.h>
#include <fixed_point.h>
#include <string.h>

// -- include from lenet-c
#include "lenet.h"
// --

// #include "lenet.h"
#include <stdlib.h>
#include <stdio.h>
// #include <time.h>

#define FILE_TRAIN_IMAGE "train-images-idx3-ubyte"
#define FILE_TRAIN_LABEL "train-labels-idx1-ubyte"
#define FILE_TEST_IMAGE  "t10k-images-idx3-ubyte"
#define FILE_TEST_LABEL  "t10k-labels-idx1-ubyte"
#define LENET_FILE       "model.dat"
#define COUNT_TRAIN      60000
#define COUNT_TEST       1

int read_data(unsigned char (*data)[28][28], unsigned char label[], const int count, const char data_file[],
              const char label_file[])
{
    int i, j, c;

    printf("[humu][read_data]: start data\n");

    // token_t *mem_data = 0xA0020000;
    for (c = 0; c < COUNT_TEST; c++) {
        for (i = 0; i < 28; i++) {
            for (j = 0; j < 28; j++) {
                data[c][i][j] = 77;
                printf("%d, %d, %d", c, i, j);
            }
        }
    }

    printf("[humu][read_data]: finish data\n");

    // token_t *mem_label = 0xA0030000;
    for (c = 0; c < COUNT_TEST; c++) {
        label[c] = 6;
    }

    printf("[humu][read_data]: finish label\n");

    // FILE *fp_image = fopen(data_file, "rb");
    // FILE *fp_label = fopen(label_file, "rb");
    // if (!fp_image || !fp_label)
    //     return 1;
    // fseek(fp_image, 16, SEEK_SET);
    // fseek(fp_label, 8, SEEK_SET);
    // fread(data, sizeof(*data) * count, 1, fp_image);
    // fread(label, count, 1, fp_label);
    // fclose(fp_image);
    // fclose(fp_label);
    return 0;
}

// void training(LeNet5 *lenet, image *train_data, uint8 *train_label, int batch_size, int total_size)
// {
//     for (int i = 0, percent = 0; i <= total_size - batch_size; i += batch_size) {
//         TrainBatch(lenet, train_data + i, train_label + i, batch_size);
//         if (i * 100 / total_size > percent)
//             printf("batchsize:%d\ttrain:%2d%%\n", batch_size, percent = i * 100 / total_size);
//     }
// }

int testing(LeNet5 *lenet, image *test_data, uint8 *test_label, int total_size)
{
    int right = 0, percent = 0;
    for (int i = 0; i < total_size; ++i) {

        printf("[humu][testing]: before Predict(), number: %d\n", i);

        uint8 l = test_label[i];
        int   p = Predict(lenet, test_data[i], 10);
        right += l == p;
        if (i * 100 / total_size > percent)
            printf("test:%2d%%\n", percent = i * 100 / total_size);
    }
    return right;
}

// int save(LeNet5 *lenet, char filename[])
// {
//     FILE *fp = fopen(filename, "wb");
//     if (!fp)
//         return 1;
//     fwrite(lenet, sizeof(LeNet5), 1, fp);
//     fclose(fp);
//     return 0;
// }

int load(LeNet5 *lenet, char filename[])
{
    // token_t *mem_model = 0xA0010000;

    int a, b, c, d;

    for (a = 0; a < INPUT; a++) {
        for (b = 0; b < LAYER1; b++) {
            for (c = 0; c < LENGTH_KERNEL; c++) {
                for (d = 0; d < LENGTH_KERNEL; d++) {
                    lenet->weight0_1[a][b][c][d] = 10;
                }
            }
        }
    }

    for (a = 0; a < LAYER2; a++) {
        for (b = 0; b < LAYER3; b++) {
            for (c = 0; c < LENGTH_KERNEL; c++) {
                for (d = 0; d < LENGTH_KERNEL; d++) {
                    lenet->weight2_3[a][b][c][d] = 32;
                }
            }
        }
    }

    for (a = 0; a < LAYER4; a++) {
        for (b = 0; b < LAYER5; b++) {
            for (c = 0; c < LENGTH_KERNEL; c++) {
                for (d = 0; d < LENGTH_KERNEL; d++) {
                    lenet->weight4_5[a][b][c][d] = 54;
                }
            }
        }
    }

    for (a = 0; a < LAYER5 * LENGTH_FEATURE5 * LENGTH_FEATURE5; a++) {
        for (b = 0; b < OUTPUT; b++) {
            lenet->weight5_6[a][b] = 65;
        }
    }

    for (a = 0; a < LAYER1; a++) {
        lenet->bias0_1[a] = 10;
    }

    for (a = 0; a < LAYER3; a++) {
        lenet->bias2_3[a] = 32;
    }

    for (a = 0; a < LAYER5; a++) {
        lenet->bias4_5[a] = 54;
    }

    for (a = 0; a < OUTPUT; a++) {
        lenet->bias5_6[a] = 65;
    }

    // FILE *fp = fopen(filename, "rb");
    // if (!fp)
    //     return 1;
    // fread(lenet, sizeof(LeNet5), 1, fp);
    // fclose(fp);
    return 0;
}
// ----------------------------------------------------

typedef int32_t token_t;

static unsigned DMA_WORD_PER_BEAT(unsigned _st)
{
    return (sizeof(void *) / _st);
}

#define SLD_GEMM3 0x093
#define DEV_NAME  "sld,gemm3_vivado"

/* <<--params-->> */
const int32_t d3        = 2;
const int32_t d2        = 2;
const int32_t d1        = 2;
const int32_t m1_offset = d1 * d3;
const int32_t m2_offset = m1_offset + d1 * d2;
const int32_t m3_offset = 0;

// for conv2d
const int32_t n_channels         = 2;
const int32_t feature_map_height = 5;
const int32_t feature_map_width  = 5;
const int32_t n_kernels          = 2;
const int32_t kernel_height      = 3;
const int32_t kernel_width       = 3;
const int32_t pad_h              = 1;
const int32_t pad_w              = 1;
const int32_t is_padded          = 1;
const int32_t stride_h           = 1;
const int32_t stride_w           = 1;
const int32_t do_relu            = 0;
const int32_t pool_type          = 0;
const int32_t batch_size         = 1;

// static unsigned in_words_adj;
// static unsigned out_words_adj;
static unsigned in1_len;
static unsigned in2_len;
static unsigned out_len;
static unsigned in1_size;
static unsigned in2_size;
static unsigned out_size;
// static unsigned out_offset;
static unsigned mem_size;

/* Size of the contiguous chunks for scatter/gather */
#define CHUNK_SHIFT 20
#define CHUNK_SIZE  BIT(CHUNK_SHIFT)
#define NCHUNK(_sz) ((_sz % CHUNK_SIZE == 0) ? (_sz / CHUNK_SIZE) : (_sz / CHUNK_SIZE) + 1)

/* User defined registers */
/* <<--regs-->> */
#define GEMM3_M3_OFFSET_REG 0x54
#define GEMM3_D3_REG        0x50
#define GEMM3_D2_REG        0x4c
#define GEMM3_D1_REG        0x48
#define GEMM3_M2_OFFSET_REG 0x44
#define GEMM3_M1_OFFSET_REG 0x40

void check_mem_buf(token_t *mem, int length)
{
    int i;
    for (i = 0; i < length; i++) {
        printf("check mem[%d] = %d\n", i, mem[i]);
    }
}

void print_matrix(int d1, int d2, int *m1_data)
{
    printf("--------------------------------------------\n");
    for (int i = 0; i < d1; i++) {
        for (int j = 0; j < d2; j++) {
            printf("%d, ", m1_data[i * d2 + j]);
        }
        printf("\n");
    }
    printf("--------------------------------------------\n");
}

void init_m1(int d1, int d2, int *m1_data)
{
    for (int i = 0; i < d1; i++) {
        for (int j = 0; j < d2; j++) {
            m1_data[i * d2 + j] = i * d2 + j;
        }
    }

    print_matrix(d1, d2, m1_data);
}

void init_m2(int d2, int d3, int *m2_data)
{
    for (int i = 0; i < d2; i++) {
        for (int j = 0; j < d3; j++) {
            m2_data[i * d3 + j] = i * d3 + j + 100;
        }
    }

    print_matrix(d2, d3, m2_data);
}

void gemm_pv(int d1, int d2, int d3, int *m1_data, int *m2_data, int *m3_data)
{
    for (int i = 0; i < d1; i++) {
        for (int k = 0; k < d3; k++) {
            for (int j = 0; j < d2; j++) {
                m3_data[i * d3 + k] += m1_data[i * d2 + j] * m2_data[j * d3 + k];
                // printf("m1_data[%d] = %d\t", i*d2+j, m1_data[i*d2+j]);
                // printf("m2_data[%d] = %d\t", j*d3+k, m2_data[j*d3+k]);
                // printf("m3_data[%d] = %d\n", i*d3+k, m3_data[i*d3+k]);
            }
        }
    }
}

static int validate_buf(token_t *mem_out, token_t *gold)
{
    int j;
    int errors = 0;

    for (j = 0; j < d1 * d3; j++) {

        if (mem_out[m3_offset + j] != gold[j]) {
            errors++;
            printf("[%d]: outbuff = %d\tgold = %d\n", j, mem_out[m3_offset + j], gold[j]);
        }
    }

    return errors;
}

static void init_buf(token_t *mem_in, token_t *gold, int *m1_data, int *m2_data, int *m3_data)
{
    int i;
    int j;

    for (j = 0; j < d1 * d2; j++) {
        mem_in[m1_offset + j] = (token_t)m1_data[j];
    }

    for (j = 0; j < d2 * d3; j++) {
        // mem_in[m2_offset + j] = (token_t)m2_data[j];
        mem_in[m2_offset + j] = (token_t)(j + 10);
    }

    for (j = 0; j < d1 * d3; j++) {
        gold[j] = (token_t)m3_data[j];
    }
}

int gemm3(int argc, char *argv[])
{
    printf("Baremetal App of Vivado Gemm3\n");

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

    int *m1_data = (int *)aligned_malloc(d1 * d2 * sizeof(int));
    int *m2_data = (int *)aligned_malloc(d2 * d3 * sizeof(int));
    int *m3_data = (int *)aligned_malloc(d1 * d3 * sizeof(int));

    init_m1(d1, d2, m1_data);
    init_m2(d2, d3, m2_data);

    // Calculate golden output
    memset(m3_data, 0, d1 * d3 * sizeof(int));
    // print_matrix(d1, d3, m3_data);
    gemm_pv(d1, d2, d3, m1_data, m2_data, m3_data);
    print_matrix(d1, d3, m3_data);

    // if (DMA_WORD_PER_BEAT(sizeof(token_t)) == 0) {
    //     in_words_adj  = m2_offset;
    //     out_words_adj = m1_offset;
    // } else {
    //     in_words_adj  = round_up(m2_offset, DMA_WORD_PER_BEAT(sizeof(token_t)));
    //     out_words_adj = round_up(m1_offset, DMA_WORD_PER_BEAT(sizeof(token_t)));
    // }
    in1_len  = d1 * d2;
    in2_len  = d2 * d3;
    out_len  = d1 * d3;
    in1_size = in1_len * sizeof(token_t);
    in2_size = in2_len * sizeof(token_t);
    out_size = out_len * sizeof(token_t);
    mem_size = in1_size + in2_size + out_size;

    espdevs_tmp.addr = 0x60010000;
    // Search for the device
    printf("Scanning device tree... \n");
    ndev = 1;

    /*
        ndev = probe(&espdevs, VENDOR_SLD, SLD_GEMM3, DEV_NAME);
        if (ndev == 0) {
            printf("gemm3 not found\n");
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
            init_buf(mem, gold, m1_data, m2_data, m3_data);
            check_mem_buf(mem, d1 * d2 * 3);

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
            iowrite32(dev, GEMM3_M3_OFFSET_REG, m3_offset);
            iowrite32(dev, GEMM3_D3_REG, d3);
            iowrite32(dev, GEMM3_D2_REG, d2);
            iowrite32(dev, GEMM3_D1_REG, d1);
            iowrite32(dev, GEMM3_M2_OFFSET_REG, m2_offset);
            iowrite32(dev, GEMM3_M1_OFFSET_REG, m1_offset);

            // Flush (customize coherence model here)
            esp_flush(coherence);

            // Start accelerators
            printf("  Start...\n");
            iowrite32(dev, CMD_REG, CMD_MASK_START);

            // Wait for completion
            done = 0;
            while (!done) {
                done = ioread32(dev, STATUS_REG);
                done &= STATUS_MASK_DONE;
            }
            iowrite32(dev, CMD_REG, 0x0);

            printf("  Done\n");
            printf("  validating...\n");

            /* Validation */
            errors = validate_buf(mem, gold);
            check_mem_buf(mem, d1 * d2 * 3);

            if (errors) {
                printf("  ... FAIL\n");
            } else {
                printf("  ... PASS\n");
            }
        }
        aligned_free(ptable);
        aligned_free(mem);
        aligned_free(gold);
    }

    aligned_free(m1_data);
    aligned_free(m2_data);
    aligned_free(m3_data);

    return 0;
}

int lenet5(int argc, char *argv[])
{
    printf("[humu]: Start lenet5() !\n");

    // image *train_data  = (image *)calloc(COUNT_TRAIN, sizeof(image));
    // uint8 *train_label = (uint8 *)calloc(COUNT_TRAIN, sizeof(uint8));
    image *test_data  = (image *)aligned_malloc(COUNT_TEST * sizeof(image));
    uint8 *test_label = (uint8 *)aligned_malloc(COUNT_TEST * sizeof(uint8));
    // if (read_data(train_data, train_label, COUNT_TRAIN, FILE_TRAIN_IMAGE, FILE_TRAIN_LABEL)) {
    //     printf("ERROR!!!\nDataset File Not Find!Please Copy Dataset to the Floder Included the exe\n");
    //     free(train_data);
    //     free(train_label);
    //     // system("pause");
    // }
    
#include "test_data.h"
    // if (read_data(test_data, test_label, COUNT_TEST, FILE_TEST_IMAGE, FILE_TEST_LABEL)) {
    //     printf("ERROR!!!\nDataset File Not Find!Please Copy Dataset to the Floder Included the exe\n");
    //     aligned_free(test_data);
    //     aligned_free(test_label);
    //     // system("pause");
    // }

    printf("[humu]: Finish read_data()\n");

    LeNet5 *lenet = (LeNet5 *)aligned_malloc(sizeof(LeNet5));
#include "model_data.h"
    // load(lenet, LENET_FILE);

    printf("[humu]: Finish load()\n");

    // clock_t start     = clock();
    int batches[] = {300};
    // for (int i = 0; i < sizeof(batches) / sizeof(*batches); ++i)
    //     training(lenet, train_data, train_label, batches[i], COUNT_TRAIN);

    int right = testing(lenet, test_data, test_label, COUNT_TEST);

    printf("[humu]: Finish testing()\n");

    printf("%d/%d\n", right, COUNT_TEST);
    // printf("Time:%u\n", (unsigned)(clock() - start));
    // save(lenet, LENET_FILE);
    aligned_free(lenet);
    // free(train_data);
    // free(train_label);
    aligned_free(test_data);
    aligned_free(test_label);
    // system("pause");

    return 0;
}

int main(int argc, char *argv[])
{
    // gemm3(argc, argv);
    lenet5(argc, argv);
    return 0;
}