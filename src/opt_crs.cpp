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
        double *xv = x.val;
        double *yv = y.val;
        int nRow = A.nRow;
        int nCol = A.nCol;
        int nNnz = A.nNnz;

        //------------------------------
        // Format specific 
        //------------------------------
        int *ptr = A.ptr;
        int *idx = A.idx;
        double *val = A.val;
        int W = ALIGNMENT / sizeof(double);
#pragma omp parallel for
        for (int i = 0; i < nRow; i++) {
            yv[i] = 0;
#if defined(CPU) && defined(SIMD)
            if (ptr[i+1] - ptr[i] > ALIGNMENT/sizeof(double)) {
                int simd_begin = (ptr[i] & (~(W-1))) + ((ptr[i] & (W-1)) ? W:0);
                int simd_end = ptr[i+1] - (ptr[i+1]&(W-1));
                for (int j = ptr[i]; j < simd_begin; j++) {
                    int col = idx[j];
                    double lv = val[j];
                    double rv = xv[col];
                    double v = lv * rv;
                    yv[i] += v;
                }
                double *yv_tmp = (double *)_mm_malloc(ALIGNMENT, ALIGNMENT);
                for (int j = simd_begin; j < simd_end; j+=W) {
                    assert(__int64(idx+j)%16==0);
                    assert(__int64(val+j)%32==0);
                    assert(__int64(yv_tmp)%32==0);
                    __m128i col = _mm_load_si128((__m128i *)(idx+j));
                    __m256d lv = _mm256_load_pd(val+j);
                    __m256d rv = _mm256_i32gather_pd(xv, col, 8);
                    __m256d v = _mm256_mul_pd(lv, rv);
                    _mm256_store_pd(yv_tmp, v);
                    for (int k = 0; k < ALIGNMENT / sizeof(double); k++) {
                        yv[i] += yv_tmp[k];
                    }
                }
                _mm_free(yv_tmp);
                for (int j = simd_end; j < ptr[i+1]; j++) {
                    int col = idx[j];
                    double lv = val[j];
                    double rv = xv[col];
                    double v = lv * rv;
                    yv[i] += v;
                }
            } else {
                for (int j = ptr[i]; j < ptr[i+1]; j++) {
                    int col = idx[j];
                    double lv = val[j];
                    double rv = xv[col];
                    double v = lv * rv;
                    yv[i] += v;
                }
            }
#elif defined(MIC) && defined(SIMD)
#else 
            for (int j = ptr[i]; j < ptr[i+1]; j++) {
                int col = idx[j];
                double lv = val[j];
                double rv = xv[col];
                double v = lv * rv;
                yv[i] += v;
            }
            /*
            double yv_tmp = 0;
            for (int j = ptr[i]; j < ptr[i+1]; j++) {
                int col = idx[j];
                double lv = val[j];
                double rv = xv[col];
                double v = lv * rv;
                yv_tmp += v;
            }
            yv[i] = yv_tmp;
            */
#endif
        }
    }
}



