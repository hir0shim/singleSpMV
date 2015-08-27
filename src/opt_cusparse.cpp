#include <iostream>
#include <cassert>
#include <cuda_runtime_api.h>
#include <cusparse_v2.h>
#include "opt_cusparse.h"
#include "util.h"
using namespace std;
void OptimizeProblem (const SpMat &A, const Vec &x, SpMatOpt &A_opt, VecOpt &x_opt) {
    int nRow = A.nRow;
    int nCol = A.nCol;
    int nNnz = A.nNnz;
    int *ptr = new int[nRow+1];
    int *idx = new int[nNnz];
    double *val = new double[nNnz];
    int p = 0;
    for (int i = 0; i < nNnz; i++) {
        int r = A.row_idx[i];
        idx[i] = A.col_idx[i];
        val[i] = A.val[i];
        while (p <= r) { ptr[p++] = i; }
    }
    while (p <= nRow) ptr[p++] = nNnz;

    int *cuda_ptr;
    int *cuda_idx;
    double *cuda_val;
    double *cuda_x;
    double *cuda_y;
    CUDA_SAFE_CALL(cudaMalloc((void**)&cuda_ptr, (A.nRow + 1) * sizeof(int)));
    CUDA_SAFE_CALL(cudaMalloc((void**)&cuda_idx, A.nNnz * sizeof(int)));
    CUDA_SAFE_CALL(cudaMalloc((void**)&cuda_val, A.nNnz * sizeof(double)));

    CUDA_SAFE_CALL(cudaMemcpy((void *)cuda_ptr, ptr, (A.nRow + 1) * sizeof(int), cudaMemcpyHostToDevice));
    CUDA_SAFE_CALL(cudaMemcpy((void *)cuda_idx, idx, A.nNnz * sizeof(int), cudaMemcpyHostToDevice));
    CUDA_SAFE_CALL(cudaMemcpy((void *)cuda_val, val, A.nNnz * sizeof(double), cudaMemcpyHostToDevice));

    CUDA_SAFE_CALL(cudaMalloc((void**)&cuda_x, nCol * sizeof(double)));
    CUDA_SAFE_CALL(cudaMalloc((void**)&cuda_y, nRow * sizeof(double)));

    A_opt.nRow = nRow;
    A_opt.nCol = nCol;
    A_opt.nNnz = nNnz;
    A_opt.cuda_ptr = cuda_ptr;
    A_opt.cuda_idx = cuda_idx;
    A_opt.cuda_val = cuda_val;
    A_opt.cuda_x = cuda_x;
    A_opt.cuda_y = cuda_y;
    x_opt.size = x.size;
    x_opt.val = x.val;
}
void SpMV (const SpMatOpt &A, const VecOpt &x, Vec &y) {
    double *xv = x.val;
    double *yv = y.val;
    int nRow = A.nRow;
    int nCol = A.nCol;
    int nNnz = A.nNnz;
    int *cuda_ptr = A.cuda_ptr;
    int *cuda_idx = A.cuda_idx;
    double *cuda_val = A.cuda_val;
    double *cuda_x = A.cuda_x;
    double *cuda_y = A.cuda_y;
    CUDA_SAFE_CALL(cudaMemcpy((void *)cuda_x, xv, nCol * sizeof(double), cudaMemcpyHostToDevice));
    cusparseHandle_t cusparse;
    cusparseCreate(&cusparse);
    cusparseMatDescr_t matDescr;
    cusparseCreateMatDescr(&matDescr);
    cusparseSetMatType(matDescr, CUSPARSE_MATRIX_TYPE_GENERAL);
    cusparseSetMatIndexBase(matDescr, CUSPARSE_INDEX_BASE_ZERO);
    const double ALPHA = 1;
    const double BETA = 0;
    cusparseDcsrmv(cusparse, CUSPARSE_OPERATION_NON_TRANSPOSE, nRow, nCol, nNnz, &ALPHA, matDescr, cuda_val, cuda_ptr, cuda_idx, cuda_x, &BETA, cuda_y);
    CUDA_SAFE_CALL(cudaMemcpy((void *)yv, cuda_y, nRow * sizeof(double), cudaMemcpyDeviceToHost));
}

