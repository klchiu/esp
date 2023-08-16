// Copyright (c) 2011-2023 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <my_stringify.h>
#include <nightp2p_stratus.h>
#include <test/test.h>
#include <test/time.h>

#include "cfg_stm.h"
#include "cfg_p2p.h"


#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <byteswap.h>
#include <unistd.h>
#define _byteswap_ushort(p16) bswap_16(p16)
#define _byteswap_ulong(p32)  bswap_32(p32)
#define _byteswap_uint64(p64) bswap_64(p64)
#define MAX_PATH              (1024)
#define __int64 long long
#define _getcwd(a, b) getcwd(a, b)
#define _chdir(a)     chdir(a)

#define DEVNAME "/dev/nightp2p_stratus.0"
#define NAME    "nightp2p_stratus"

// Default command line arguments
#define DEFAULT_NIMAGES       1
#define DEFAULT_NROWS         120
#define DEFAULT_NCOLS         160
#define DEFAULT_NBYTESPP      2
#define DEFAULT_SWAPBYTES     0
#define DEFAULT_DO_DWT        1
#define DEFAULT_INFILE_IS_RAW 0
#define DEFAULT_NBPP_IN       16
#define DEFAULT_NBPP_OUT      10

#define IMAGE_SRC_DIR "."
#define IMAGE_DST_DIR "."
#define MEDIAN_NELEM  (3 * 3)
#define USE_SHIFT

// typedef float          fltPixel_t;
// typedef unsigned short senPixel_t;
// typedef int            algPixel_t;

#define ODD(A)    ((A)&0x01)
#define EVEN(A)   (!ODD(A))
#define ABS(X)    ((X) < 0 ? (-(X)) : (X))
#define MAX(A, B) ((A) > (B) ? (A) : (B))
#define MIN(A, B) ((A) < (B) ? (A) : (B))

#define DBGMSG(MSG)                                         \
    {                                                       \
        fprintf(stderr, "%s, line %d: %s", __FILE__, __LINE__, MSG); \
    }

#define CLIP_INRANGE(LOW, VAL, HIGH) ((VAL) < (LOW) ? (LOW) : ((VAL) > (HIGH) ? (HIGH) : (VAL)))

int dwt53_row_transpose(algPixel_t *data, algPixel_t *data2, int nrows, int ncols)
{
    int i, j, cur;

    for (i = 0; i < nrows; i++) {
        // Predict the odd pixels using linear interpolation of the even pixels
        for (j = 1; j < ncols - 1; j += 2) {
            cur = i * ncols + j;
#ifdef USE_SHIFT
            data[cur] -= (data[cur - 1] + data[cur + 1]) >> 1;
#else
            data[cur] -= (algPixel_t)(0.5 * (data[cur - 1] + data[cur + 1]));
#endif
        }
        // The last odd pixel only has its left neighboring even pixel
        cur = i * ncols + ncols - 1;
        data[cur] -= data[cur - 1];

        // Update the even pixels using the odd pixels
        // to preserve the mean value of the pixels
        for (j = 2; j < ncols; j += 2) {
            cur = i * ncols + j;
#ifdef USE_SHIFT
            data[cur] += (data[cur - 1] + data[cur + 1]) >> 2;
#else
            data[cur] += (algPixel_t)(0.25 * (data[cur - 1] + data[cur + 1]));
#endif
        }
        // The first even pixel only has its right neighboring odd pixel
        cur = i * ncols + 0;
#ifdef USE_SHIFT
        data[cur] += data[cur + 1] >> 1;
#else
        data[cur] += (algPixel_t)(0.5 * data[cur + 1]);
#endif

        // Now rearrange the data putting the low
        // frequency components at the front and the
        // high frequency components at the back,
        // transposing the data at the same time

        for (j = 0; j < ncols / 2; j++) {
            data2[j * nrows + i]               = data[i * ncols + 2 * j];
            data2[(j + ncols / 2) * nrows + i] = data[i * ncols + 2 * j + 1];
        }
    }

    return 0;
}

