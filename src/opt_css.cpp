#include "opt_css.h"
#include "util.h"
#include "param.h"
#include <algorithm>
#include <vector>
#include <utility>
#include <cassert>
#include <iostream>
#include <cmath>
struct Block {
    vector<double> val;
    vector<int> col_idx;
    vector<int> row_idx;
};

extern vector<double> g_profile;
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

    int W = SEGMENT_WIDTH;
    int B = ceil((double)nCol / N_BLOCK);

    int nBlock = nCol/B + (nCol%B?1:0);
    vector<Block> blocks(nBlock);
    for (int i = 0; i < nNnz; i++) {
        int r = row_idx[i];
        int c = col_idx[i];
        double v = val[i];
        blocks[c/B].row_idx.push_back(r);
        blocks[c/B].col_idx.push_back(c);
        blocks[c/B].val.push_back(v);
    }

    int *H = (int*)_mm_malloc(nBlock*sizeof(int), ALIGNMENT);
    int **row_ptr = (int**)_mm_malloc(nBlock*sizeof(int*), ALIGNMENT);
    int ***row_idx_2d = (int ***)_mm_malloc(nBlock*sizeof(int**), ALIGNMENT);
    idx_t ***col_idx_2d = (idx_t***)_mm_malloc(nBlock*sizeof(idx_t**), ALIGNMENT);
    double ***val_2d = (double***)_mm_malloc(nBlock*sizeof(double**), ALIGNMENT);
    double ***val_buf_2d = (double***)_mm_malloc(nBlock*sizeof(double**), ALIGNMENT);

    int totalH = 0;
    for (int b = 0; b < nBlock; b++) {
        int block_nNnz = blocks[b].val.size();
        H[b] = block_nNnz/W + (block_nNnz%W?1:0);
        totalH += H[b];
    }
    double* val_ptr = (double*)_mm_malloc(totalH*W*sizeof(double), ALIGNMENT);
    double* val_buf_ptr = (double*)_mm_malloc(totalH*W*sizeof(double), ALIGNMENT);
    int* row_idx_ptr = (idx_t*)_mm_malloc(totalH*W*sizeof(int), ALIGNMENT);
    idx_t* col_idx_ptr = (idx_t*)_mm_malloc(totalH*W*sizeof(idx_t), ALIGNMENT);

    for (int b = 0; b < nBlock; b++) {
        int block_nNnz = blocks[b].val.size();
        row_ptr[b] = (int*)_mm_malloc((nRow+1)*sizeof(int), ALIGNMENT);
        row_idx_2d[b] = (int**)_mm_malloc(H[b]*sizeof(int*), ALIGNMENT);
        col_idx_2d[b] = (idx_t**)_mm_malloc(H[b]*sizeof(idx_t*), ALIGNMENT);
        val_2d[b] = (double**)_mm_malloc(H[b]*sizeof(double*), ALIGNMENT);
        val_buf_2d[b] = (double**)_mm_malloc(H[b]*sizeof(double*), ALIGNMENT);

        int N = H[b] * W;
        /*
           col_idx_2d[b][0] = (idx_t*)_mm_malloc(N*sizeof(idx_t), ALIGNMENT);
           val_2d[b][0] = (double*)_mm_malloc(N*sizeof(double), ALIGNMENT);
           */

        for (int i = 0; i < H[b]; i++) {
            row_idx_2d[b][i] = row_idx_ptr;
            col_idx_2d[b][i] = col_idx_ptr;
            val_2d[b][i] = val_ptr;
            val_buf_2d[b][i] = val_buf_ptr;
            row_idx_ptr += W;
            col_idx_ptr += W;
            val_ptr += W;
            val_buf_ptr += W;
        }

        row_ptr[b][0] = 0;
        int cur_row = 0;
        for (int i = 0; i < H[b]; i++) {
            for (int j = 0; j < W; j++) {
                int p = i*W + j;
                if (p < block_nNnz) {
                    row_idx_2d[b][i][j] = blocks[b].row_idx[p];
                    col_idx_2d[b][i][j] = blocks[b].col_idx[p];
                    val_2d[b][i][j] = blocks[b].val[p];
                    val_buf_2d[b][i][j] = blocks[b].val[p];
                    int r = blocks[b].row_idx[p];
                    while (cur_row <= r) { row_ptr[b][cur_row++] = p; }
                } else {
                    row_idx_2d[b][i][j] = 0;
                    col_idx_2d[b][i][j] = 0;
                    val_2d[b][i][j] = 0;
                    val_buf_2d[b][i][j] = 0;
                }
            }
        }
        while (cur_row <= nRow) row_ptr[b][cur_row++] = block_nNnz;
    }
    // Segment index
    int *nStep = new int[nBlock];
    int **sum_segs_count = (int **)_mm_malloc(nBlock*sizeof(int*), ALIGNMENT);
    int ***sum_segs = (int***)_mm_malloc(nBlock*sizeof(int**), ALIGNMENT);
    for (int b = 0; b < nBlock; b++) {
        int *segment_index = (int *)_mm_malloc(H[b]*sizeof(int), ALIGNMENT);
        segment_index[0] = 0;
        for (int i = 1; i < H[b]; i++) {
            bool same = true;
            if (row_idx_2d[b][i-1][0] == row_idx_2d[b][i][0]) {
                for (int j = 1; j < W; j++) {
                    if (row_idx_2d[b][i][j-1] != row_idx_2d[b][i][j]) same = false;
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
        int max_index = *max_element(segment_index, segment_index + H[b]);
        nStep[b] = int(ceil(log2(max_index+1)));
        int counter = 1 << nStep[b];
        sum_segs[b] = (int **)_mm_malloc(nStep[b]*sizeof(int*), ALIGNMENT);
        sum_segs_count[b] = (int *)_mm_malloc(nStep[b]*sizeof(int), ALIGNMENT);
        for (int i = 0; i < nStep[b]; i++) {
            counter >>= 1;
            int nSeg = 0;
            for (int j = 0; j < H[b]; j++) {
                if (counter <= segment_index[j] && segment_index[j] < counter*2) {
                    nSeg++;
                }
            }
            sum_segs_count[b][i] = nSeg;
            sum_segs[b][i] = (int *)_mm_malloc(nSeg*sizeof(int), ALIGNMENT);
            int idx = 0;
            for (int j = 0; j < H[b]; j++) {
                if (counter <= segment_index[j] && segment_index[j] < counter*2) {
                    sum_segs[b][i][idx++] = j;
                }
            }
            assert(idx == nSeg);
        }
    }
    A_opt.nStep = nStep;
    A_opt.sum_segs_count = sum_segs_count;
    A_opt.sum_segs = sum_segs;

    A_opt.nBlock = nBlock;
    A_opt.nRow = nRow;
    A_opt.nCol = nCol;
    A_opt.nNnz = nNnz;
    A_opt.H = H;
    A_opt.row_ptr = row_ptr;
    A_opt.col_idx = col_idx_2d;
    A_opt.val = val_2d;
    A_opt.val_buf = val_buf_2d;
    A_opt.B = B;
    A_opt.totalH = totalH;

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
        const int W = SEGMENT_WIDTH; 
        int B = A.B;
        int nBlock = A.nBlock;
        int* restrict H = A.H;
        idx_t*** restrict col_idx = A.col_idx;
        double*** restrict val = A.val;
        double*** restrict val_buf = A.val_buf;
        int** restrict row_ptr = A.row_ptr;
        int* restrict nStep = A.nStep;
        int*** restrict sum_segs = A.sum_segs;
        int** restrict sum_segs_count = A.sum_segs_count;

#ifdef SIMPLE
        //{{{
        // Mul
        PROF_BEGIN(g_profile[0]);
        int totalH = A.totalH;
#pragma omp parallel for
        for (int i = 0; i < totalH; i++) {
#pragma ivdep
            for (int j = 0; j < W; j++) {
                *(val_buf[0][0] + i*W+j) = *(val[0][0]+i*W+j) * xv[*(col_idx[0][0]+i*W+j)];
            }
        }
        PROF_END(g_profile[0]);
        // Sum
        PROF_BEGIN(g_profile[1]);
#pragma omp parallel for
        for (int i = 0; i < nRow; i++) yv[i] = 0;
        for (int b = 0; b < nBlock; b++) {
#pragma omp parallel for
            for (int i = 0; i < nRow; i++) {
#pragma ivdep
                for (int j = row_ptr[b][i]; j < row_ptr[b][i+1]; j++) {
                    yv[i] += *(val_buf[b][0]+j);
                }
            }
        }
        PROF_END(g_profile[1]);
        //}}}
#elif OPTIMIZED
        //{{{
        // Mul
        
        PROF_BEGIN(g_profile[0]);
        int totalH = A.totalH;
#pragma omp parallel for
        for (int i = 0; i < totalH; i++) {
#pragma ivdep
            for (int j = 0; j < W; j++) {
                *(val_buf[0][0] + i*W+j) = *(val[0][0]+i*W+j) * xv[*(col_idx[0][0]+i*W+j)];
            }
        }
        PROF_END(g_profile[0]);
        // Sum
        PROF_BEGIN(g_profile[1]);
#pragma omp parallel for
        for (int i = 0; i < nRow; i++) yv[i] = 0;

        for (int b = 0; b < nBlock; b++) {
            int counter = 1<<nStep[b];
            for (int s = 0; s < nStep[b]; s++) {
                counter >>= 1;
#pragma omp parallel for
                for (int i = 0; i < sum_segs_count[b][s]; i++) {
                    int h = sum_segs[b][s][i];
#pragma ivdep
                    //#pragma vector aligned
                    for (int j = 0; j < W; j++) {
                        val_buf[b][h-counter][j] += val_buf[b][h][j];
                    }
                }
            }

#pragma omp parallel for
            for (int i = 0; i < nRow; i++) {
                double yv_tmp = 0;
                int begin = row_ptr[b][i];
                int end = row_ptr[b][i+1];
                int begin_seg = begin / W;
                int end_seg = end / W;
                if (begin_seg == end_seg) {
                    int j_begin = begin & (W-1);
                    int j_end = end & (W-1);
                    double* restrict val_tmp = val_buf[b][begin_seg];
                    for (int j = j_begin; j < j_end; j++) {
                        yv_tmp += val_tmp[j];
                    }
                } else {
                    // upper
                    if (begin & (W-1)) {
                        int j_end = (begin & ~(W-1)) + W;
                        for (int j = begin; j < j_end; j++) {
                            yv_tmp += *(val_buf[b][0] + j);
                        }
                        begin = (begin & ~(W-1)) + W;
                    }
                    // lower
                    if (end & (W-1)) {
                        int j_end = end & ~(W-1);
                        for (int j = end; j > j_end; j--) {
                            yv_tmp += *(val_buf[b][0] + j - 1);
                        }
                        end = end & ~(W-1);
                    }
                    // center
                    if (begin != end) {
                        for (int j = 0; j < W; j++) {
                            yv_tmp += *(val_buf[b][0] + begin + j);
                        }
                    }
                }
                yv[i] += yv_tmp;
            }
        }
        PROF_END(g_profile[1]);
        //}}}
#endif

    }
}



