#ifndef _FILE_IO_TXT_HPP_
#define _FILE_IO_TXT_HPP_

#include <stdio.h>  /*  printf, scanf, NULL */
#include <stdlib.h> /*  malloc, free, rand */

// #include "../wami_pv_headers/wami_C_data.h"
#include "wami_C_data.hpp"

/* Print a matrix of RGB values on the console output */
int print_rgb_matrix(rgb_pixel_t *mat,    // input matrix
                     int          n_rows, // number of rows of the input matrix
                     int          n_cols);         // number of columns of the input matrix

/* Write a matrix of short numbers from an ASCII file */
int write_matrix_to_file(unsigned short *data,     // output matrix
                         int             n_rows,   // number of rows of the input matrix
                         int             n_cols,   // number of columns of the input matrix
                         const char *    filename, // name of the input file
                         char *          name);              // name of the matrix

/* Read a matrix of short numbers from an ASCII file */
int read_matrix_from_file(unsigned short *data,   // input matrix
                          int *           n_rows, // number of rows of the input matrix
                          int *           n_cols, // number of columns of the input matrix
                          const char *    filename);  // name of the input file

/* Write a matrix of short numbers to an ASCII file */
int write_rgb_matrix_to_file(rgb_pixel_t *data,     // output matrix
                             int          n_rows,   // number of columns of the output matrix
                             int          n_cols,   // number of columns of the output matrix
                             const char * filename, // name of the output file
                             char *       name);           // name of the matrix

/* Read a matrix of short numbers to an ASCII file, remember to malloc the data before calling the function */
int read_rgb_matrix_from_file(rgb_pixel_t *data,     // input matrix
                              int *        n_rows,   // number of columns of the input matrix
                              int *        n_cols,   // number of columns of the input matrix
                              const char * filename); // name of the input file

/* Write a matrix of float numbers to an ASCII file */
int write_float_matrix_to_file(float *     data,     // output matrix
                               int         n_rows,   // number of columns of the output matrix
                               int         n_cols,   // number of columns of the output matrix
                               const char *filename, // name of the output file
                               char *      name);          // name of the matrix

/* Read a matrix of float numbers to an ASCII file, remember to malloc the data before calling the function */
int read_float_matrix_from_file(float *     data,      // input matrix
                                int *       n_rows,    // number of columns of the input matrix
                                int *       n_cols,    // number of columns of the input matrix
                                const char *filename); // name of the input file

/* Write a matrix of float numbers to an ASCII file */
int write_float_1D_to_file(float *     data,     // output array
                           int         length,   // number of data of the output array
                           const char *filename, // name of the output file
                           char *      name);          // name of the array

/* Read a matrix of float numbers to an ASCII file, remember to malloc the data before calling the function */
int read_float_1D_from_file(float *     data,      // input array
                            int *       length,    // number of data of the input array
                            const char *filename); // name of the input file

/* Compare 2 files, return 0 if same, return 1 if different */
int compareFiles(FILE *file1, FILE *file2);

int print_rgb_matrix(rgb_pixel_t *mat, int n_rows, int n_cols)
{
    int x, y;

    /* Check the data-structure pointer (not NULL) */
    if (!mat)
        return -1;

    /* Check if the size of the input image is correct (non-negative) */
    if (n_cols < 0 || n_rows < 0)
        return -2;

    /* Check if the size of the input image is non-zero */
    if (!n_cols || !n_rows)
        return 0;

    printf("[");
    for (y = 0; y < n_rows; y++) {
        for (x = 0; x < n_cols; x++) {
            printf(" %hu ", mat[y * n_cols + x].r);
            printf(" %hu ", mat[y * n_cols + x].g);
            printf(" %hu ", mat[y * n_cols + x].b);
        }
        printf(y == (n_rows - 1) ? "]\n" : ";\n");
    }

    return 0;
}

int write_matrix_to_file(unsigned short *data, int n_rows, int n_cols, const char *filename, char *name)
{
    FILE *fp = fopen(filename, "w");
    int   i, j;
    int   n = n_rows;
    int   m = n_cols;

    fprintf(fp, "# Float output: %s\n\n", name);
    fprintf(fp, "%5d %5d\n", n, m);

    for (i = 0; i < n; i++) {
        for (j = 0; j < m; j++) {
            fprintf(fp, "%6hu ", data[(i * m) + j]);
        }
        fprintf(fp, "\n");
    }

    fclose(fp);

    return 0;
}

