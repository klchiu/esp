#ifndef _WAMI_CONFIG_H_
#define _WAMI_CONFIG_H_

// WAMI_DEBAYER_PAD: The number of edge pixels clipped during the
// debayer process due to not having enough pixels for the full
// interpolation kernel. Other interpolations could be applied near
// the edges, but we instead clip the image for simplicity.
#define WAMI_DEBAYER_PAD 2

// Shorten the name of the Debayer pad for readability
#define PAD WAMI_DEBAYER_PAD

#define NUM_IMGS 1
#define NUM_ROWS 132
#define NUM_COLS 132

#define WAMI_DEBAYER_IMG_NUM_IMGS NUM_IMGS
#define WAMI_DEBAYER_IMG_NUM_ROWS NUM_ROWS
#define WAMI_DEBAYER_IMG_NUM_COLS NUM_ROWS

#define WAMI_DEBAYER_FOOTPRINT_SIZE 5

#define WAMI_STEEP_DESCENT_HESSIAN_IMG_NUM_IMGS NUM_IMGS
#define WAMI_STEEP_DESCENT_HESSIAN_IMG_NUM_ROWS (WAMI_DEBAYER_IMG_NUM_ROWS - (2 * PAD))
#define WAMI_STEEP_DESCENT_HESSIAN_IMG_NUM_COLS (WAMI_DEBAYER_IMG_NUM_COLS - (2 * PAD))

#define WAMI_GRADIENT_PAD 1

#define WAMI_GRADIENT_IMG_NUM_IMGS NUM_IMGS
#define WAMI_GRADIENT_IMG_NUM_ROWS (WAMI_DEBAYER_IMG_NUM_ROWS - (2 * PAD))
#define WAMI_GRADIENT_IMG_NUM_COLS (WAMI_DEBAYER_IMG_NUM_COLS - (2 * PAD))

#define WAMI_GRADIENT_FOOTPRINT_SIZE 3

#define WAMI_WARP_IMG_NUM_IMGS NUM_IMGS
#define WAMI_WARP_IMG_NUM_ROWS (WAMI_DEBAYER_IMG_NUM_ROWS - (2 * PAD))
#define WAMI_WARP_IMG_NUM_COLS (WAMI_DEBAYER_IMG_NUM_COLS - (2 * PAD))

#define WAMI_SUBTRACT_IMG_NUM_IMGS NUM_IMGS
#define WAMI_SUBTRACT_IMG_NUM_ROWS (WAMI_DEBAYER_IMG_NUM_ROWS - (2 * PAD))
#define WAMI_SUBTRACT_IMG_NUM_COLS (WAMI_DEBAYER_IMG_NUM_COLS - (2 * PAD))

#define WAMI_GMM_IMG_NUM_IMGS NUM_IMGS
#define WAMI_GMM_IMG_NUM_ROWS (WAMI_DEBAYER_IMG_NUM_ROWS - (2 * PAD))
#define WAMI_GMM_IMG_NUM_COLS (WAMI_DEBAYER_IMG_NUM_COLS - (2 * PAD))
#define WAMI_GMM_NUM_MODELS   5

#define WAMI_CHANGE_DETECTION_NUM_CHUNKS 4

// NOTE: the number of lines for the circular buffer has to be equal or greater
// than the FOOTPRINT_SIZE. If equal, then there will be not pipelining.
#define WAMI_DEBAYER_IMG_NUM_CB_ROWS  (WAMI_DEBAYER_FOOTPRINT_SIZE + 1)
#define WAMI_GRADIENT_IMG_NUM_CB_ROWS (WAMI_GRADIENT_FOOTPRINT_SIZE + 3)

#define WAMI_DEBAYER_IMG_PIXEL_MAX 65535

// Kernel IDs
#define DEBAYER_KERN_ID             1
#define GRAYSCALE_KERN_ID           2
#define GRADIENT_KERN_ID            3
#define WARP_KERN_ID                4
#define SUBTRACT_KERN_ID            5
#define STEEPEST_DESCENT_KERN_ID    6
#define HESSIAN_KERN_ID             7
#define INVERT_GAUSS_JORDAN_KERN_ID 8
#define SD_UPDATE_KERN_ID           9
#define MULT_KERN_ID                10
#define RESHAPE_KERN_ID             11
#define ADD_KERN_ID                 12
#define CHANGE_DETECTION_KERN_ID    13

#endif /* _WAMI_CONFIG_H_ */