int dwt53_row_transpose_inverse(algPixel_t *data, algPixel_t *data2, int nrows, int ncols)
{
    int i, j, cur;
    for (i = 0; i < nrows; i++) {
        // Rearrange the data putting the low frequency components at the front
        // and the high frequency components at the back, transposing the data
        // at the same time
        for (j = 0; j < ncols / 2; j++) {
            data2[i * ncols + 2 * j]     = data[j * nrows + i];
            data2[i * ncols + 2 * j + 1] = data[(j + ncols / 2) * nrows + i];
        }

        // Update the even pixels using the odd pixels
        // to preserve the mean value of the pixels
        for (j = 2; j < ncols; j += 2) {
            cur = i * ncols + j;
#ifdef USE_SHIFT
            data2[cur] -= ((data2[cur - 1] + data2[cur + 1]) >> 2);
#else
            data2[cur] -= (algPixel_t)(0.25 * (data2[cur - 1] + data2[cur + 1]));
#endif
        }
        // The first even pixel only has its right neighboring odd pixel
        cur = i * ncols + 0;
#ifdef USE_SHIFT
        data2[cur] -= (data2[cur + 1] >> 1);
#else
        data2[cur] -= (algPixel_t)(0.5 * data2[cur + 1]);
#endif

        // Predict the odd pixels using linear
        // interpolation of the even pixels
        for (j = 1; j < ncols - 1; j += 2) {
            cur = i * ncols + j;
#ifdef USE_SHIFT
            data2[cur] += ((data2[cur - 1] + data2[cur + 1]) >> 1);
#else
            data2[cur] += (algPixel_t)(0.5 * (data2[cur - 1] + data2[cur + 1]));
#endif
        }
        // The last odd pixel only has its left neighboring even pixel
        cur = i * ncols + ncols - 1;
        data2[cur] += data2[cur - 1];
    }

    return 0;
}

int dwt53_inverse(algPixel_t *data, int nrows, int ncols)
{
    int         err   = 0;
    algPixel_t *data2 = (algPixel_t *)calloc(nrows * ncols, sizeof(algPixel_t));
    if (!data2) {
        perror("Could not allocate temp space for dwt53_inverse op");
        return -1;
    }

    err = dwt53_row_transpose_inverse(data, data2, ncols, nrows);
    err = dwt53_row_transpose_inverse(data2, data, nrows, ncols);

    free(data2);

    return err;
}

int dwt53(algPixel_t *data, int nrows, int ncols)
{
    int         err   = 0;
    algPixel_t *data2 = (algPixel_t *)calloc(nrows * ncols, sizeof(algPixel_t));
    if (!data2) {
        fprintf(stderr, "File %s, Line %d, Memory Allocation Error", __FILE__, __LINE__);
        return -1;
    }

    // First do all rows; This function will transpose the data
    // as it performs its final shuffling

    err = dwt53_row_transpose(data, data2, nrows, ncols);

    // We next do all the columns (they are now the rows)

    err = dwt53_row_transpose(data2, data, ncols, nrows);

    free(data2);

    return err;
}

int hist(algPixel_t *streamA, int *h, int nRows, int nCols, int nBpp)
{
    int nBins = 1 << nBpp;
    int nPxls = nRows * nCols;
    int i     = 0;

    if (h == (int *)NULL) {
        fprintf(stderr, "File %s, Line %d, Memory Allocation Error\n", __FILE__, __LINE__);
        return -1;
    }

    memset((void *)h, 0, nBins * sizeof(int));

    for (i = 0; i < nPxls; i++) {
        if (streamA[i] >= nBins) {
            fprintf(stderr, "File %s, Line %d, Range Error in hist() -- using max val ---", __FILE__, __LINE__);
            h[nBins - 1]++;
        } else {
            h[(int)streamA[i]]++;
        }
    }

    return 0;
}

