#ifndef _WAMI_PARAMS_H_
#define _WAMI_PARAMS_H_

#include "wami_C_data.h"

#define RED_CHAN   0
#define GREEN_CHAN 1
#define BLUE_CHAN  2

#define INPUT_SIZE_TINY    0
#define INPUT_SIZE_132_17  1
#define INPUT_SIZE_SMALL   2
#define INPUT_SIZE_516_17  3
#define INPUT_SIZE_516_33  4
#define INPUT_SIZE_MEDIUM  5
#define INPUT_SIZE_1028_17 6
#define INPUT_SIZE_1028_33 7
#define INPUT_SIZE_LARGE   8

#ifndef INPUT_SIZE
    #define INPUT_SIZE INPUT_SIZE_132_17
#endif

/*
 * WAMI_DEBAYER_PAD: The number of edge pixels clipped during the
 * debayer process due to not having enough pixels for the full
 * interpolation kernel. Other interpolations could be applied near
 * the edges, but we instead clip the image for simplicity.
 */

#if INPUT_SIZE == INPUT_SIZE_TINY
    #ifndef WAMI_DEBAYER_IMG_NUM_ROWS
        #define WAMI_DEBAYER_IMG_NUM_ROWS 16
    #endif
    #ifndef WAMI_DEBAYER_IMG_NUM_COLS
        #define WAMI_DEBAYER_IMG_NUM_COLS 16
    #endif
    #ifndef WAMI_GMM_IMG_NUM_ROWS
        #define WAMI_GMM_IMG_NUM_ROWS 16
    #endif
    #ifndef WAMI_GMM_IMG_NUM_COLS
        #define WAMI_GMM_IMG_NUM_COLS 16
    #endif
    #define WAMI_GMM_NUM_FRAMES 4

#elif INPUT_SIZE == INPUT_SIZE_132_17
    #ifndef WAMI_DEBAYER_IMG_NUM_ROWS
        #define WAMI_DEBAYER_IMG_NUM_ROWS 132
    #endif
    #ifndef WAMI_DEBAYER_IMG_NUM_COLS
        #define WAMI_DEBAYER_IMG_NUM_COLS 132
    #endif
    #ifndef WAMI_GMM_IMG_NUM_ROWS
        #define WAMI_GMM_IMG_NUM_ROWS 132
    #endif
    #ifndef WAMI_GMM_IMG_NUM_COLS
        #define WAMI_GMM_IMG_NUM_COLS 132
    #endif
    #define WAMI_GMM_NUM_FRAMES 17

#elif INPUT_SIZE == INPUT_SIZE_SMALL
    #ifndef WAMI_DEBAYER_IMG_NUM_ROWS
        #define WAMI_DEBAYER_IMG_NUM_ROWS 512
    #endif
    #ifndef WAMI_DEBAYER_IMG_NUM_COLS
        #define WAMI_DEBAYER_IMG_NUM_COLS 512
    #endif
    #ifndef WAMI_GMM_IMG_NUM_ROWS
        #define WAMI_GMM_IMG_NUM_ROWS 512
    #endif
    #ifndef WAMI_GMM_IMG_NUM_COLS
        #define WAMI_GMM_IMG_NUM_COLS 512
    #endif
    #define WAMI_GMM_NUM_FRAMES 5

#elif INPUT_SIZE == INPUT_SIZE_516_17
    #ifndef WAMI_DEBAYER_IMG_NUM_ROWS
        #define WAMI_DEBAYER_IMG_NUM_ROWS 516
    #endif
    #ifndef WAMI_DEBAYER_IMG_NUM_COLS
        #define WAMI_DEBAYER_IMG_NUM_COLS 516
    #endif
    #ifndef WAMI_GMM_IMG_NUM_ROWS
        #define WAMI_GMM_IMG_NUM_ROWS 516
    #endif
    #ifndef WAMI_GMM_IMG_NUM_COLS
        #define WAMI_GMM_IMG_NUM_COLS 516
    #endif
    #define WAMI_GMM_NUM_FRAMES 17

