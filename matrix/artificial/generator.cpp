#include <iostream>
#include <set>
#include <cassert>
#include <vector>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <cmath>
using namespace std;
typedef pair<int,int> P;
typedef vector<P> M;
M band_matrix (int N, int width) {
    M m;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            if (abs(i-j) < width) m.push_back(P(i, j));
        }
    }
    return m;
}

M dense_matrix (int N) {
    M m;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            m.push_back(P(i, j));
        }
    }
    return m;
}

M unbalance_matrix (int N) {
    M m;
    for (int i = 0; i < sqrt(N); i++) {
        for (int j = 0; j < N; j++) {
            m.push_back(P(i, j));
        }
    }
    return m;
}

M random_unbalance_matrix (int N) {
    M m;
    for (int i = 0; i < sqrt(N); i++) {
        int r = rand() % N;
        for (int j = 0; j < N; j++) {
            m.push_back(P(r, j));
        }
    }
    return m;
}

M random_matrix (int N, int cnt) {
    assert(N*N >= cnt && "too many cnt");
    M m;
    set<P> s;
    for (int i = 0; i < cnt; i++) {
        int r = rand()%N;
        int c = rand()%N;
        while (s.count(P(r, c))) {
            r = rand()%N;
            c = rand()%N;
        }
        s.insert(P(r, c));
        m.push_back(P(r, c));
    }
    return m;
}

void print_matrix (int N, M m) {
    cout << "%%MatrixMarket matrix coordinate real general" << endl;
    cout << N << " " << N << " " << m.size() << endl;
    for (int i = 0; i < m.size(); i++) {
        cout << m[i].first+1 << " " << m[i].second+1 << " 1.0" << endl;
    }
}

int main (int argc, char* argv[]) {
    int N = 10000;
    int param = 10;
    if (argc <= 1) {
        printf("Usage: %s <type> [parameter] \n", argv[0]);
        exit(1);
    }
    if (argc >= 3) N = atoi(argv[2]);
    if (argc >= 4) param = atoi(argv[3]);
    if (strcmp(argv[1], "band") == 0) print_matrix(N, band_matrix(N, param));
    else if (strcmp(argv[1], "dense") == 0) print_matrix(N, dense_matrix(N));
    else if (strcmp(argv[1], "unbalance") == 0) print_matrix(N, unbalance_matrix(N));
    else if (strcmp(argv[1], "random_unbalance") == 0) print_matrix(N, random_unbalance_matrix(N));
    else if (strcmp(argv[1], "random") == 0) print_matrix(N, random_matrix(N, param));
    else {
        printf("%s: %s: type not found\n", argv[0], argv[1]);
    }
}