int histEq(algPixel_t *streamA, algPixel_t *out, int *h, int nRows, int nCols, int nInpBpp, int nOutBpp)
{
    int  nOutBins = (1 << nOutBpp);
    int  nInpBins = (1 << nInpBpp);
    int *CDF      = (int *)calloc(nInpBins, sizeof(int));
    int *LUT      = (int *)calloc(nInpBins, sizeof(int));

    if (!(CDF && LUT)) { // Ok to call free() on potentially NULL pointer
        free(CDF);
        free(LUT);
        fprintf(stderr, "File %s, Line %d, Memory Allocation Error\n", __FILE__, __LINE__);
        return -1;
    }

    int CDFmin = INT_MAX;
    int sum    = 0;
    int nPxls  = nRows * nCols;
    int i      = 0;

    for (i = 0; i < nInpBins; i++) {
        sum += h[i];
        CDF[i] = sum;
    }

    for (i = 0; i < nInpBins; i++) {
        CDFmin = MIN(CDFmin, h[i]);
    }

    for (i = 0; i < nInpBins; i++) {
        LUT[i] = ((CDF[i] - CDFmin) * (nOutBins - 1)) / (nPxls - CDFmin);
    }

    for (i = 0; i < nPxls; i++) {
        out[i] = LUT[(int)streamA[i]];
    }

    free(CDF);
    free(LUT);

    return 0;
}

int comparePxls(const void *arg1, const void *arg2)
{
    algPixel_t *p1 = (algPixel_t *)arg1;
    algPixel_t *p2 = (algPixel_t *)arg2;

    if (*p1 < *p2)
        return -1;
    else if (*p1 == *p2)
        return 0;
    else
        return 1;
}

int slowMedian3x3(algPixel_t *src, algPixel_t *dst, int nRows, int nCols)
{
    int         i = 0, j = 0, k = 0;
    int         r = 0, c = 0;
    algPixel_t *pxlList = (algPixel_t *)calloc(MEDIAN_NELEM, sizeof(algPixel_t));

    if (!pxlList) {
        fprintf(stderr, "File %s, Line %d, Memory Allocation Error\n", __FILE__, __LINE__);
        return -1;
    }

    for (r = 1; r < nRows - 1; r++) {
        for (c = 1; c < nCols - 1; c++) {
            k = 0;
            for (i = -1; i <= 1; i++) {
                for (j = -1; j <= 1; j++) {
                    pxlList[k++] = src[(r + i) * nCols + c + j];
                }
            }
            qsort((void *)pxlList, MEDIAN_NELEM, sizeof(algPixel_t), comparePxls);
            // qsort() really not necessary for such a small array,
            // but it works and I don't care about speed right now.
            dst[r * nCols + c] = pxlList[4];
        }
    }
    free(pxlList);
    return 0;
}

int nf(algPixel_t *streamA, algPixel_t *out, int nRows, int nCols)
{
    int err = 0;
    err     = slowMedian3x3(streamA, out, nRows, nCols);

    return err;
}

int readFrame(FILE *fp, void *image, int nPxls, int nBytesPerPxl, bool bSwap)
{
    /* __int64 *p64 = (__int64 *)image; */
    unsigned long * p32       = (unsigned long *)image;
    unsigned short *p16       = (unsigned short *)image;
    int             nPxlsRead = 0;
    int             i         = 0;

    if (fp == (FILE *)NULL) {
        fprintf(stderr, "File %s, Line %d, NULL fp passed to readFrame()\n", __FILE__, __LINE__);
        return -1;
    }

    nPxlsRead = fread(image, nBytesPerPxl, nPxls, fp);

    if (bSwap) {
        for (i = 0; i < nPxlsRead; i++) {
            /* fprintf(stderr, "[%d]: raw-noswap=%hu\n", */
            /*        i, *p16); */

            if (nBytesPerPxl == sizeof(unsigned short)) {
                *p16 = _byteswap_ushort(*p16);
                p16++;
            } else if (nBytesPerPxl == sizeof(unsigned long)) {
                *p32 = _byteswap_ulong(*p32);
                p32++;
            }
            /* else if (nBytesPerPxl == sizeof(unsigned __int64)) */
            /* { */
            /* 	*p64 = _byteswap_uint64(*p64); */
            /* 	p64++; */
            /* } */
        }
    }

    return nPxlsRead;
}

// Supply "." for srcDir if files reside in current working directory

