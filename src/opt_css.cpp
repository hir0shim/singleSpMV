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
    idx_t ***col_idx_2d = (idx_t***)_mm_malloc(nBlock*sizeof(idx_t**), ALIGNMENT);
    double ***val_2d = (double***)_mm_malloc(nBlock*sizeof(double**), ALIGNMENT);

    int totalH = 0;
    for (int b = 0; b < nBlock; b++) {
        int block_nNnz = blocks[b].val.size();
        H[b] = block_nNnz/W + (block_nNnz%W?1:0);
        totalH += H[b];
    }
    double* val_ptr = (double*)_mm_malloc(totalH*W*sizeof(double), ALIGNMENT);
    idx_t* col_idx_ptr = (idx_t*)_mm_malloc(totalH*W*sizeof(idx_t), ALIGNMENT);

    for (int b = 0; b < nBlock; b++) {
        int block_nNnz = blocks[b].val.size();
        H[b] = block_nNnz/W + (block_nNnz%W?1:0);
        row_ptr[b] = (int*)_mm_malloc((nRow+1)*sizeof(int), ALIGNMENT);
        col_idx_2d[b] = (idx_t**)_mm_malloc(H[b]*sizeof(idx_t*), ALIGNMENT);
        val_2d[b] = (double**)_mm_malloc(H[b]*sizeof(double*), ALIGNMENT);

        int N = H[b] * W;
        /*
        col_idx_2d[b][0] = (idx_t*)_mm_malloc(N*sizeof(idx_t), ALIGNMENT);
        val_2d[b][0] = (double*)_mm_malloc(N*sizeof(double), ALIGNMENT);
        */

        for (int i = 0; i < H[b]; i++) {
            col_idx_2d[b][i] = col_idx_ptr;
            val_2d[b][i] = val_ptr;
            col_idx_ptr += W;
            val_ptr += W;
        }

        row_ptr[b][0] = 0;
        int cur_row = 0;
        for (int i = 0; i < H[b]; i++) {
            for (int j = 0; j < W; j++) {
                int p = i*W + j;
                if (p < block_nNnz) {
                    col_idx_2d[b][i][j] = blocks[b].col_idx[p];
                    val_2d[b][i][j] = blocks[b].val[p];
                    int r = blocks[b].row_idx[p];
                    while (cur_row <= r) { row_ptr[b][cur_row++] = p; }
                } else {
                    col_idx_2d[b][i][j] = 0;
                    val_2d[b][i][j] = 0;
                }
            }
        }
        while (cur_row <= nRow) row_ptr[b][cur_row++] = block_nNnz;
        //cout << b << "/" << nBlock << " " << total << "/" << nNnz << " " << H[b] << "*" << W << "=" << H[b]*W << ">=" << block_nNnz << endl;
    }

    A_opt.nBlock = nBlock;
    A_opt.nRow = nRow;
    A_opt.nCol = nCol;
    A_opt.nNnz = nNnz;
    A_opt.H = H;
    A_opt.row_ptr = row_ptr;
    A_opt.col_idx = col_idx_2d;
    A_opt.val = val_2d;
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
        const int W = SEGMENT_WIDTH; //ALIGNMENT / sizeof(double); 
        int B = A.B;
        int nBlock = A.nBlock;
        int* restrict H = A.H;
        idx_t*** restrict col_idx = A.col_idx;
        double*** restrict val = A.val;
        int** restrict row_ptr = A.row_ptr;

#ifdef SIMPLE
        //{{{
#pragma omp parallel for
        for (int b = 0; b < nBlock; b++) {
            for (int i = 0; i < H[b]; i++) {
                for (int j = 0; j < W; j++) {
                    val[b][i][j] *= xv[col_idx[b][i][j]];
                }
            }
        }

#pragma omp parallel for
        for (int i = 0; i < nRow; i++) yv[i] = 0;
        for (int b = 0; b < nBlock; b++) {
#pragma omp parallel for
            for (int i = 0; i < nRow; i++) {
                for (int j = row_ptr[b][i]; j < row_ptr[b][i+1]; j++) {
                    yv[i] += *(val[b][0]+j);
                }
            }
        }
    }
    //}}}
#elif OPTIMIZED
    //{{{
    int totalH = A.totalH;
    for (int i = 0; i < totalH; i++) {
        for (int j = 0; j < W; j++) {
            *(val[0][0]+i*W+j) *= xv[*(col_idx[0][0]+i*W+j)];
        }
    }
    // TODO:Summation
    //}}}
#endif

}



