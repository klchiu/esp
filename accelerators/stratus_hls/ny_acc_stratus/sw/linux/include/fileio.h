#ifndef _FILEIO_H_
#define _FILEIO_H_

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "wami_params.h"
#include "wami_typedef.h"

int integrated_binary_start(const char *filename, uint32_t srcRows, uint32_t srcCols, uint32_t padding,
                            fltPixel_t *start_image, uint32_t nModels, float *mu, float *sigma, float *weight,
                            uint32_t nTestImages);

int integrated_binary_add_image(const char *filename, uint32_t srcRows, uint32_t srcCols, uint32_t padding,
                                uint16_t *image, uint8_t *result);

int integrated_binary_read(const char *filename, uint32_t *srcRows, uint32_t *srcCols, uint32_t *padding,
                           uint32_t *nRows, uint32_t *nCols, fltPixel_t **start_image, uint32_t *nModels, float **mu,
                           float **sigma, float **weight, uint32_t *nTestImages, uint16_t **images, uint8_t **results);

void write_binary_file(void *data, size_t size, size_t count, const char *filename);

void read_binary_file(void *data, const char *filename, size_t num_bytes);

void read_image_file(void *data, const char *filename, size_t num_bytes);

/* FILE FORMAT
 *
 * uint32_t                             srcRows
 * uint32_t                             srcCols
 * uint32_t                             padding
 * uint32_t                             nRows
 * uint32_t                             nCols
 * flt_pixel_t [nRows x nCols]           start_image (debayed, luma, registered, GMMed)
 * uint32_t                             nModels
 * float      [nRows x nCols x nModels] mu
 * float      [nRows x nCols x nModels] sigma
 * float      [nRows x nCols x nModels] weight
 * uint32_t                             nTestImages
 * [nTestImages]
 * uint16_t   [srcRows x srcCols]  Test Images (bayer filtered)
 * uint8_t    [srcRows x srcCols]  'correct' result
 */

int integrated_binary_start(const char *filename, uint32_t srcRows, uint32_t srcCols, uint32_t padding,
                            flt_pixel_t *start_image, uint32_t nModels, float *mu, float *sigma, float *weight,
                            uint32_t nTestImages)
{
    uint32_t nRows, nCols;

    FILE *fp = fopen(filename, "w");

    if (!fp) {
        return -1;
    }

    fwrite(&srcRows, sizeof(uint32_t), 1, fp);
    fwrite(&srcCols, sizeof(uint32_t), 1, fp);
    fwrite(&padding, sizeof(uint32_t), 1, fp);

    nRows = srcRows - 2 * padding;
    nCols = srcCols - 2 * padding;

    fwrite(&nRows, sizeof(uint32_t), 1, fp);
    fwrite(&nCols, sizeof(uint32_t), 1, fp);
    fwrite(start_image, sizeof(flt_pixel_t), nRows * nCols, fp);
    fwrite(&nModels, sizeof(uint32_t), 1, fp);
    fwrite(mu, sizeof(float), nRows * nCols * nModels, fp);
    fwrite(sigma, sizeof(float), nRows * nCols * nModels, fp);
    fwrite(weight, sizeof(float), nRows * nCols * nModels, fp);
    fwrite(&nTestImages, sizeof(uint32_t), 1, fp);

    fclose(fp);
    return 0;
}

int integrated_binary_add_image(const char *filename, uint32_t srcRows, uint32_t srcCols, uint32_t padding,
                                uint16_t *image, uint8_t *result)
{
    FILE *fp = fopen(filename, "a");

    if (!fp) {
        return -1;
    }

    fwrite(image, sizeof(uint16_t), srcRows * srcCols, fp);
    fwrite(result, sizeof(uint8_t), (srcRows - 2 * padding) * (srcCols - 2 * padding), fp);

    fclose(fp);
    return 0;
}

#if 1
void printhex_unsigned(unsigned *a, char *s)
{
    unsigned char *pa = (unsigned char *)a;

    #ifdef __sparc__
    printf("%s0x%02x%02x%02x%02x", s, pa[0], pa[1], pa[2], pa[3]);
    #else  /* __x86__ */
    printf("%s0x%02x%02x%02x%02x", s, pa[3], pa[2], pa[1], pa[0]);
    #endif /* __sparc__ */
}
#endif