int readImage(void *image, char *srcDir, char *fileName, int nRows, int nCols, int nFrames, int nBytesPerPxl,
              bool bSwap)
{
    char *          origDir   = NULL;
    __int64 *       p64       = (__int64 *)image;
    unsigned long * p32       = (unsigned long *)image;
    unsigned short *p16       = (unsigned short *)image;
    int             nPxlsRead = 0;
    int             i         = 0;

    origDir = _getcwd(NULL, MAX_PATH);
    if (_chdir(srcDir) == -1) {
        fprintf(stderr, "File %s, Line %d, Could not change to directory=%s\n", __FILE__, __LINE__, srcDir);
        return -1;
    }

    FILE *fp = fopen(fileName, "rb");
    if (fp == (FILE *)NULL) {
        fprintf(stderr, "File %s, Line %d, Could not open %s for reading\n", __FILE__, __LINE__, fileName);
        return -2;
    }
    nPxlsRead = fread(image, nBytesPerPxl, nRows * nCols * nFrames, fp);
    fclose(fp);

    if (bSwap) {
        for (i = 0; i < nPxlsRead; i++) {
            if (nBytesPerPxl == sizeof(unsigned short)) {
                *p16 = _byteswap_ushort(*p16);
                p16++;
            } else if (nBytesPerPxl == sizeof(unsigned long)) {
                *p32 = _byteswap_ulong(*p32);
                p32++;
            } else if (nBytesPerPxl == sizeof(unsigned __int64)) {
                *p64 = _byteswap_uint64(*p64);
                p64++;
            }
        }
    }

    _chdir(origDir);
    free(origDir);

    return nPxlsRead;
}

int saveFrame(void *image, char *dstDir, char *baseFileName, int nRows, int nCols, int frameNo, int nBytesPerPxl,
              bool bSwap)
{
    // char *origDir = NULL;
    __int64 *       p64 = (__int64 *)image;
    unsigned long * p32 = (unsigned long *)image;
    unsigned short *p16 = (unsigned short *)image;

    char fullFileName[MAX_PATH];
    int  nPxlsToWrite = nRows * nCols;
    // int err = 0;
    int i = 0;

    // origDir = _getcwd(NULL, MAX_PATH);

    if (_chdir(dstDir) == -1) {
        fprintf(stderr, "File %s, Line %d, Could not change to directory=%s\n", __FILE__, __LINE__, dstDir);
        return -1;
    }

    sprintf(fullFileName, "outputs/out_%dx%d.raw", nCols, nRows);
    if (bSwap) {
        for (i = 0; i < nPxlsToWrite; i++) {
            if (nBytesPerPxl == sizeof(unsigned short)) {
                *p16 = _byteswap_ushort(*p16);
                p16++;
            } else if (nBytesPerPxl == sizeof(unsigned long)) {
                *p32 = _byteswap_ulong(*p32);
                p32++;
            } else if (nBytesPerPxl == sizeof(unsigned __int64)) {
                *p64 = _byteswap_uint64(*p64);
                p64++;
            }
        }
    }
    FILE *fp = fopen(fullFileName, "wb");
    if (fp == (FILE *)NULL) {
        fprintf(stderr, "File %s, Line %d, Failed fopen() on file: %s\n", __FILE__, __LINE__, fullFileName);
        return -1;
    }
    if (fwrite((void *)image, nBytesPerPxl, nPxlsToWrite, fp) != (size_t)nPxlsToWrite) {
        fclose(fp);
        fprintf(stderr, "File %s, Line %d, Failed fwrite() on file: %s\n", __FILE__, __LINE__, fullFileName);
        return -1;
    }
    fclose(fp);
    // err = _chdir(origDir);
    return nPxlsToWrite * nBytesPerPxl;
}

