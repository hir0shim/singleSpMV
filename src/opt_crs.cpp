#include "opt_crs.h"
#include "util.h"
#include <algorithm>
#include <vector>
#include <utility>
#include <cassert>
#include <iostream>
#include <immintrin.h>
#include <mm_malloc.h>
void OptimizeProblem (const SpMat &A, const Vec &x, SpMatOpt &A_opt, VecOpt &x_opt) {
    x_opt.size = x.size;
    x_opt.val = x.val;

    int nRow = A.nRow;
    int nCol = A.nCol;
    int nNnz = A.nNnz;

    //------------------------------
    // Format specific 
    //------------------------------
    {
        int *ptr = (int *)_mm_malloc((nRow+1) * sizeof(int), ALIGNMENT);
        int *idx = (int *)_mm_malloc(nNnz * sizeof(int), ALIGNMENT);
        double *val = (double *)_mm_malloc(nNnz * sizeof(double), ALIGNMENT); 

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
#pragma omp parallel for
        for (int i = 0; i < nRow; i++) {
            double yv_tmp = 0;
#pragma ivdep
            for (int j = ptr[i]; j < ptr[i+1]; j++) {
                int col = idx[j];
                double lv = val[j];
                double rv = xv[col];
                double v = lv * rv;
                yv_tmp += v;
            }
            yv[i] = yv_tmp;
        }
    }
}
