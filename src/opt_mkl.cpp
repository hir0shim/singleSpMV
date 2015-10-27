#include <mkl.h>
#include "opt_mkl.h"
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
    int *ptr = (int *)mkl_malloc((nRow+1) * sizeof(int), ALIGNMENT);
    int *idx = (int *)mkl_malloc(nNnz * sizeof(int), ALIGNMENT);
    double *val =(double *)mkl_malloc(nNnz * sizeof(double), ALIGNMENT); 
    
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
}
extern "C" {
    void SpMV (const SpMatOpt &A, const VecOpt &x, Vec &y) {
        double* restrict xv = x.val;
        double* restrict yv = y.val;
        int nRow = A.nRow;
        int nCol = A.nCol;
        int nNnz = A.nNnz;

        //------------------------------
        // Format specific 
        //------------------------------
        int* restrict ptr = A.ptr;
        int* restrict idx = A.idx;
        double* restrict val = A.val;
        double ALPHA = 1;
        double BETA = 0;
        MKL_INT *ptr_b = static_cast<MKL_INT*>(ptr);
        MKL_INT *ptr_e = ptr_b + 1;
        char transa = 'N';
        char *matdescra = "GLNC";
        mkl_dcsrmv(&transa, &nRow, &nRow, &ALPHA, matdescra, val, idx, ptr_b, ptr_e, xv, &BETA, yv);
    }
}