int ensemble_1(algPixel_t *streamA, algPixel_t *out, int nRows, int nCols, int nInpBpp, int nOutBpp)
{
    // NF
    // H
    // HE
    // DWT

    int         err       = 0;
    int         i         = 0;
    int         nHistBins = 1 << nInpBpp;
    int *       h         = (int *)calloc(nHistBins, sizeof(int));
    algPixel_t *wrkBuf1   = (algPixel_t *)calloc(nRows * nCols, sizeof(algPixel_t));
    algPixel_t *wrkBuf2   = (algPixel_t *)calloc(nRows * nCols, sizeof(algPixel_t));

    if (!(h && wrkBuf1 && wrkBuf2)) {
        free(h);
        free(wrkBuf1);
        free(wrkBuf2);
        fprintf(stderr, "File %s, Line %d, Memory Allocation Error\n", __FILE__, __LINE__);
        return -1;
    }

    memcpy(wrkBuf1, streamA, nRows * nCols * sizeof(algPixel_t));
    // memcpy(wrkBuf2, streamA, nRows * nCols * sizeof(algPixel_t));
    memset(wrkBuf2, 0, nRows * nCols * sizeof(algPixel_t));

    FILE *fileInput = fopen("input.txt", "w");
    for (i = 0; i < nRows * nCols; i++) {
        fprintf(fileInput, "%d\n", wrkBuf1[i]);
    }
    fclose(fileInput);

    err          = nf(wrkBuf1, wrkBuf2, nRows, nCols);
    FILE *fileNF = fopen("AfterNF.txt", "w");
    for (i = 0; i < nRows * nCols; i++) {
        fprintf(fileNF, "%d\n", wrkBuf2[i]);
    }
    fclose(fileNF);

    err            = hist(wrkBuf2, h, nRows, nCols, nInpBpp);
    FILE *fileHist = fopen("AfterHist.txt", "w");
    for (i = 0; i < nHistBins; i++) {
        fprintf(fileHist, "%d\n", h[i]);
    }
    fclose(fileHist);

    err              = histEq(wrkBuf2, out, h, nRows, nCols, nInpBpp, nOutBpp);
    FILE *fileHistEq = fopen("AfterHistEq.txt", "w");
    for (i = 0; i < nRows * nCols; i++) {
        fprintf(fileHistEq, "%d\n", out[i]);
    }
    fclose(fileHistEq);

    if (DEFAULT_DO_DWT) {
        err = dwt53(out, nRows, nCols);
    }
    FILE *fileDWT = fopen("AfterDWT.txt", "w");
    for (i = 0; i < nRows * nCols; i++) {
        fprintf(fileDWT, "%d\n", out[i]);
    }
    fclose(fileDWT);

    // FOR TESTING, INVERT BACK TO GET DECENT IMAGE FOR COMPARISON...
    // dwt53_inverse(wrkBuf2, nRows, nCols);

    // memcpy(out, wrkBuf2, nRows * nCols * sizeof(algPixel_t));

    free(wrkBuf2);
    free(wrkBuf1);
    free(h);

    return err;
}

static const char usage_str[] = "usage: ./nightp2p_stratus.exe coherence infile [nimages] [ncols] [nrows] [-v]\n"
                                "  coherence: none|llc-coh-dma|coh-dma|coh\n"
                                "  infile : input file name (includes the path)\n\n"
                                "Optional arguments:.\n"
                                "  nimages : number of images to be processed. Default: 1\n"
                                "  ncols: number of columns of input image. Default: 160\n"
                                "  nrows: number of rows of input image. Default: 120\n\n\n"
                                "The remaining option is only optional for 'test':\n"
                                "  -v: enable verbose output for output-to-gold comparison\n"
                                "Notice:\n"
                                "  The ordering of the argument is important. For now nimages, nrows\n"
                                "  and ncols should either be all present or not at all.\n";

// static int check_gold(int *gold, pixel_t *array, unsigned len, bool verbose)
// {
//     int i;
//     int rtn = 0;
//     for (i = 0; i < len; i++) {
//         if (((int)array[i]) != gold[i]) {
//             if (verbose)
//                 fprintf(stderr, "A[%d]: array=%d; gold=%d\n", i, (int)array[i], gold[i]);
//             rtn++;
//         }
//     }

//     return rtn;
// }

