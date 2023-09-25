#ifndef _WAMI_CONFIG_TB_H_
#define _WAMI_CONFIG_TB_H_

#include "wami_config.h"

#define INPUT_SIZE_TINY    0
#define INPUT_SIZE_132_17  1
#define INPUT_SIZE_SMALL   2
#define INPUT_SIZE_516_17  3
#define INPUT_SIZE_516_33  4
#define INPUT_SIZE_MEDIUM  5
#define INPUT_SIZE_1028_17 6
#define INPUT_SIZE_1028_33 7
#define INPUT_SIZE_LARGE   8

/* static configuration */
#define ITERATIONS 20

/* disagreement after change detection up to 1% of total pixels in image (MxN) */
#define ERROR_PERCENTAGE 0.01f

#if INPUT_SIZE == INPUT_SIZE_TINY
    #define INPUT_IMG_NUM_ROWS 16
    #define INPUT_IMG_NUM_COLS 16
    #define INPUT_NUM_IMGS     4
static const char *input_filename = "./inout/tiny_app_input.bin";

#elif INPUT_SIZE == INPUT_SIZE_132_17
    #define INPUT_IMG_NUM_ROWS 132
    #define INPUT_IMG_NUM_COLS 132
    #define INPUT_NUM_IMGS     17
static const char *input_filename = "./inout/fish_132_17.bin";

#elif INPUT_SIZE == INPUT_SIZE_SMALL
    #define INPUT_IMG_NUM_ROWS 512
    #define INPUT_IMG_NUM_COLS 512
    #define INPUT_NUM_IMGS     5
static const char *input_filename = "./inout/small_app_input.bin";

#elif INPUT_SIZE == INPUT_SIZE_516_17
    #define INPUT_IMG_NUM_ROWS 516
    #define INPUT_IMG_NUM_COLS 516
    #define INPUT_NUM_IMGS     17
static const char *input_filename = "./inout/fish_516_17.bin";

#elif INPUT_SIZE == INPUT_SIZE_516_33
    #define INPUT_IMG_NUM_ROWS 516
    #define INPUT_IMG_NUM_COLS 516
    #define INPUT_NUM_IMGS     33
static const char *input_filename = "./inout/fish_516_33.bin";

#elif INPUT_SIZE == INPUT_SIZE_MEDIUM
    #define INPUT_IMG_NUM_ROWS 1024
    #define INPUT_IMG_NUM_COLS 1024
    #define INPUT_NUM_IMGS     5
static const char *input_filename = "./inout/medium_app_input.bin";

#elif INPUT_SIZE == INPUT_SIZE_1028_17
    #define INPUT_IMG_NUM_ROWS 1028
    #define INPUT_IMG_NUM_COLS 1028
    #define INPUT_NUM_IMGS     17
static const char *input_filename = "../../../../inout/fish_1028_17.bin";

#elif INPUT_SIZE == INPUT_SIZE_1028_33
    #define INPUT_IMG_NUM_ROWS 1028
    #define INPUT_IMG_NUM_COLS 1028
    #define INPUT_NUM_IMGS     33
static const char *input_filename = "../../../../inout/fish_1028_33.bin";

#elif INPUT_SIZE == INPUT_SIZE_LARGE
    #define INPUT_IMG_NUM_ROWS 2048
    #define INPUT_IMG_NUM_COLS 2048
    #define INPUT_NUM_IMGS     5
static const char *input_filename = "./inout/large_app_input.bin";
#else
    #error "Unhandled value for INPUT_SIZE"
#endif

#define MAX_LK_ITER 1

// == define image sizes (in pixels) ==========================================

// Debayer
#define DEBAYER_INPUT_NUM_PXL  (INPUT_IMG_NUM_ROWS * INPUT_IMG_NUM_COLS)
#define DEBAYER_OUTPUT_NUM_PXL ((INPUT_IMG_NUM_ROWS - 2 * PAD) * (INPUT_IMG_NUM_COLS - 2 * PAD))

#define DEBAYER_TOTAL_INPUT_NUM_PXL  INPUT_NUM_IMGS *DEBAYER_INPUT_NUM_PXL
#define DEBAYER_TOTAL_OUTPUT_NUM_PXL DEBAYER_OUTPUT_NUM_PXL

// Grayscale
#define GRAYSCALE_OUTPUT_NUM_PXL       DEBAYER_OUTPUT_NUM_PXL
#define GRAYSCALE_TOTAL_OUTPUT_NUM_PXL GRAYSCALE_OUTPUT_NUM_PXL

