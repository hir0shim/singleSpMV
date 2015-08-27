#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cstring>
#include <cmath>
#include <sys/time.h>
#include <stdlib.h>
#include "util.h"

using namespace std;
struct Element {
    int row, col;
    double val;
    Element () {}
    Element (int r, int c, double v) : row(r), col(c), val(v) {}
    bool operator < (const Element &e) const { 
        if (row == e.row) return col < e.col;
        return row < e.row;
    }
};


double GetTimeBySec () {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + tv.tv_usec * 1e-6;
}
string GetBasename (const string &path) {
    size_t p = path.rfind("/");
    return path.substr(p+1);
}
void LoadSparseMatrix (SpMat &A, const string &matFile) {
    ifstream ifs(matFile.c_str());
    if (!ifs.is_open()) {
        std::cerr << "File not Found" << std::endl;
        exit(1);
    }
    std::string line;
    do {
        std::getline(ifs, line);
    } while (line[0] == '%');
    std::stringstream ss(line);
    int M, N, L;
    ss >> M >> N >> L;
    std::vector<Element> elements(L); 
    for (int i = 0; i < L; i++) {
        int row, col;
        double val;
        ifs >> row >> col >> val;
        row--; col--;
        elements[i] = Element(row, col, val);
    }
    std::sort(elements.begin(), elements.end());
    int *row_idx = new int[L];
    int *col_idx = new int[L];
    double *val = new double[L];
    for (int i = 0; i < L; i++) {
        row_idx[i] = elements[i].row;
        col_idx[i] = elements[i].col;
        val[i] = elements[i].val;
    }
    A.nRow = M;
    A.nCol = N;
    A.nNnz = L;
    A.row_idx = row_idx;
    A.col_idx = col_idx;
    A.val = val;
}
bool VerifyResult (const SpMat &A, const Vec &x, const Vec &y) {
    double *res = new double[A.nRow];
    memset(res, 0, sizeof(double)*A.nRow);
    for (int i = 0; i < A.nNnz; i++) {
        res[A.row_idx[i]] += A.val[i] * x.val[A.col_idx[i]];
    }
    for (int i = 0; i < A.nRow; i++) {
        double relative_error = fabs(fabs(res[i] - y.val[i]) / res[i]);
        double absolute_error = fabs(res[i]-y.val[i]);
        const double EPS = 1e-6;
        if (absolute_error > EPS && relative_error > EPS)  {
            fprintf(stderr, "Error: %lf != %lf\n", res[i], y.val[i]);
            return false;
        }
    }
    return true;
}

void ViewVec (const Vec &v) {
    cerr << "Printing Vec ... ";
    for (int i = 0; i < v.size; i++) 
        cout << i << " " << v.val[i] << endl;
    cerr << "done" << endl;
}

Vec CreateRandomVector (int size) {
    Vec x;
    x.size = size;
    x.val = new double[size];
    for (int i = 0; i < size; i++) {
        x.val[i] = double(rand()) / RAND_MAX;
    }
    return x;
}