static void init_buf()
{
    // TODO load raw and repeat image in buffer as many times as nimages

    //  ========================  ^
    //  |  in/out image (int)  |  | img_size (in bytes)
    //  ========================  v

    fprintf(stderr, "init buffers\n");

    int             i = 0, j = 0, hbuf_i = 0;
    unsigned        nPxls;
    FILE *          fd = NULL;
    unsigned short *rawBuf;

    // open input file
    if (test_infile_is_raw) {
        if ((fd = fopen(test_infile, "rb")) == (FILE *)NULL) {
            fprintf(stderr, "[ERROR] Could not open %s\n", test_infile);
            exit(1);
        }
    } else {
        if ((fd = fopen(test_infile, "r")) == (FILE *)NULL) {
            fprintf(stderr, "[ERROR] Could not open %s\n", test_infile);
            exit(1);
        }
    }

    // allocate buffers
    nPxls  = test_rows * test_cols;
    rawBuf = (unsigned short *)calloc(nPxls, sizeof(unsigned short));

    if (!rawBuf) {
        free(rawBuf);
        fprintf(stderr, "[ERROR] Could not allocate buffer (in init_buf())\n");
        exit(1);
    }

    if (test_infile_is_raw) {
        // read raw image file
        if (readFrame(fd, rawBuf, nPxls, test_nbytespp, test_swapbytes) != nPxls) {
            fprintf(stderr, "[ERROR] readFrame returns wrong number of pixels\n");
            exit(1);
        }
    } else {
        // read txt image file
        int      i   = 0;
        uint16_t val = 0;

        fscanf(fd, "%hu", &val);
        while (!feof(fd)) {
            rawBuf[i++] = val;
            fscanf(fd, "%hu", &val);
        }

        fclose(fd);
    }

    // store image in accelerator buffer
    // repeat image according to nimages parameter
    hbuf_i = 0;
    for (i = 0; i < test_nimages; i++) {
        for (j = 0; j < nPxls; j++) {
            test_hbuf[hbuf_i]    = (pixel_t)rawBuf[j];
            test_sbuf_in[hbuf_i] = (int)rawBuf[j];
            hbuf_i++;
        }
    }

    free(rawBuf);
    fclose(fd);
}

static void NORETURN usage(void)
{
    fprintf(stderr, "%s", usage_str);
    exit(1);
}

void test_sw() { ensemble_1(inputA, output, test_rows, test_cols, 16, 10); }

void test_stm(token_t *buf, int num_col, int num_row, int test_batch)
{
    cfg_nightNF_stm[0].hw_buf     = buf;
    cfg_nightHist_stm[0].hw_buf   = buf;
    cfg_nightHistEq_stm[0].hw_buf = buf;
    cfg_nightDwt_stm[0].hw_buf    = buf;

    struct nightNF_stratus_access *    tmp1;
    struct nightvision_stratus_access *tmp2;
    tmp1               = (struct nightNF_stratus_access *)cfg_nightNF_stm[0].esp_desc;
    tmp1->cols         = num_col;
    tmp1->rows         = num_row;
    tmp1->nimages      = test_batch;
    tmp1->is_p2p       = 0;
    tmp1->p2p_config_0 = 1;
    tmp2               = (struct nightvision_stratus_access *)cfg_nightHist_stm[0].esp_desc;
    tmp2->cols         = num_col;
    tmp2->rows         = num_row;
    tmp2->nimages      = test_batch;
    tmp2               = (struct nightvision_stratus_access *)cfg_nightHistEq_stm[0].esp_desc;
    tmp2->cols         = num_col;
    tmp2->rows         = num_row;
    tmp2->nimages      = test_batch;
    tmp2               = (struct nightvision_stratus_access *)cfg_nightDwt_stm[0].esp_desc;
    tmp2->cols         = num_col;
    tmp2->rows         = num_row;
    tmp2->nimages      = test_batch;

    gettime(&t_test_1);
    for (i = 0; i < test_batch; i++) {
        esp_run(cfg_nightNF_stm, 1);
        esp_run(cfg_nightHist_stm, 1);
        esp_run(cfg_nightHistEq_stm, 1);
        esp_run(cfg_nightDwt_stm, 1);
    }
    gettime(&t_test_2);

    time_s = ts_subtract(&t_test_1, &t_test_2);

    fprintf(stderr, "[stm] cols: %d\trows: %d\tbatch: %d\ttime: %llu\n", num_col, num_row, test_batch, time_s);
}

void test_mtm(token_t *buf, int num_col, int num_row, int test_batch) {}

