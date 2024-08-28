/* Copyright (c) 2011-2023 Columbia University, System Level Design Group */
/* SPDX-License-Identifier: Apache-2.0 */

#include <stdio.h>
#ifndef __riscv
#include <stdlib.h>
#endif

#include <esp_accelerator.h>
#include <esp_probe.h>
#include "monitors.h"

#include <string.h>

#include <time.h>

// #include "mojo.h"
// #include "mojo_utils.h"
#include "mojo/dwarf.h"


// #define IMAGE_PATH "../data/truck3.bmp"
// #define MODEL_FILE "../models/dwarf7.mojo"



int main(int argc, char * argv[])
{
	int i, j;

	for(i = 0 ; i < 20 ; i++){
		printf("Hey Blook~ %d\n", i);
	}
	

float dst[36992];
float bias[32];
float src[3468];
float w[1152];
memset(dst, 0, 36992);
memset(bias, 0, 32);
memset(src, 0, 3468);
memset(w, 0, 1152);
	printf("-- memset done\n");



// for(i = 0 ; i < 36992 ; i++){
// 	dst[i] = (float) i / 11;
// }
// 	printf("-- load dst done\n");


// for(i = 0 ; i < 32 ; i++){
// 	bias[i] = (float) i / 7;
// }
// printf("-- load bias done\n");

// for(i = 0 ; i < 3468 ; i++){
// 	src[i] = (float) i / 9;
// }
// printf("-- load src done\n");

// for(i = 0 ; i < 1152 ; i++){
// 	w[i] = (float) i / 13;
// }
// printf("-- load w done\n");




    // ------- Run inference -------
	printf("-- before convolution_compute()\n");

// convolution_compute(
// dst, // cnn.layer_sets[1]->node.x,
// bias, // cnn.layer_sets[1]->bias.x,
// src, // cnn.layer_sets[0]->node.x,
// w, // cnn.W[0]->x,
// 34, // cnn.layer_sets[1]->node.cols,
// 34, // cnn.layer_sets[1]->node.rows,
// 3, // cnn.layer_sets[0]->node.chans,
// 32, // cnn.layer_sets[1]->node.chans,
// 34, // get_pool_size(cnn.layer_sets[1]->node.cols,
// // 			  cnn.layer_sets[1]->node.rows,
// //               cnn.layer_sets[1]->do_pool,
// // 			  cnn.layer_sets[1]->do_pad),
// 324, // get_pool_stride(cnn.layer_sets[1]->node.cols,
// // 			    cnn.layer_sets[1]->node.rows,
// // 				cnn.layer_sets[1]->do_pool,
// // 				cnn.layer_sets[1]->do_pad),
// 1, // cnn.layer_sets[1]->do_pool,
// 1 // cnn.layer_sets[1]->do_pad
// );
	
printf("-- after convolution_compute()\n");

	return 0;
}
