
#include "opt_css.h"
#include "util.h"
#include <algorithm>
#include <vector>
#include <utility>
#include <cassert>
#include <iostream>
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

    int B = A_opt.B;
    int W = A_opt.W;

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

    int *H = new int[nBlock];
    int **row_ptr = new int*[nBlock];
    int ***row_idx_2d = new int**[nBlock];
    int ***col_idx_2d = new int**[nBlock];
    double ***val_2d = new double**[nBlock];
    bool ***flag_2d = new bool**[nBlock];

    for (int b = 0; b < nBlock; b++) {
        int block_nNnz = blocks[b].val.size();
        H[b] = block_nNnz/W + (block_nNnz%W?1:0);
        row_ptr[b] = new int[nRow+1];
        row_idx_2d[b] = new int*[H[b]];
        col_idx_2d[b] = new int*[H[b]];
        val_2d[b] = new double*[H[b]];
        flag_2d[b] = new bool*[H[b]];

        int N = H[b] * W;
        row_idx_2d[b][0] = new int[N];
        col_idx_2d[b][0] = new int[N];
        val_2d[b][0] = new double[N];
        flag_2d[b][0] = new bool[N];

        for (int i = 1; i < H[b]; i++) {
            row_idx_2d[b][i] = row_idx_2d[b][i-1] + W;
            col_idx_2d[b][i] = col_idx_2d[b][i-1] + W;
            val_2d[b][i] = val_2d[b][i-1] + W;
            flag_2d[b][i] = flag_2d[b][i-1] + W;
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
                    int r = blocks[b].row_idx[p];
                    while (cur_row <= r) { row_ptr[b][cur_row++] = p; }
                } else {
                    row_idx_2d[b][i][j] = H[b];
                    col_idx_2d[b][i][j] = 0;
                    val_2d[b][i][j] = 0;
                }
            }
            while (cur_row <= nRow) row_ptr[b][cur_row++] = block_nNnz;
        }
    }
    
    A_opt.nBlock = nBlock;
    A_opt.nRow = nRow;
    A_opt.nCol = nCol;
    A_opt.nNnz = nNnz;
    A_opt.H = H;
    A_opt.row_ptr = row_ptr;
    A_opt.row_idx = row_idx_2d;
    A_opt.col_idx = col_idx_2d;
    A_opt.flag = flag_2d;
    A_opt.val = val_2d;
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
        int W = A.W;
        int B = A.B;
        int nBlock = A.nBlock;
        int* restrict H = A.H;
        int*** restrict col_idx = A.col_idx;
        double*** restrict val = A.val;
        int** restrict row_ptr = A.row_ptr;

        for (int b = 0; b < nBlock; b++) {
            for (int i = 0; i < H[b]; i++) {
                for (int j = 0; j < W; j++) {
                    val[b][i][j] *= xv[col_idx[b][i][j]];
                }
            }
        }
        for (int i = 0; i < nRow; i++) yv[i] = 0;
        for (int b = 0; b < nBlock; b++) {
            for (int i = 0; i < nRow; i++) {
                for (int j = row_ptr[b][i]; j < row_ptr[b][i+1]; j++) {
                    yv[i] += *(val[b][0] + j);
                }
            }
        }
    }
}



