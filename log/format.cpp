#include <map>
#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <cstdlib>
#include <sstream>
#include <vector>
using namespace std;
void PrintData (map<string, string> &data) {
    cout <<        data["Matrix"] 
        << "\t" << data["Architecture"]
        << "\t" << data["MatrixFormat"]
        << "\t" << data["Performance(GFLOPS)"]
        << "\t" << data["nRow"]
        << "\t" << data["nCol"]
        << "\t" << data["nNnz"]
        << endl;
}
int main (int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <log file>\n", argv[0]);
        exit(1);
    }
    ifstream ifs(argv[1]);
    string line;
    string matrix;
    string performance;
    vector<map<string, string> > data;
    int p = 0;
    map<string, string> tmp;
    while (getline(ifs, line)) {
        if (line == "++++++++++++++++++++++++++++++++++++++++") {
            tmp.clear();
        }
        else if (line == "----------------------------------------") {
            data.push_back(tmp);
            continue;
        } else {
            stringstream ss(line);
            string key, val;
            ss >> key >> val;
            tmp[key] = val;
        }
    }
    sort(data.begin(), data.end(), [&](map<string, string> a, map<string, string> b) {if (atoi(a["nNnz"].c_str()) == atoi(b["nNnz"].c_str())) return a["Matrix"] < b["Matrix"]; return atoi(a["nNnz"].c_str()) < atoi(b["nNnz"].c_str()); });
    for (auto it : data) {
        PrintData(it);
    }
}