int integrated_binary_read(const char *filename, uint32_t *srcRows, uint32_t *srcCols, uint32_t *padding,
                           uint32_t *nRows, uint32_t *nCols, flt_pixel_t **start_image, uint32_t *nModels, float **mu,
                           float **sigma, float **weight, uint32_t *nTestImages, uint16_t **images, uint8_t **results)
{
#if 0
  uint32_t n, r, c;
#endif
    int   bytes_read = 0;
    int   i          = 0;
    FILE *fp         = fopen(filename, "r");
#if 0
  unsigned index = 0;
  unsigned word = 0, hi = 0, low = 0;
#endif

    if (!fp) {
        return -1;
    }

    bytes_read += fread(srcRows, sizeof(uint32_t), 1, fp);
    bytes_read += fread(srcCols, sizeof(uint32_t), 1, fp);
    bytes_read += fread(padding, sizeof(uint32_t), 1, fp);
    bytes_read += fread(nRows, sizeof(uint32_t), 1, fp);
    bytes_read += fread(nCols, sizeof(uint32_t), 1, fp);

    if (!bytes_read) {
        return -2;
    }
    bytes_read = 0;

    *start_image = malloc(sizeof(flt_pixel_t) * *nRows * *nCols);

    if (!start_image) {
        return -1;
    }

    bytes_read += fread(*start_image, sizeof(flt_pixel_t), *nRows * *nCols, fp);
    bytes_read += fread(nModels, sizeof(uint32_t), 1, fp);

    if (!bytes_read) {
        return -2;
    }
    bytes_read = 0;

    *mu     = malloc(sizeof(float) * *nRows * *nCols * *nModels);
    *sigma  = malloc(sizeof(float) * *nRows * *nCols * *nModels);
    *weight = malloc(sizeof(float) * *nRows * *nCols * *nModels);

    if (!mu || !sigma || !weight) {
        return -1;
    }

    bytes_read += fread(*mu, sizeof(float), *nRows * *nCols * *nModels, fp);
    bytes_read += fread(*sigma, sizeof(float), *nRows * *nCols * *nModels, fp);
    bytes_read += fread(*weight, sizeof(float), *nRows * *nCols * *nModels, fp);
    bytes_read += fread(nTestImages, sizeof(uint32_t), 1, fp);

    if (!bytes_read) {
        return -2;
    }
    bytes_read = 0;

    *images  = malloc(sizeof(uint16_t) * *srcRows * *srcCols * *nTestImages);
    *results = malloc(sizeof(uint8_t) * *nRows * *nCols * *nTestImages);

    if (!images) {
        return -1;
    }

    for (i = 0; i < *nTestImages; i++) {
        bytes_read += fread((*images) + (i * *srcRows * *srcCols), sizeof(uint16_t), *srcRows * *srcCols, fp);

        bytes_read += fread((*results) + (i * *nRows * *nCols), sizeof(uint8_t), *nRows * *nCols, fp);
    }

    if (!bytes_read) {
        return -2;
    }

    fclose(fp);

    return 0;
}

/* This function is from the TAV Suite 0.2.0 */
void write_binary_file(void *data, size_t size, size_t count, const char *filename)
{
    size_t nwritten = 0;
    FILE  *fp       = NULL;

    assert(data != NULL);
    assert(filename != NULL);

    fp = fopen(filename, "wb");
    if (fp == NULL) {
        fprintf(stderr, "Error: Unable to open input file %s for reading.\n", filename);
        exit(EXIT_FAILURE);
    }

    nwritten = fwrite(data, size, count, fp);
    if (nwritten != count) {
        fprintf(stderr,
                "Error: write failure on %s. "
                "Expected to write %lu bytes, but only wrote %lu.\n",
                filename, count, nwritten);
        fclose(fp);
        exit(EXIT_FAILURE);
    }

    fclose(fp);
}

/* This function is from the TAV Suite 0.2.0 */
void read_binary_file(void *data, const char *filename, size_t num_bytes)
{
    size_t nread = 0;
    FILE  *fp    = NULL;

    assert(data != NULL);
    assert(filename != NULL);

    fp = fopen(filename, "rb");
    if (fp == NULL) {
        fprintf(stderr, "Error: Unable to open input file %s for reading.\n", filename);
        exit(EXIT_FAILURE);
    }

    nread = fread(data, sizeof(char), num_bytes, fp);
    if (nread != num_bytes) {
        fprintf(stderr,
                "Error: read failure on %s. "
                "Expected %lu bytes, but only read %lu.\n",
                filename, num_bytes, nread);
        fclose(fp);
        exit(EXIT_FAILURE);
    }

    fclose(fp);
}

/* This function is from the TAV Suite 0.2.0 */
void read_image_file(void *data, const char *filename, size_t num_bytes)
{
    int      success = 1;
    size_t   nread   = 0, num_bytes_reported_by_header;
    FILE    *fp      = NULL;
    uint16_t width, height, channels, depth;

    assert(data != NULL);
    assert(filename != NULL);

    fp = fopen(filename, "rb");
    if (fp == NULL) {
        fprintf(stderr, "Error: Unable to open input file %s for reading.\n", filename);
        exit(EXIT_FAILURE);
    }

    success &= (fread(&width, sizeof(uint16_t), 1, fp) == 1);
    success &= (fread(&height, sizeof(uint16_t), 1, fp) == 1);
    success &= (fread(&channels, sizeof(uint16_t), 1, fp) == 1);
    success &= (fread(&depth, sizeof(uint16_t), 1, fp) == 1);
    if (!success) {
        fprintf(stderr, "Error: header read failure on %s. ", filename);
        fclose(fp);
        exit(EXIT_FAILURE);
    }

    num_bytes_reported_by_header = (size_t)width * height * channels * depth;
    if (num_bytes_reported_by_header != num_bytes) {
        fprintf(stderr,
                "Error: header dimensions in %s inconsistent with requested "
                "number of bytes to read (width=%u,height=%u,channels=%u,depth=%u) - from header %zu, in file %zu\n",
                filename, width, height, channels, depth, num_bytes_reported_by_header, num_bytes);
        fclose(fp);
        exit(EXIT_FAILURE);
    }

    nread = fread(data, sizeof(char), num_bytes, fp);
    if (nread != num_bytes) {
        fprintf(stderr,
                "Error: read failure on %s. "
                "Expected %lu bytes, but only read %lu.\n",
                filename, num_bytes, nread);
        fclose(fp);
        exit(EXIT_FAILURE);
    }

    fclose(fp);
}

#endif /* _FILEIO_H_ */
