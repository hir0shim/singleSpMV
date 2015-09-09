#pragma once
#include <cuda_runtime_api.h>
#include <cusparse_v2.h>
#include "util.h"
struct SpMatOpt {
    int nRow;
    int nCol;
    int nNnz;
    /*
    int *ptr;
    int *idx;
    double *val;
    */

    int *cuda_ptr;
    int *cuda_idx;
    double *cuda_val;
    double *cuda_x;
    double *cuda_y;
    SpMatOpt () {
        nRow = nCol = nNnz = -1;
        cuda_ptr = cuda_idx = NULL;
        cuda_val = cuda_x = cuda_y = NULL;
    }
};
struct VecOpt {
    int size;
    double *val;
    VecOpt () {
        size = -1;
        val = NULL;
    }
};
void OptimizeProblem (const SpMat &A, const Vec &x, SpMatOpt &A_opt, VecOpt &x_opt);
extern "C" {
void SpMV (const SpMatOpt &A, const VecOpt &x, Vec &y);
}
//void CUDA_SAFE_CALL (cudaError_t err) { }
