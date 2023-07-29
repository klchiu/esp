// Copyright (c) 2011-2023 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0

#ifndef __PVC_GEMMU8_PV_H__
#define __PVC_GEMMU8_PV_H__

#define M_DIMS 3

#include "double_matrix_t.h"
#include "fpdata.hpp"

void gemmU8_pv(double_matrix_t *matrix_in1, double_matrix_t *matrix_in2, double_matrix_t **matrix_out);

#endif // __PVC_GEMMU8_PV_H__
