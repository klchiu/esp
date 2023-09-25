#ifndef _WAMI_C_DATA_H_
#define _WAMI_C_DATA_H_

/* == unsigned integer (8, 16, 32 bits) ================================ */
typedef unsigned char  uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int   uint32_t;

/* == signed integer (8, 16, 32 bits) ================================ */
/* typedef char  int8_t; TODO: defined in some linux header file */
typedef short int16_t;
typedef int   int32_t;

/* == floating / fixed point =========================================== */
typedef float  flt_pixel_t;
typedef double dbl_pixel_t;

/* == pixel ============================================================ */
typedef struct __rgb_pixel {
    uint16_t r, g, b;
} rgb_pixel_t;

#endif /* _WAMI_C_DATA_H_ */
