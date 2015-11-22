#include "opt_coo.h"
#include "util.h"
void OptimizeProblem (const SpMat &A, const Vec &x, SpMatOpt &A_opt, VecOpt &x_opt) {
    x_opt.size = x.size;
    x_opt.val = x.val;

    int nRow = A.nRow;
    int nCol = A.nCol;
    int nNnz = A.nNnz;

    //------------------------------
    // Format specific 
    //------------------------------
    A_opt.nRow = nRow;
    A_opt.nCol = nCol;
    A_opt.nNnz = nNnz;
    A_opt.row_idx = A.row_idx;
    A_opt.col_idx = A.col_idx;
    A_opt.val = A.val;
}
void SpMV (const SpMatOpt &A, const VecOpt &x, Vec &y) {
    double *xv = x.val;
    double *yv = y.val;
    int nRow = A.nRow;
    int nCol = A.nCol;
    int nNnz = A.nNnz;

    //------------------------------
    // Format specific 
    //------------------------------
    int* restrict row_idx = A.row_idx;
    int* restrict col_idx = A.col_idx;
    double* restrict val = A.val;
#pragma omp parallel
    {
#pragma omp for
        for (int i = 0; i < nRow; i++) yv[i] = 0;
#pragma omp for
        for (int i = 0; i < nNnz; i++) {
            int r = row_idx[i];
            int c = col_idx[i];
            double v = val[i]*xv[c];
#pragma omp atomic
            yv[r] += v;
        }
    }
}



