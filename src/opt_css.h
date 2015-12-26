#pragma once
#include "util.h"
#include "param.h"
struct SpMatOpt {
    int nRow;
    int nCol;
    int nNnz;

    const int W = SEGMENT_WIDTH;

    int B;
    int nBlock;
    int *H;
    int **row_ptr; // [block][row]
    idx_t ***col_idx; // [block][vi][vj]
    double ***val;

    // for collapse
    int totalH;

    // for fast reduction
    int *nStep;
    int **sum_segs_count;
    int ***sum_segs;


    // for protecting matrix
    double ***val_buf;

};
struct VecOpt {
    int size;
    double *val;
};
void OptimizeProblem (const SpMat &A, const Vec &x, SpMatOpt &A_opt, VecOpt &x_opt);
extern "C" {
void SpMV (const SpMatOpt &A, const VecOpt &x, Vec &y);
}