int read_matrix_from_file(unsigned short *data, int *n_rows, int *n_cols, const char *filename)
{
    FILE *fp = fopen(filename, "r");
    int   i, j;
    char  buffer[100];
    char  comment;

    if (!filename) {
        fprintf(stderr, "File name is not valid!\n");
        return -1;
    }

    if (!fp) {
        fprintf(stderr, "File not found: %s\n", filename);
        return -1;
    }

    /* File format:
     *
     * The first lines are just comments (and begin with '#' or '%'); then there
     * is an empty line; then there is the  number of rows and the number of
     * columns (on the same line); finally there is the matrix data.
     *
     * # comments (% is also a valid comment marker)
     * # comments
     * # comments
     *
     * n_rows n_cols
     * data data data data data ...
     * data data data data data ...
     * data data data data data ...
     * data data data data data ...
     * ...
     */

    /* Read comments */
    fscanf(fp, "%c", &comment);
    while (comment == '#' || comment == '%') {
        fgets(buffer, 100, fp);
        fscanf(fp, "%c", &comment);
    }

    /* Read the number of rows */
    int tmp;
    fscanf(fp, "%d", &tmp);
    *n_rows = tmp;

    /* Read the number of columns */
    fscanf(fp, "%d", &tmp);
    *n_cols = tmp;

    fscanf(fp, "\n");

    // data = (unsigned short *)malloc(((*n_rows) * (*n_cols)) * sizeof(unsigned short));

    /* Read the matrix (row by row) */
    for (i = 0; i < *n_rows; i++) {
        for (j = 0; j < *n_cols; j++) {
            unsigned short tmp;
            fscanf(fp, "%hu", &tmp);
            data[(i * (*n_cols)) + j] = (unsigned short)tmp;
        }
    }
    fclose(fp);

    return 0;
}

int write_rgb_matrix_to_file(rgb_pixel_t *data, int n_rows, int n_cols, const char *filename, char *name)
{
    FILE *fp = fopen(filename, "w");
    int   i, j;
    int   n = n_rows;
    int   m = n_cols;

    fprintf(fp, "# RGB output: %s\n\n", name);
    fprintf(fp, "%5d %5d\n", n, m);

    for (i = 0; i < n; i++) {
        for (j = 0; j < m; j++) {
            fprintf(fp, "%6hu", data[(i * m) + j].r);
            fprintf(fp, "%6hu", data[(i * m) + j].g);
            fprintf(fp, "%6hu", data[(i * m) + j].b);
        }
        fprintf(fp, "\n");
    }

    fclose(fp);

    return 0;
}

int read_rgb_matrix_from_file(rgb_pixel_t *data, int *n_rows, int *n_cols, const char *filename)
{
    FILE *fp = fopen(filename, "r");
    int   i, j;
    char  buffer[100];
    char  comment;

    // printf("read_rgb_matrix_from_file: %s\n", filename);

    if (!filename) {
        fprintf(stderr, "File name is not valid!\n");
        return -1;
    }
    // printf("read_rgb_matrix_from_file: [1]\n");

    if (!fp) {
        fprintf(stderr, "File not found: %s\n", filename);
        return -1;
    }
    // printf("read_rgb_matrix_from_file: [2]\n");

    /* File format:
     *
     * The first lines are just comments (and begin with '#' or '%'); then there
     * is an empty line; then there is the  number of rows and the number of
     * columns (on the same line); finally there is the matrix data.
     *
     * # comments (% is also a valid comment marker)
     * # comments
     * # comments
     *
     * n_rows n_cols
     * r g b r g b r g b r g b ... (each number takes 4 characters, pad with space if needed)
     * r g b r g b r g b r g b ...
     * r g b r g b r g b r g b ...
     * ...
     */

    // Read comments
    fscanf(fp, "%c", &comment);
    while (comment == '#' || comment == '%') {
        fgets(buffer, 100, fp);
        fscanf(fp, "%c", &comment);
    }

    // Read the number of rows
    int tmp;
    fscanf(fp, "%d", &tmp);
    *n_rows = tmp;

    // Read the number of columns
    fscanf(fp, "%d", &tmp);
    *n_cols = tmp;

    fscanf(fp, "\n");
    // printf("n_rows: %d, n_cols: %d\n", *n_rows, *n_cols);

    // Read the matrix (row by row)
    for (i = 0; i < *n_rows; i++) {
        for (j = 0; j < *n_cols; j++) {
            unsigned short tmp;
            fscanf(fp, "%hu", &tmp);
            // printf("%4hu", tmp);
            data[(i * (*n_cols)) + j].r = (unsigned short)tmp;
            fscanf(fp, "%hu", &tmp);
            // printf("%4hu", tmp);
            data[(i * (*n_cols)) + j].g = (unsigned short)tmp;
            fscanf(fp, "%hu", &tmp);
            // printf("%4hu", tmp);
            data[(i * (*n_cols)) + j].b = (unsigned short)tmp;
        }
        // printf("\n--------  %d ----------\n", i);
    }
    fclose(fp);

    return 0;
}