// Gradient
#define GRADIENT_OUTPUT_NUM_PXL       DEBAYER_OUTPUT_NUM_PXL
#define GRADIENT_TOTAL_OUTPUT_NUM_PXL GRADIENT_OUTPUT_NUM_PXL

// Warp
#define WARP_OUTPUT_NUM_PXL       DEBAYER_OUTPUT_NUM_PXL
#define WARP_TOTAL_OUTPUT_NUM_PXL WARP_OUTPUT_NUM_PXL

// Subtract
#define SUBTRACT_OUTPUT_NUM_PXL       DEBAYER_OUTPUT_NUM_PXL
#define SUBTRACT_TOTAL_OUTPUT_NUM_PXL SUBTRACT_OUTPUT_NUM_PXL

// Steepest descent
#define STEEPEST_DESCENT_OUTPUT_NUM_PXL       6 * DEBAYER_OUTPUT_NUM_PXL
#define STEEPEST_DESCENT_TOTAL_OUTPUT_NUM_PXL STEEPEST_DESCENT_OUTPUT_NUM_PXL

// Hessian
#define HESSIAN_OUTPUT_NUM_PXL       36
#define HESSIAN_TOTAL_OUTPUT_NUM_PXL HESSIAN_OUTPUT_NUM_PXL

// Invert Gauss Jordan
#define INVERT_GAUSS_JORDAN_OUTPUT_NUM_PXL       36
#define INVERT_GAUSS_JORDAN_TOTAL_OUTPUT_NUM_PXL INVERT_GAUSS_JORDAN_OUTPUT_NUM_PXL

// TODO: the input/output size should be the sum of the operand/result sizes
// SD update
#define SD_UPDATE_OUTPUT_NUM_PXL       6
#define SD_UPDATE_TOTAL_OUTPUT_NUM_PXL SD_UPDATE_OUTPUT_NUM_PXL

// Mult
#define MULT_OUTPUT_NUM_PXL       6
#define MULT_TOTAL_OUTPUT_NUM_PXL MULT_OUTPUT_NUM_PXL

// Reshape
#define RESHAPE_OUTPUT_NUM_PXL       6
#define RESHAPE_TOTAL_OUTPUT_NUM_PXL RESHAPE_OUTPUT_NUM_PXL

// Add
#define ADD_OUTPUT_NUM_PXL       6
#define ADD_TOTAL_OUTPUT_NUM_PXL ADD_OUTPUT_NUM_PXL

// Training
#define TRAIN_INPUT_MODELS        5
#define TRAIN_INPUT_NUM_PXL       DEBAYER_OUTPUT_NUM_PXL
#define TRAIN_TOTAL_INPUT_NUM_PXL TRAIN_INPUT_MODELS *TRAIN_INPUT_NUM_PXL

// Change detection
#define CHANGE_DETECTION_OUTPUT_NUM_PXL       DEBAYER_OUTPUT_NUM_PXL
#define CHANGE_DETECTION_TOTAL_OUTPUT_NUM_PXL CHANGE_DETECTION_OUTPUT_NUM_PXL

// == define memory sizes (in 32bit words) ====================================

// Debayer
#define DEBAYER_INPUT_MEM_SIZE  (INPUT_IMG_NUM_ROWS * INPUT_IMG_NUM_COLS >> 1) * INPUT_NUM_IMGS
#define DEBAYER_OUTPUT_MEM_SIZE (3 * (INPUT_IMG_NUM_ROWS - 2 * PAD) * (INPUT_IMG_NUM_COLS - 2 * PAD) >> 1)

// Grayscale (save 64 bits!)
#define GRAYSCALE_OUTPUT_MEM_SIZE 2 * (INPUT_IMG_NUM_ROWS - 2 * PAD) * (INPUT_IMG_NUM_COLS - 2 * PAD)

// Gradient (save 64 bits!)
#define GRADIENT_OUTPUT_MEM_SIZE 2 * (INPUT_IMG_NUM_ROWS - 2 * PAD) * (INPUT_IMG_NUM_COLS - 2 * PAD)

// Warp (save 64 bits!)
#define WARP_OUTPUT_MEM_SIZE 2 * (INPUT_IMG_NUM_ROWS - 2 * PAD) * (INPUT_IMG_NUM_COLS - 2 * PAD)

// Affine parameters (save 64 bits!)
#define AFFINE_PARAMETERS_MEM_SIZE 2 * 6

// Subtract (save 64 bits!)
#define SUBTRACT_OUTPUT_MEM_SIZE 2 * (INPUT_IMG_NUM_ROWS - 2 * PAD) * (INPUT_IMG_NUM_COLS - 2 * PAD)

