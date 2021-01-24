#include <iostream>
#include "utils.h"
#include <fstream>
#include "preprocess.h"
#include <bitset>
#include "RabinHash.h"

using namespace std;

int main() {
//    convert_binary("TX_SOUID100434_valid.txt", "TX_SOUID100434");
//    convert_binary("test.txt", "test");
    ifstream in("test", ios::in | ios::binary);
    int c = compute_sample_num(in, 32, 0.01);
    cout << "c: " << c << endl;
    vector<vector<bool>> chunks = extract_chunks(in, 32, c);
    for(int i=0;i<chunks.size();i++) {
        cout << "chunks " << i << ": ";
        for(int j=0;j<chunks[i].size();j++) {
            cout << (int)chunks[i][j] << " ";
        }
        cout << endl;
    }
    double** correlation_matrix = estimate_correlation_matrix(chunks);
    for(int i=0;i<chunks[0].size();i++) {
        cout << "bit " << i << ":";
        for(int j=0;j<chunks[0].size();j++) {
            cout << correlation_matrix[i][j] << " ";
        }
        cout << endl;
    }
    return 0;
}