#elif INPUT_SIZE == INPUT_SIZE_516_33
    #ifndef WAMI_DEBAYER_IMG_NUM_ROWS
        #define WAMI_DEBAYER_IMG_NUM_ROWS 516
    #endif
    #ifndef WAMI_DEBAYER_IMG_NUM_COLS
        #define WAMI_DEBAYER_IMG_NUM_COLS 516
    #endif
    #ifndef WAMI_GMM_IMG_NUM_ROWS
        #define WAMI_GMM_IMG_NUM_ROWS 516
    #endif
    #ifndef WAMI_GMM_IMG_NUM_COLS
        #define WAMI_GMM_IMG_NUM_COLS 516
    #endif
    #define WAMI_GMM_NUM_FRAMES 33

#elif INPUT_SIZE == INPUT_SIZE_MEDIUM
    #ifndef WAMI_DEBAYER_IMG_NUM_ROWS
        #define WAMI_DEBAYER_IMG_NUM_ROWS 1024
    #endif
    #ifndef WAMI_DEBAYER_IMG_NUM_COLS
        #define WAMI_DEBAYER_IMG_NUM_COLS 1024
    #endif
    #define WAMI_GMM_IMG_NUM_ROWS 1024
    #define WAMI_GMM_IMG_NUM_COLS 1024
    #define WAMI_GMM_NUM_FRAMES   5

#elif INPUT_SIZE == INPUT_SIZE_1028_17
    #ifndef WAMI_DEBAYER_IMG_NUM_ROWS
        #define WAMI_DEBAYER_IMG_NUM_ROWS 1028
    #endif
    #ifndef WAMI_DEBAYER_IMG_NUM_COLS
        #define WAMI_DEBAYER_IMG_NUM_COLS 1028
    #endif
    #ifndef WAMI_GMM_IMG_NUM_ROWS
        #define WAMI_GMM_IMG_NUM_ROWS 1028
    #endif
    #ifndef WAMI_GMM_IMG_NUM_COLS
        #define WAMI_GMM_IMG_NUM_COLS 1028
    #endif
    #define WAMI_GMM_NUM_FRAMES 17

#elif INPUT_SIZE == INPUT_SIZE_1028_33
    #ifndef WAMI_DEBAYER_IMG_NUM_ROWS
        #define WAMI_DEBAYER_IMG_NUM_ROWS 1028
    #endif
    #ifndef WAMI_DEBAYER_IMG_NUM_COLS
        #define WAMI_DEBAYER_IMG_NUM_COLS 1028
    #endif
    #ifndef WAMI_GMM_IMG_NUM_ROWS
        #define WAMI_GMM_IMG_NUM_ROWS 1028
    #endif
    #ifndef WAMI_GMM_IMG_NUM_COLS
        #define WAMI_GMM_IMG_NUM_COLS 1028
    #endif
    #define WAMI_GMM_NUM_FRAMES 33

#elif INPUT_SIZE == INPUT_SIZE_LARGE
    #ifndef WAMI_DEBAYER_IMG_NUM_ROWS
        #define WAMI_DEBAYER_IMG_NUM_ROWS 2048
    #endif
    #ifndef WAMI_DEBAYER_IMG_NUM_COLS
        #define WAMI_DEBAYER_IMG_NUM_COLS 2048
    #endif
    #define WAMI_GMM_IMG_NUM_ROWS 2048
    #define WAMI_GMM_IMG_NUM_COLS 2048
    #define WAMI_GMM_NUM_FRAMES   5

#else
    #error "Unhandled value for INPUT_SIZE"
#endif

#ifndef WAMI_GMM_NUM_MODELS
    #define WAMI_GMM_NUM_MODELS 5
#endif

#define WAMI_DEBAYER_PAD 2

/* Shorten the name of the Debayer pad for readability */
#define PAD WAMI_DEBAYER_PAD

#endif /* _WAMI_PARAMS_H_ */