void test_p2p(token_t *buf, int num_col, int num_row, int test_batch) {
    cfg_night_p2p[0].hw_buf     = buf;
    cfg_night_p2p[1].hw_buf     = buf;
    // cfg_night_p2p[2].hw_buf     = buf;
    // cfg_night_p2p[3].hw_buf     = buf;
    fprintf(stderr, "-- test_p2p -- \n");
    
    struct nightNF_stratus_access *    tmp1;
    struct nightvision_stratus_access *tmp2;
    // tmp1               = (struct nightNF_stratus_access *)cfg_night_p2p[0].esp_desc;
    // tmp1->cols         = num_col;
    // tmp1->rows         = num_row;
    // tmp1->nimages      = test_batch;
    // tmp1->is_p2p       = 1;
    // tmp1->p2p_config_0 = 2;
    // tmp2               = (struct nightvision_stratus_access *)cfg_night_p2p[1].esp_desc;
    // tmp2->cols         = num_col;
    // tmp2->rows         = num_row;
    // tmp2->nimages      = test_batch;
    tmp2               = (struct nightvision_stratus_access *)cfg_night_p2p[0].esp_desc;
    tmp2->cols         = num_col;
    tmp2->rows         = num_row;
    tmp2->nimages      = test_batch;
    tmp2               = (struct nightvision_stratus_access *)cfg_night_p2p[1].esp_desc;
    tmp2->cols         = num_col;
    tmp2->rows         = num_row;
    tmp2->nimages      = test_batch;

    gettime(&t_test_1);
    for (i = 0; i < test_batch; i++) {
        fprintf(stderr, "-- test_p2p -- debug 1\n");
        esp_run(cfg_night_p2p, 2);
        fprintf(stderr, "-- test_p2p -- debug 2\n");
    }
    gettime(&t_test_2);

    time_s = ts_subtract(&t_test_1, &t_test_2);

    fprintf(stderr, "[p2p] cols: %d\trows: %d\tbatch: %d\ttime: %llu\n", num_col, num_row, test_batch, time_s);
}

/*
 * The app currently has some hard-coded parameters, that should be made configurable.:
 * - do_dwt = false. DWT always disabled. This means that the output of the accelerator
 *   is expected to be unsigned.
 * - in/out images in TXT format only, although the RAW format is supported as well
 * - word bitwidth and related data type, it should be extracted from the HLS config of
 *   the accelerator. NBPP_IN and NBPP_OUT.
 */
int main(int argc, char *argv[])
{
    fprintf(stderr, "=== Helloo from nightp2p\n");
    if (argc < 3 || argc == 5 || argc > 7) {
        usage();

    } else {
        fprintf(stderr, "\nCommand line arguments received:\n");
        fprintf(stderr, "\tcoherence: %s\n", argv[1]);

        test_infile = argv[2];
        fprintf(stderr, "\tinfile: %s\n", test_infile);

        if (argc == 6 || argc == 7) {
            test_nimages = strtol(argv[3], NULL, 10);
            test_cols    = strtol(argv[4], NULL, 10);
            test_rows    = strtol(argv[5], NULL, 10);
            fprintf(stderr, "\tnimages: %u\n", test_nimages);
            fprintf(stderr, "\tncols: %u\n", test_cols);
            fprintf(stderr, "\tnrows: %u\n", test_rows);
        } else {
            test_nimages = DEFAULT_NIMAGES;
            test_rows    = DEFAULT_NROWS;
            test_cols    = DEFAULT_NCOLS;
        }

        if (argc == 4 || argc == 7) {
            if ((strcmp(argv[3], "-v") && argc == 4) || (strcmp(argv[6], "-v") && argc == 7)) {
                usage();
            } else {
                test_verbose = true;
                fprintf(stderr, "\tverbose enabled\n");
            }
        }
        test_nbytespp      = DEFAULT_NBYTESPP;
        test_swapbytes     = DEFAULT_SWAPBYTES;
        test_do_dwt        = DEFAULT_DO_DWT;
        test_infile_is_raw = DEFAULT_INFILE_IS_RAW;
        test_nbpp_in       = DEFAULT_NBPP_IN;
        test_nbpp_out      = DEFAULT_NBPP_OUT;
        fprintf(stderr, "\n");
    }

    // return test_main(&test_info, argv[1], "test");

    inputA = (algPixel_t *)calloc(test_rows * test_cols, sizeof(algPixel_t));
    output = (algPixel_t *)calloc(test_rows * test_cols, sizeof(algPixel_t));

    token_t *buf;
    buf = (token_t *)esp_alloc(5000000); // MEM_ONE_IMAGE_SIZE

    // init_buf();

    // test_sw();
    //test_stm(buf, test_cols, test_rows, 1);
    test_mtm(buf, test_cols, test_rows, 1);
    test_p2p(buf, test_cols, test_rows, 1);


    esp_free(buf);
    free(inputA);
    free(output);

    return 0;
}
