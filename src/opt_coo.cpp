#include "opt_coo.h"
#include "util.h"
void OptimizeProblem (const SpMat &A, const Vec &x, SpMatOpt &A_opt, VecOpt &x_opt) {
    A_opt.nRow = A.nRow;
    A_opt.nCol = A.nCol;
    A_opt.nNnz = A.nNnz;
    A_opt.row_idx = A.row_idx;
    A_opt.col_idx = A.col_idx;
    A_opt.val = A.val;

    x_opt.size = x.size;
    x_opt.val = x.val;

}
void SpMV (const SpMatOpt &A, const VecOpt &x, Vec &y) {
    double *xv = x.val;
    double *yv = y.val;
    int nRow = A.nRow;
    int nCol = A.nCol;
    int nNnz = A.nNnz;
    int *row_idx = A.row_idx;
    int *col_idx = A.col_idx;
    double *val = A.val;
#pragma omp parallel
    {
#pragma omp for
    for (int i = 0; i < nRow; i++) yv[i] = 0;
#pragma omp for
    for (int i = 0; i < nNnz; i++) {
        yv[row_idx[i]] += val[i] * xv[col_idx[i]];
    }
    }
}