int write_float_matrix_to_file(float *data, int n_rows, int n_cols, const char *filename, char *name)
{
    FILE *fp = fopen(filename, "w");
    int   i, j;
    int   n = n_rows;
    int   m = n_cols;

    fprintf(fp, "# Float output: %s\n\n", name);
    fprintf(fp, "%5d %5d\n", n, m);

    for (i = 0; i < n; i++) {
        for (j = 0; j < m; j++) {
            fprintf(fp, "%8f ", data[(i * m) + j]);
        }
        fprintf(fp, "\n");
    }

    fclose(fp);

    return 0;
}

int read_float_matrix_from_file(float *data, int *n_rows, int *n_cols, const char *filename)
{
    FILE *fp = fopen(filename, "r");
    int   i, j;
    char  buffer[100];
    char  comment;
    
    if (!filename) {
        fprintf(stderr, "File name is not valid!\n");
        return -1;
    }
    
    if (!fp) {
        fprintf(stderr, "File not found: %s\n", filename);
        return -1;
    }
    
    /* File format:
     *
     * The first lines are just comments (and begin with '#' or '%'); then there
     * is an empty line; then there is the  number of rows and the number of
     * columns (on the same line); finally there is the matrix data.
     *
     * # comments (% is also a valid comment marker)
     * # comments
     * # comments
     *
     * n_rows n_cols
     * data data data data data ...
     * data data data data data ...
     * data data data data data ...
     * data data data data data ...
     * ...
     */

    /* Read comments */
    fscanf(fp, "%c", &comment);
    while (comment == '#' || comment == '%') {
        fgets(buffer, 100, fp);
        fscanf(fp, "%c", &comment);
    }
    
    /* Read the number of rows */
    int tmp;
    fscanf(fp, "%d", &tmp);
    *n_rows = tmp;
    
    /* Read the number of columns */
    fscanf(fp, "%d", &tmp);
    *n_cols = tmp;

    fscanf(fp, "\n");
    
    //    *data = (unsigned short *)malloc(((*n_rows) * (*n_cols)) * sizeof(unsigned short));
    /* Read the matrix (row by row) */
    for (i = 0; i < *n_rows; i++) {
        for (j = 0; j < *n_cols; j++) {
            float tmp;
            fscanf(fp, "%f", &tmp);
            data[(i * (*n_cols)) + j] = tmp;
        }
    }
    fclose(fp);

    return 0;
}

int write_float_1D_to_file(float *data, int length, const char *filename, char *name)
{
    FILE *fp = fopen(filename, "w");
    int   i;
    int   n = length;

    fprintf(fp, "# Float output: %s\n\n", name);
    fprintf(fp, "%5d\n", n);

    for (i = 0; i < n; i++) {
        fprintf(fp, "%8f ", data[i]);
    }

    fclose(fp);

    return 0;
}

int read_float_1D_from_file(float *data, int *length, const char *filename)
{
    FILE *fp = fopen(filename, "r");
    int   i, j;
    char  buffer[100];
    char  comment;

    if (!filename) {
        fprintf(stderr, "File name is not valid!\n");
        return -1;
    }

    if (!fp) {
        fprintf(stderr, "File not found: %s\n", filename);
        return -1;
    }

    /* File format:
     *
     * The first lines are just comments (and begin with '#' or '%'); then there
     * is an empty line; then there is the  number of rows and the number of
     * columns (on the same line); finally there is the matrix data.
     *
     * # comments (% is also a valid comment marker)
     * # comments
     * # comments
     *
     * length
     * data data data data data ...
     */

    /* Read comments */
    fscanf(fp, "%c", &comment);
    while (comment == '#' || comment == '%') {
        fgets(buffer, 100, fp);
        fscanf(fp, "%c", &comment);
    }

    /* Read the number of length */
    int tmp;
    fscanf(fp, "%d", &tmp);
    *length = tmp;

    fscanf(fp, "\n");

    //    *data = (unsigned short *)malloc(((*n_rows) * (*n_cols)) * sizeof(unsigned short));
    /* Read the matrix (row by row) */
    for (i = 0; i < *length; i++) {
        float tmp;
        fscanf(fp, "%f", &tmp);
        data[i] = tmp;
    }
    fclose(fp);

    return 0;
}

int compareFiles(FILE *file1, FILE *file2)
{
    char ch1  = getc(file1);
    char ch2  = getc(file2);
    int  err  = 0;
    int  pos  = 0;
    int  line = 1;
    while (ch1 != EOF && ch2 != EOF) {
        pos++;
        if (ch1 == '\n' && ch2 == '\n') {
            line++;
            pos = 0;
        }
        if (ch1 != ch2) {
            err++;
        }
        ch1 = getc(file1);
        ch2 = getc(file2);
    }
    return err;
}

#endif // _FILE_IO_TXT_HPP_
