#include <iostream>
#include <algorithm>
#include <sstream>
#include <numeric>
#include <vector>
#include <cstdlib>
#include <cassert>
#include <cmath>
using namespace std;
int main () {
    string line;
    do {
        getline(cin, line);
    } while (line[0] == '%');
    stringstream ss(line);
    int N, M, L;
    ss >> N >> M >> L;
    assert(N == M);
    vector<int> cntr(M);
    vector<int> cntc(N);
    for (int i = 0; i < L; i++) {
        int r, c;
        double v;
        cin >> r >> c >> v;
        r--; c--;
        cntr[r]++;
        cntc[c]++;
    }
    double var = 0;
    double ave = (double)accumulate(cntr.begin(), cntr.end(), 0) / N;
    for (int i = 0; i < N; i++) {
        var += (cntr[i] - ave) * (cntr[i]-ave) / N;
    }

    cout << N << " " 
        << M << " " 
        << L << " " 
        << *max_element(cntr.begin(), cntr.end()) << " "
        << *min_element(cntr.begin(), cntr.end()) << " "
        << *max_element(cntc.begin(), cntc.end()) << " "
        << *min_element(cntc.begin(), cntc.end()) << " "
        << var << endl;

    return 0;
}