// IWxp (save 64 bits!)
#define IWXP_MEM_SIZE 2 * (INPUT_IMG_NUM_ROWS - 2 * PAD) * (INPUT_IMG_NUM_COLS - 2 * PAD)

// Steepest descent (save 64 bits!)
#define STEEPEST_DESCENT_OUTPUT_MEM_SIZE 2 * 6 * (INPUT_IMG_NUM_ROWS - 2 * PAD) * (INPUT_IMG_NUM_COLS - 2 * PAD)

// Hessian (save 64 bits!)
#define HESSIAN_OUTPUT_MEM_SIZE (2 * 36)

// Invert Gauss Jordan (save 64 bits!)
#define INVERT_GAUSS_JORDAN_OUTPUT_MEM_SIZE (2 * 36)

// SD Update (save 64 bits!)
#define SD_UPDATE_OUTPUT_MEM_SIZE (2 * 6)

// Mult (save 64 bits!)
#define MULT_OUTPUT_MEM_SIZE (2 * 6)

// Reshape (save 64 bits!)
#define RESHAPE_OUTPUT_MEM_SIZE (2 * 6)

// Add (save 64 bits!)
#define ADD_OUTPUT_MEM_SIZE (2 * 6)

// Training (save 64 bits!)
#define TRAIN_INPUT_MEM_SIZE (2 * (INPUT_IMG_NUM_ROWS - 2 * PAD) * (INPUT_IMG_NUM_COLS - 2 * PAD) * TRAIN_INPUT_MODELS)

// Change detection
#define CHANGE_DETECTION_OUTPUT_MEM_SIZE (((INPUT_IMG_NUM_ROWS - 2 * PAD) * (INPUT_IMG_NUM_COLS - 2 * PAD)) >> 2)

#define MEM_SIZE                                                                                                  \
    DEBAYER_INPUT_MEM_SIZE + DEBAYER_OUTPUT_MEM_SIZE + GRAYSCALE_OUTPUT_MEM_SIZE + 2 * GRADIENT_OUTPUT_MEM_SIZE + \
        AFFINE_PARAMETERS_MEM_SIZE + WARP_OUTPUT_MEM_SIZE + IWXP_MEM_SIZE + SUBTRACT_OUTPUT_MEM_SIZE +            \
        2 * WARP_OUTPUT_MEM_SIZE + STEEPEST_DESCENT_OUTPUT_MEM_SIZE + HESSIAN_OUTPUT_MEM_SIZE +                   \
        INVERT_GAUSS_JORDAN_OUTPUT_MEM_SIZE + SD_UPDATE_OUTPUT_MEM_SIZE + MULT_OUTPUT_MEM_SIZE +                  \
        RESHAPE_OUTPUT_MEM_SIZE + ADD_OUTPUT_MEM_SIZE + WARP_OUTPUT_MEM_SIZE + 3 * TRAIN_INPUT_MEM_SIZE +         \
        CHANGE_DETECTION_OUTPUT_MEM_SIZE

// == define maximum tolerated error ==========================================
// maximum percentage of wrong pixels
// 0.1 -> 100 wrong pixel out of 1000
#define PXL_MAX_ERROR 0.1
// tollerance of error per single pixel
// 1e-05 = 0.00001 -> 1e-03% = 0.001%
// 1e-n -> 1e-(n-2)%
#define GRAYSCALE_MAX_ERROR              5 * 1e-05
#define GRADIENT_MAX_ERROR               5 * 1e-03
#define WARP_MAX_ERROR                   1e-03
#define SUBTRACT_MAX_ERROR               5 * 1e-04
#define STEEPEST_DESCENT_MAX_ERROR       5 * 1e-03
#define HESSIAN_MAX_ERROR                1e-06
#define INVERT_GAUSS_JORDAN_MAX_ERROR    3 * 1e-02
#define SD_UPDATE_MAX_ERROR              5 * 1e-05
#define MULT_MAX_ERROR                   3 * 1e-03
#define RESHAPE_MAX_ERROR                3 * 1e-03
#define ADD_MAX_ERROR                    3 * 1e-03
#define FWARP_MAX_ERROR                  3 * 1e-03
#define CHANGE_DETECTION_MAX_ERROR       5 * 1e-03
#define CHANGE_DETECTION_TRAIN_MAX_ERROR 1e-03

#if 0
    #define ADD_MAX_ERROR 0.001
#endif

#endif /* _WAMI_CONFIG_TB_H_ */
