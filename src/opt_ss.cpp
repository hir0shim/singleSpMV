#include "opt_ss.h"
#include "util.h"
#include <algorithm>
#include <vector>
#include <utility>
#include <cassert>
#include <iostream>
#include <cmath>
#include <immintrin.h>
void OptimizeProblem (const SpMat &A, const Vec &x, SpMatOpt &A_opt, VecOpt &x_opt) {
    x_opt.size = x.size;
    x_opt.val = x.val;

    int nRow = A.nRow;
    int nCol = A.nCol;
    int nNnz = A.nNnz;

    //------------------------------
    // Format specific 
    //------------------------------

    int *row_idx = A.row_idx;
    int *col_idx = A.col_idx;
    double *val = A.val;

    int W = A_opt.W;
    int H = nNnz / W + (nNnz % W != 0);
    int *row_ptr = (int *)_mm_malloc((nRow+1)*sizeof(int), ALIGNMENT);
    int **row_idx_2d = (int **)_mm_malloc(H*sizeof(int*), ALIGNMENT);
    int **col_idx_2d = (int **)_mm_malloc(H*sizeof(int*), ALIGNMENT);
    double **val_2d = (double **)_mm_malloc(H*sizeof(double*), ALIGNMENT);
    bool **flag_2d = (bool **)_mm_malloc(H*sizeof(bool*), ALIGNMENT);
    int **index_2d = (int **)_mm_malloc(H*sizeof(int*), ALIGNMENT);

    row_idx_2d[0] = (int *)_mm_malloc(H*W*sizeof(int), ALIGNMENT);
    col_idx_2d[0] = (int *)_mm_malloc(H*W*sizeof(int), ALIGNMENT);
    val_2d[0] = (double *)_mm_malloc(H*W*sizeof(double), ALIGNMENT);
    flag_2d[0] = (bool *)_mm_malloc(H*W*sizeof(bool), ALIGNMENT);
    index_2d[0] = (int *)_mm_malloc(H*W*sizeof(int), ALIGNMENT);

    for (int i = 1; i < H; i++) {
        row_idx_2d[i] = row_idx_2d[i-1] + W;
        col_idx_2d[i] = col_idx_2d[i-1] + W;
        val_2d[i] = val_2d[i-1] + W;
        flag_2d[i] = flag_2d[i-1] + W;
        index_2d[i] = index_2d[i-1] + W;
    }
    row_ptr[0] = 0;
    int cur_row = 0;
    for (int i = 0; i < H; i++) {
        for (int j = 0; j < W; j++) {
            int p = i*W + j;
            if (p < nNnz) {
                row_idx_2d[i][j] = row_idx[p];
                col_idx_2d[i][j] = col_idx[p];
                val_2d[i][j] = val[p];
                int r = row_idx[p];
                while (cur_row <= r) { row_ptr[cur_row++] = p; }
            } else {
                row_idx_2d[i][j] = nRow;
                col_idx_2d[i][j] = 0;
                val_2d[i][j] = 0;
                index_2d[i][j] = 0;
            }
        }
    }
    while (cur_row <= nRow) row_ptr[cur_row++] = nNnz;


    //------------------------------
    // Segment index
    //------------------------------
    int *segment_index = (int *)_mm_malloc(H*sizeof(int), ALIGNMENT);
    segment_index[0] = 0;
    for (int i = 1; i < H; i++) {
        bool same = true; // row idx of the segment i are same or not.
        if (row_idx_2d[i-1][0] == row_idx_2d[i][0]) {
            for (int j = 1; j < W; j++) {
                if (row_idx_2d[i][j-1] != row_idx_2d[i][j]) same = false;
            }
        } else {
            same = false;
        }
        if (same) {
            segment_index[i] = segment_index[i-1] + 1;
        } else {
            segment_index[i] = 0;
        }
    }
    int max_index = *max_element(segment_index, segment_index + H);
    for (int i = 0; i < H; i++) {
        if (segment_index[i] > 0) {
            for (int j = 1; j < W; j++) {
                assert(row_idx_2d[i][j] == row_idx_2d[i][j-1]);
            }
            for (int j = 0; j < W; j++) {
                assert(row_idx_2d[i][j] == row_idx_2d[i-1][j]);
            }
        }
    }

    A_opt.nRow = nRow;
    A_opt.nCol = nCol;
    A_opt.nNnz = nNnz;
    A_opt.H = H;
    A_opt.row_ptr = row_ptr;
    A_opt.row_idx = row_idx_2d;
    A_opt.col_idx = col_idx_2d;
    A_opt.flag = flag_2d;
    A_opt.val = val_2d;
    A_opt.segment_index = segment_index;
    A_opt.max_index = 1 << int(ceil(log2(max_index+1)));

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
        const int H = A.H;
        const int W = A.W;
        int** restrict col_idx = A.col_idx;
        double** restrict val = A.val;
        int* restrict row_ptr = A.row_ptr;
        int* restrict segment_index = A.segment_index;
        int max_index = A.max_index;
#pragma omp parallel for schedule(static)
        for (int i = 0; i < H; i++) {
            //------------------------------
            // Mul
            //------------------------------
#if defined(MIC) && defined(INTRINSICS)
            __m512i col = _mm512_load_epi32(col_idx[i]);
            __m512d lv = _mm512_load_pd(val[i]);
            __m512d rv = _mm512_i32logather_pd(col, xv, sizeof(double));
            __m512d v = _mm512_mul_pd(lv, rv);
            _mm512_storenrngo_pd(val[i], v);

            col = _mm512_alignr_epi32(col, col, ALIGNMENT/sizeof(int)/2);
            lv = _mm512_load_pd(val[i] + ALIGNMENT/sizeof(double));
            rv = _mm512_i32logather_pd(col, xv, sizeof(double));
            v = _mm512_mul_pd(lv, rv);
            _mm512_storenrngo_pd(val[i] + ALIGNMENT/sizeof(double), v);

#elif defined(CPU) && defined(INTRINSICS) 
            asset(false); // TODO
#else
            double* restrict val_tmp = val[i];
            for (int j = 0; j < W; j++) {
                int col = col_idx[i][j];
                double rv = xv[col];
                val_tmp[j] *= rv;
            }
#endif
        }

        //------------------------------
        // Summation
        //------------------------------
        /*
#if defined(MIC) && defined(INTRINSICS)
        asset(false); // TODO
#elif defined(CPU) && defined(INTRINSICS)
        asset(false); // TODO
#else 
*/
        // reduction
        int counter = max_index>>1;
        while (counter > 0) {
#pragma omp parallel for
            for (int i = 0; i < H; i++) {
                if (counter <= segment_index[i] && segment_index[i] < counter*2) {
                    for (int j = 0; j < W; j++) {
                        val[i-counter][j] += val[i][j];
                        val[i][j] = 0;
                    }
                }
            }
            counter >>= 1;
        }
#pragma omp parallel for
        for (int i = 0; i < nRow; i++) {
            double yv_tmp = 0;
            int begin = row_ptr[i];
            int end = row_ptr[i+1];
            // upper
            while (begin % W != 0 && begin < end) {
                yv_tmp += *(val[0] + begin);
                begin++;
            }
            // lower
            while (end % W != 0 && begin < end) {
                yv_tmp += *(val[0] + end - 1);
                end--;
            }
            // center
            if (begin != end) {
                for (int j = 0; j < W; j++) {
                    yv_tmp += *(val[0] + begin + j);
                }
            }
            yv[i] = yv_tmp;
        }
        /*
        //#pragma omp for
        for (int i = 0; i < nRow; i++) {
        yv[i] = 0;
        for (int j = row_ptr[i]; j < row_ptr[i+1]; j++) {
        yv[i] += *(val[0] + j);
        }
        }
        */
//#endif
    }
}



