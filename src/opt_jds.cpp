#include "opt_jds.h"
#include "util.h"
#include <algorithm>
#include <vector>
#include <utility>
#include <cassert>
#include <iostream>
struct E {
    int col;
    double val;
    E (int c, double v) : col(c), val(v) {}
};
void OptimizeProblem (const SpMat &A, const Vec &x, SpMatOpt &A_opt, VecOpt &x_opt) {
    int nRow = A.nRow;
    int nCol = A.nCol;
    int nNnz = A.nNnz;
    vector<vector<E>> G(nRow);
    {
        int *row_idx = A.row_idx;
        int *col_idx = A.col_idx;
        double *val = A.val;
        for (int i = 0; i < nNnz; i++) {
            G[row_idx[i]].emplace_back(col_idx[i], val[i]);
        }
    }
    {
        int *perm = new int[nRow];
        int *col_idx = new int[nNnz];
        int *length = new int[nRow];
        int maxLength = 0;
        double *val = new double[nNnz];
        for (int i = 0; i < nRow; i++) {
            perm[i] = i;
            length[i] = G[i].size();
            maxLength = max<int>(maxLength, G[i].size());
        }
        int *ptr = new int[maxLength+1];
        sort(perm, perm + nRow, 
                [&](int r1, int r2)
                {
                return G[r1].size() > G[r2].size();
                }
            );
        int p = 0;
        for (int c = 0; c < maxLength; c++) {
            ptr[c] = p;
            for (int i = 0; i < nRow; i++) {
                int r = perm[i];
                if (c < G[r].size()) {
                    col_idx[p] = G[r][c].col;
                    val[p] = G[r][c].val;
                    p++;
                } else break;
            }
        }
        ptr[maxLength] = p;
        assert(p == nNnz);

        A_opt.nRow = nRow;
        A_opt.nCol = nCol;
        A_opt.nNnz = nNnz;
        A_opt.col_idx = col_idx;
        A_opt.ptr = ptr;
        A_opt.perm = perm;
        A_opt.val = val;
        A_opt.length = length;
        A_opt.maxLength = maxLength;
    }

    x_opt.size = x.size;
    x_opt.val = x.val;

}
extern "C" {
void SpMV (const SpMatOpt &A, const VecOpt &x, Vec &y) {
    double *xv = x.val;
    double *yv = y.val;
    int nRow = A.nRow;
    int nNnz = A.nNnz;
    int *ptr = A.ptr;
    int *col_idx = A.col_idx;
    double *val = A.val;
    int *perm = A.perm;
    int *length = A.length;
    int maxLength = A.maxLength;
    for (int i = 0; i < nRow; i++) yv[i] = 0;
#pragma omp parallel for
    for (int r = 0; r < nRow; r++) {
        int row = perm[r];
        for (int i = 0; i < length[row]; i++) {
            int idx = ptr[i] + r;
            int col = col_idx[idx];
            double lv = val[idx];
            double rv = xv[col];
            double val = lv * rv;
            yv[row] += val;
        }
    }
}
}



