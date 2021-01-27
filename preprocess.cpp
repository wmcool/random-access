//
// Created by wmcool on 2021/1/20.
//

#include <fstream>
#include <cmath>
#include <unordered_set>
#include <string>
#include <vector>
#include "RabinHash.h"
#include <iostream>
#include "preprocess.h"
#include <algorithm>

using namespace std;

int compute_sample_num(std::ifstream& in, int ns, double d) {
    int c = 0;
    int U = 0;
    in.seekg(0, std::ios::end);
    int size = in.tellg();
    in.seekg(0, std::ios::beg);
    int NT = size / (ns / 8);
    int Nc;
    RabinHash rh;
    do {
        c++;
        Nc = std::ceil(NT / c);
        in.seekg(0, std::ios::beg);
        std::unordered_set<int> dic;
        int num_bytes = c*ns/8;
        std::vector<char> cur_chunk(num_bytes, '0');
        for(int i=0;i<Nc;i++) {
            in.read(cur_chunk.data(), num_bytes);
            dic.insert(rh(cur_chunk.data(), num_bytes));
            if(size - in.tellg() < num_bytes) break;
        }
        if(size != in.tellg()) {
            in.read(cur_chunk.data(), size-in.tellg());
            dic.insert(rh(cur_chunk.data()));
        }
        U = dic.size();
    }while(U < (double)(1-d) * Nc);
    return c;
}

std::vector<std::vector<bool>> extract_chunks(std::ifstream& in, int ns, int c) {
    in.seekg(0, std::ios::end);
    int size = in.tellg();
    in.seekg(0, std::ios::beg);
    int num_bytes = ns*c/8;
    std::vector<std::vector<bool>> res;
    while(in.tellg() != size) {
        std::vector<bool> cur_chunk;
        std::uint8_t byte;
        for(int i=0;i<num_bytes;i++) {
            if(in.tellg() != size) {
                in.read((char*)&byte, 1);
                for(int j=7;j>=0;j--) {
                    cur_chunk.push_back((byte >> j) & 1);
                }
            }else {
                for(int j=7;j>=0;j--) {
                    cur_chunk.push_back(0);
                }
            }
        }
        res.push_back(cur_chunk);
    }
    return res;
}

double estimate_correlation(const std::vector<std::vector<bool>>& chunks, int i, int j) {
    double sum_xy = 0;
    double sum_x = 0;
    double sum_y = 0;
    int n = chunks.size();
    for(int k=0;k<n;k++) {
        sum_xy += chunks[k][i] & chunks[k][j]; // times
        sum_x += chunks[k][i];
        sum_y += chunks[k][j];
    }
    if(sum_x == 0 || sum_y == 0) return 1;
    double cor = ((n * sum_xy) - sum_x * sum_y) / (std::sqrt(n * sum_x - sum_x * sum_x) * std::sqrt(n * sum_y - sum_y * sum_y));
    return cor;
}

double** estimate_correlation_matrix(const std::vector<std::vector<bool>>& chunks) {
    double **matrix = nullptr;
    if(chunks.empty() || chunks[0].empty()) return matrix;
    int n = chunks[0].size();
    matrix = new double*[n];
    for(int i=0;i<n;i++) {
        matrix[i] = new double[n];
    }
    for(int i=0;i<n;i++) {
        matrix[i][i] = 1;
        for(int j=i+1;j<n;j++) {
            matrix[i][j] = estimate_correlation(chunks, i, j);
            matrix[j][i] = matrix[i][j];
        }
    }
    return matrix;
}

double* get_mean_correlation(double** correlation_matrix, int n) {
    double* correlation_array = new double[n];
    for(int i=0;i<n;i++) {
        double sum = 0;
        for(int j=0;j<n;j++) {
            sum += std::abs(correlation_matrix[i][j]);
        }
        correlation_array[i] = sum / n;
    }
    return correlation_array;
}

void swap(std::vector<bool>& chunk, int i, int j) {
    bool temp = chunk[i];
    chunk[i] = chunk[j];
    chunk[j] = temp;
}

void swap(double* array, int i, int j) {
    double temp = array[i];
    array[i] = array[j];
    array[j] = temp;
}

void swap(int* array, int i, int j) {
    int temp = array[i];
    array[i] = array[j];
    array[j] = temp;
}

