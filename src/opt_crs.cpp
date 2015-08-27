#include "opt_crs.h"
#include "util.h"
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
    A_opt.nRow = nRow;
    A_opt.nCol = nCol;
    A_opt.nNnz = nNnz;
    A_opt.ptr = ptr;
    A_opt.idx = idx;
    A_opt.val = val;

    x_opt.size = x.size;
    x_opt.val = x.val;

}
void SpMV (const SpMatOpt &A, const VecOpt &x, Vec &y) {
    double *xv = x.val;
    double *yv = y.val;
    int nRow = A.nRow;
    int nNnz = A.nNnz;
    int *ptr = A.ptr;
    int *idx = A.idx;
    double *val = A.val;
#pragma omp parallel for
    for (int i = 0; i < nRow; i++) {
        yv[i] = 0;
        for (int j = ptr[i]; j < ptr[i+1]; j++) {
            yv[i] += val[j] * xv[idx[j]];
        }
    }
}



