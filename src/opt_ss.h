#pragma once
#include "util.h"
struct SpMatOpt {
    int nRow;
    int nCol;
    int nNnz;

    const int W = ALIGNMENT / sizeof(double) * 2;
    int H;
    int *row_ptr;
    int **row_idx;
    int **col_idx;
    bool **flag;
    double **val;

    // for fast reduce
    int **index;
    int *segment_index;
    //int max_index;

    // for fast reduce part 2
    int nStep;
    int **sum_segs;
    int *sum_segs_count;


};
struct VecOpt {
    int size;
    double *val;
};
void OptimizeProblem (const SpMat &A, const Vec &x, SpMatOpt &A_opt, VecOpt &x_opt);
extern "C" {
void SpMV (const SpMatOpt &A, const VecOpt &x, Vec &y);
}
