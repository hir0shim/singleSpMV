#include "opt_dia.h"
#include "util.h"
#include <vector>
#include <iostream>
#include <cassert>
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
    int N = nRow + nCol - 1;
    int offset = nRow - 1;
    vector<vector<Element>> D(N);
    {
        for (int i = 0; i < nNnz; i++) {
            D[col_idx[i]-row_idx[i]+offset].emplace_back(row_idx[i], col_idx[i], val[i]);
        }
    }
    int nDiag = 0;
    for (int i = 0; i < N; i++) {
        if (D[i].size()) {
            nDiag++;
        }
    }
    int *ioff = new int[nDiag];
    int *ioff_rev = new int[N];
    int p = 0;
    for (int i = 0; i < N; i++) {
        ioff_rev[i] = -1;
        if (D[i].size()) {
            ioff[p] = i;
            ioff_rev[i] = p;
            p++;
        }
    }
    assert(p == nDiag);
    double **diag = new double*[nDiag];
    for (int i = 0; i < nDiag; i++) {
        diag[i] = new double[nCol];
        for (int j = 0; j < nCol; j++) diag[i][j] = 0;
    }
    for (int i = 0; i < nNnz; i++) {
        int d_idx = col_idx[i] - row_idx[i] + offset;
        diag[ioff_rev[d_idx]][col_idx[i]] = val[i];
        assert(ioff_rev[d_idx] != -1);
    }
    A_opt.nRow = nRow;
    A_opt.nCol = nCol;
    A_opt.nNnz = nNnz;
    A_opt.nDiag = nDiag;
    A_opt.ioff = ioff;
    A_opt.diag = diag;
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
        int nDiag = A.nDiag;
        int *ioff = A.ioff;
        double **diag = A.diag;
        int N = nRow + nCol - 1;
        int offset = nRow - 1;
        double *tmp = new double[N];
        for (int i = 0; i < N; i++) tmp[i] = 0;
        for (int i = 0; i < nRow; i++) yv[i] = 0;
        for (int i = 0; i < nDiag; i++) {
            for (int col = 0; col < nCol; col++) {
                int row = col + offset - ioff[i];
                if (row < 0 || row >= nRow) continue;
                double lv = diag[i][col];
                double rv = xv[col];
                yv[row] += lv * rv;
                //tmp[row + offset] += lv * rv;
            }
        }
        /*
        for (int i = 0; i < nRow; i++) {
            yv[i] = tmp[i + offset];
        }*/
    }
}