int* sort_bits_of_chunks(std::vector<std::vector<bool>>& chunks, double* correlation_array) {
    int n = chunks[0].size();
    int c = chunks.size();
    int* index = new int[n];
    for(int i=0;i<n;i++) {
        index[i] = i;
    }
    for(int i=0;i<n-1;i++) {
        for(int j=n-1;j>i;j--) {
            if(correlation_array[j] > correlation_array[j-1]) {
                swap(correlation_array, j, j-1);
                swap(index, j, j-1);
                for(int k=0;k<c;k++) {
                    swap(chunks[k], j, j-1);
                }
            }
        }
    }
    return index;
}

int compute_unique_chunks_num(std::vector<std::vector<bool>>& chunks) {
    std::unordered_set<int> dic;
    int c = chunks.size();
    int n = chunks[0].size();
    RabinHash rh;
    for(int i=0;i<c;i++) {
        int j = 0;
        std::vector<char> cur;
        while(j < n) {
            std::uint8_t byte = 0;
            for(int k=0;k<8;k++,j++) {
                byte <<= 1;
                byte = byte | chunks[i][j];
            }
            cur.push_back(byte);
        }
        dic.insert(rh(cur.data(), n/8));
    }
    return dic.size();
}

int compute_deviation_bits_num(std::vector<std::vector<bool>>& chunks) {
    int c = chunks.size();
    int n = chunks[0].size();
    int base = n;
    double S_k;
    double pre_S_k;
    while(base >= 0) {
        int K = compute_unique_chunks_num(chunks);
        S_k = K * base + c * (std::ceil(std::log2(K)) + (n - base));
//        std::cout << "base = " << base <<": " << S_k << std::endl;
        if(base != n && S_k - pre_S_k > (pre_S_k / 100)) return n - base - 1;
        pre_S_k = S_k;
        base--;
        if(base >= 0) {
            for(int i=0;i<c;i++) {
                chunks[i][base] = (int)0;
            }
        }
    }
}

bool comp(int a, int b) {
    return a > b;
}

std::vector<int> compute_move_index_set(const int* indexs, int deviation, int n) {
    std::vector<int> original_index;
    for(int i=n-deviation;i<n;i++) {
        original_index.push_back(indexs[i]);
    }
    std::sort(original_index.begin(), original_index.end(), comp);
    return original_index;
}

//int main() {
////    convert_binary("TX_SOUID100434_valid.txt", "TX_SOUID100434");
////    convert_binary("test.txt", "test");
//    ifstream in("test", ios::in | ios::binary);
//    int c = compute_sample_num(in, 32, 0.01);
//    cout << "c: " << c << endl;
//    vector<vector<bool>> chunks = extract_chunks(in, 32, c);
//    for(int i=0;i<chunks.size();i++) {
//        cout << "chunks " << i << ": ";
//        for(int j=0;j<chunks[i].size();j++) {
//            cout << (int)chunks[i][j] << " ";
//        }
//        cout << endl;
//    }
//    double** correlation_matrix = estimate_correlation_matrix(chunks);
////    for(int i=0;i<chunks[0].size();i++) {
////        cout << "bit " << i << ":";
////        for(int j=0;j<chunks[0].size();j++) {
////            cout << correlation_matrix[i][j] << " ";
////        }
////        cout << endl;
////    }
//    double* correlation_array = get_mean_correlation(correlation_matrix, chunks[0].size());
//    for(int i=0;i<chunks[0].size();i++) {
//        cout<< correlation_array[i] << " ";
//    }
//    cout << endl;
//    cout << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>" << endl;
//    int* indexs = sort_bits_of_chunks(chunks, correlation_array);
//    cout << "after sort: " << endl;
//    for(int i=0;i<chunks.size();i++) {
//        cout << "chunks " << i << ": ";
//        for(int j=0;j<chunks[i].size();j++) {
//            cout << (int)chunks[i][j] << " ";
//        }
//        cout << endl;
//    }
//    cout << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>" << endl;
//    int k = compute_deviation_bits_num(chunks);
//    cout << "deviation bits num: " << k << endl;
//    vector<int> original_index = compute_move_index_set(indexs, k, chunks[0].size());
//    cout << "deviation index: ";
//    for(int i=0;i<original_index.size();i++) {
//        cout << original_index[i] << " ";
//    }
//    return 0;
//}



