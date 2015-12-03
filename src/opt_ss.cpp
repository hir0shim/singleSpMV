#include "opt_ss.h"
#include "util.h"
#include "param.h"
#include <algorithm>
#include <vector>
#include <utility>
#include <cassert>
#include <iostream>
#include <cmath>
#include <immintrin.h>
vector<int> g_step_count;
vector<double> g_step_time;
void OptimizeProblem (const SpMat &A, const Vec &x, SpMatOpt &A_opt, VecOpt &x_opt) {
    x_opt.size = x.size;
    x_opt.val = x.val;

    int nRow = A.nRow;
    int nCol = A.nCol;
    int nNnz = A.nNnz;

    //------------------------------
    // Format specific 
    //------------------------------
#ifdef PADDING
    const int pad = PADDING_SIZE;
#endif
    int *row_idx = A.row_idx;
    int *col_idx = A.col_idx;
    double *val = A.val;

    int W = A_opt.W;
    int H = nNnz / W + (nNnz % W != 0);
    int *row_ptr = (int *)_mm_malloc((nRow+1)*sizeof(int), ALIGNMENT);
    int **row_idx_2d = (int **)_mm_malloc(H*sizeof(int*), ALIGNMENT);
    idx_t **col_idx_2d = (idx_t **)_mm_malloc(H*sizeof(idx_t*), ALIGNMENT);
    double **val_2d = (double **)_mm_malloc(H*sizeof(double*), ALIGNMENT);
    int **index_2d = (int **)_mm_malloc(H*sizeof(int*), ALIGNMENT);

    row_idx_2d[0] = (int *)_mm_malloc(H*W*sizeof(int), ALIGNMENT);
    col_idx_2d[0] = (idx_t *)_mm_malloc(H*W*sizeof(idx_t), ALIGNMENT);
#ifdef PADDING
    val_2d[0] = (double *)_mm_malloc(H*(W+pad)*sizeof(double), ALIGNMENT);
#else
    val_2d[0] = (double *)_mm_malloc(H*W*sizeof(double), ALIGNMENT);
#endif
    index_2d[0] = (int *)_mm_malloc(H*W*sizeof(int), ALIGNMENT);

    for (int i = 1; i < H; i++) {
        row_idx_2d[i] = row_idx_2d[i-1] + W;
        col_idx_2d[i] = col_idx_2d[i-1] + W;
#ifdef PADDING
        val_2d[i] = val_2d[i-1] + W + pad;
#else
        val_2d[i] = val_2d[i-1] + W;
#endif
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


    int nStep = int(ceil(log2(max_index+1)));
    int counter = 1 << nStep;
    int **sum_segs = (int **)_mm_malloc(nStep*sizeof(int*), ALIGNMENT);
    int *sum_segs_count = (int *)_mm_malloc(nStep*sizeof(int), ALIGNMENT);
    for (int i = 0; i < nStep; i++) {
        counter >>= 1;
        int nSeg = 0;
        for (int j = 0; j < H; j++) {
            if (counter <= segment_index[j] && segment_index[j] < counter*2) {
                nSeg++;
            }
        }
        sum_segs_count[i] = nSeg;
        sum_segs[i] = (int *)_mm_malloc(nSeg*sizeof(int), ALIGNMENT);
        int idx = 0;
        for (int j = 0; j < H; j++) {
            if (counter <= segment_index[j] && segment_index[j] < counter*2) {
                sum_segs[i][idx++] = j;
            }
        }
        assert(idx == nSeg);
    }
    g_step_count.resize(nStep);
    for (int i = 0; i < nStep; i++) {
        g_step_count[i] = sum_segs_count[i];
    }
    g_step_time.resize(nStep);

    A_opt.nRow = nRow;
    A_opt.nCol = nCol;
    A_opt.nNnz = nNnz;
    A_opt.H = H;
    A_opt.row_ptr = row_ptr;
    A_opt.row_idx = row_idx_2d;
    A_opt.col_idx = col_idx_2d;
    A_opt.val = val_2d;
    A_opt.segment_index = segment_index;

    A_opt.nStep = nStep;
    A_opt.sum_segs = sum_segs;
    A_opt.sum_segs_count = sum_segs_count;


}
extern "C" {
    void __attribute__((noinline)) SpMV (const SpMatOpt &A, const VecOpt &x, Vec &y) {
        double* restrict xv = x.val;
        double* restrict yv = y.val;
        int nRow = A.nRow;
        int nCol = A.nCol;
        int nNnz = A.nNnz;

        //------------------------------
        // Format specific 
        //------------------------------
        const int H = A.H;
        const int W = SEGMENT_WIDTH;
        idx_t** restrict col_idx = A.col_idx;
        double** restrict val = A.val;
        int* restrict row_ptr = A.row_ptr;
        int* restrict segment_index = A.segment_index;
        int nStep = A.nStep;
        int** restrict sum_segs = A.sum_segs;
        int* restrict sum_segs_count = A.sum_segs_count;

#if defined(SIMPLE) 
        //{{{
        // Mul
#pragma omp parallel for 
        for (int i = 0; i < H; i++) {
            for (int j = 0; j < W; j++) {
                val[i][j] *= xv[col_idx[i][j]];
            }
        }
        // Sum
#pragma omp parallel for
        for (int i = 0; i < nRow; i++) {
            yv[i] = 0;
            for (int j = row_ptr[i]; j < row_ptr[i+1]; j++) {
                yv[i] += *(val[0] + j);
            }
        }
        //}}}
#elif defined(OPTIMIZED)
        //{{{
        // Mul
#pragma omp parallel for schedule(static)
        for (int i = 0; i < H; i++) {
            double* restrict val_tmp = val[i];
            int* restrict col_tmp = col_idx[i];
#pragma ivdep
            for (int j = 0; j < W; j++) {
                int col = col_tmp[j];
                double rv = xv[col];
                val_tmp[j] *= rv;
            }
        }
        // Sum 1
        int counter = 1<<nStep;
        for (int s = 0; s < nStep; s++) {
#ifdef MEASURE_STEP_TIME
            g_step_time[s] -= GetTimeBySec();
#endif
            counter >>= 1;
#pragma omp parallel for
            for (int i = 0; i < sum_segs_count[s]; i++) {
                int h = sum_segs[s][i];
#pragma ivdep
#pragma vector aligned
                for (int j = 0; j < W; j++) {
                    val[h-counter][j] += val[h][j];
                }
            }
#ifdef MEASURE_STEP_TIME
            g_step_time[s] += GetTimeBySec();
#endif
        }

        // Sum 2
#ifndef PADDING
#pragma omp parallel for schedule(guided)
        for (int i = 0; i < nRow; i++) {
            double yv_tmp = 0;
            int begin = row_ptr[i];
            int end = row_ptr[i+1];
            int begin_seg = begin / W;
            int end_seg = end / W;
            if (begin_seg == end_seg) {
                int j_begin = begin & (W-1);
                int j_end = end & (W-1);
                /*
                for (int j = begin; j < end; j++) {
                    yv_tmp += *(val[0] + j);
                }
                */
                double* restrict val_tmp = val[begin_seg];
                for (int j = j_begin; j < j_end; j++) {
                    yv_tmp += val_tmp[j];
                }
            } else {
                // upper
                if (begin & (W-1)) {
                    int j_end = (begin & ~(W-1)) + W;
                    for (int j = begin; j < j_end; j++) {
                        yv_tmp += *(val[0] + j);
                    }
                    begin = (begin & ~(W-1)) + W;
                }
                // lower
                if (end & (W-1)) {
                    int j_end = end & ~(W-1);
                    for (int j = end; j > j_end; j--) {
                        yv_tmp += *(val[0] + j - 1);
                    }
                    end = end & ~(W-1);
                }
                // center
                if (begin != end) {
                    for (int j = 0; j < W; j++) {
                        yv_tmp += *(val[0] + begin + j);
                    }
                }
            }
            yv[i] = yv_tmp;
        }
#else // PADDING
#pragma omp parallel for
        for (int i = 0; i < nRow; i++) {
            double yv_tmp = 0;
            int begin = row_ptr[i];
            int end = row_ptr[i+1];
            int begin_seg = begin / W;
            int end_seg = end / W;
            if (begin_seg == end_seg) {
                int j_begin = begin & (W-1);
                int j_end = end & (W-1);
                for (int j = j_begin; j < j_end; j++) {
                    yv_tmp += val[begin_seg][j];
                }
            } else {
                // upper
                if (begin & (W-1)) {
                    int j_begin = begin & (W-1);
                    for (int j = j_begin; j < W; j++) {
                        yv_tmp += val[begin_seg][j];
                    }
                    begin = (begin & ~(W-1)) + W;
                }
                // lower
                if (end & (W-1)) {
                    int j_begin = end & (W-1);
                    for (int j = j_begin; j > 0; j--) {
                        yv_tmp += val[end_seg][j-1];
                    }
                    end = end & ~(W-1);
                }
                // center
                begin_seg = begin / W;
                if (begin != end) {
                    for (int j = 0; j < W; j++) {
                        yv_tmp += val[begin_seg][j];
                    }
                }
            }
            yv[i] = yv_tmp;
        }
#endif // PADDING
        //}}}
#endif
    }
}



