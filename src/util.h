#pragma once
#include <string>
using namespace std;

// COO foramt
struct SpMat {
    int nRow, nCol, nNnz;
    int *row_idx;
    int *col_idx;
    double *val;
    /*
    ~SpMat () {
        delete [] row_idx;
        delete [] col_idx;
        delete [] val;
    }
    */
};
struct Vec {
    int size;
    double *val;
    /*
    ~Vec () {
        delete [] val;
    }
    */
};

struct Element {
    int row, col;
    double val;
    Element () {}
    Element (int r, int c, double v) : row(r), col(c), val(v) {}
    inline bool operator < (const Element &e) const { 
        if (row == e.row) return col < e.col;
        return row < e.row;
    }
};
void LoadSparseMatrix (SpMat &A, const string &matFile); 
double GetTimeBySec ();
bool VerifyResult (const SpMat &A, const Vec &x, const Vec &y);
string GetBasename (const string &path);
Vec CreateRandomVector (int size);
void ViewVec (const Vec &v);


#define CUDA_SAFE_CALL(func) \
    do { \
        cudaError_t err = (func); \
        if (err != cudaSuccess) { \
            fprintf(stderr, "[Error] %s (error code: %d) at %s line %d\n", cudaGetErrorString(err), err, __FILE__, __LINE__); \
            exit(err); \
        } \
    } while(0)
