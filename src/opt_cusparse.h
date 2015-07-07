#pragma once
#include <cuda_runtime_api.h>
#include <cusparse_v2.h>
#include "util.h"
struct SpMatOpt {
    int nRow;
    int nCol;
    int nNnz;
    int *ptr;
    int *idx;
    double *val;

    int *cuda_ptr;
    int *cuda_idx;
    double *cuda_val;
    double *cuda_x;
    double *cuda_y;
};
struct VecOpt {
    int size;
    double *val;
};
void OptimizeProblem (const SpMat &A, const Vec &x, SpMatOpt &A_opt, VecOpt &x_opt);
void SpMV (const SpMatOpt &A, const VecOpt &x, Vec &y);
void CUDA_SAFE_CALL (cudaError_t err) { }
