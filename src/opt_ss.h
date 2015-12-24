#pragma once
#include "util.h"
#include "param.h"
struct SpMatOpt {
    int nRow;
    int nCol;
    int nNnz;

    const int W = SEGMENT_WIDTH;
    int H;
    int *row_ptr;
    int **row_idx;
    idx_t **col_idx;
    double **val;

    // for fast reduce
    int nSeg;
    int **index;
    int *segment_index;
    //int max_index;

    // for fast reduce part 2
    int nStep;
    int **sum_segs;
    int *sum_segs_count;

    // for protecting matrix
    double **val_buf;

};
struct VecOpt {
    int size;
    double *val;
};
void OptimizeProblem (const SpMat &A, const Vec &x, SpMatOpt &A_opt, VecOpt &x_opt);
extern "C" {
void SpMV (const SpMatOpt &A, const VecOpt &x, Vec &y);
}
