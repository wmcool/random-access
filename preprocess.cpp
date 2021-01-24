//
// Created by wmcool on 2021/1/20.
//

#include <fstream>
#include <cmath>
#include <unordered_set>
#include <string>
#include <vector>
#include "RabinHash.h"
#include <cmath>

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
            in.read(cur_chunk.data(), cur_chunk.size());
            dic.insert(rh(cur_chunk.data()));
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
        matrix[i][i] = 0;
        for(int j=i+1;j<n;j++) {
            matrix[i][j] = estimate_correlation(chunks, i, j);
            matrix[j][i] = matrix[i][j];
        }
    }
    return matrix;
}



