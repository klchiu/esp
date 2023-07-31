#ifndef _WAMI_UTILS_HPP_
#define _WAMI_UTILS_HPP_

#include <stdlib.h>
/* #include "wami_params.h" */
#include "wami_C_data.hpp"

#define MAX_DIR_AND_FILENAME_LEN (1024)

#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif
#if 0
/* appends filename to directory */
void concat_dir_and_filename(
    char dir_and_filename[MAX_DIR_AND_FILENAME_LEN],
    const char *directory,
    const char *filename);

/* reads input data files */
void read_data_file(
    char *data,
    const char *filename,
    const char *directory,
    size_t num_bytes);

/* reads RGB image files with a trivial header */
void read_image_file(
    char *data,
    const char *filename,
    const char *directory,
    size_t num_bytes);

/* writes image files with a trivial header */
void write_image_file(
    char *data,
    const char *filename,
    const char *directory,
    uint16_t width,
    uint16_t height,
    uint16_t channels);

/* error checked memory allocation */
#define XMALLOC(size) xmalloc(size, __FILE__, __LINE__)
void *xmalloc(size_t size, const char *file, int line);

#define FREE_AND_NULL(x) \
    do { \
        if (x) { free(x); } \
        x = NULL; \
    } while (0);
#endif
#endif /* _WAMI_UTILS_HPP_ */
