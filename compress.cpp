//
// Created by wmcool on 2021/1/27.
//
#include <vector>
#include "compress.h"
#include "RabinHash.h"
#include <cmath>
#include <iostream>

void permute(std::vector<bool>& chunk, std::vector<int> I) {
    for(int move : I) {
        bool ori = chunk[move];
        chunk.erase(chunk.begin() + move);
        chunk.push_back(ori);
    }
}

void unpermute(std::vector<bool>& chunk, std::vector<int> I) {
    int n = I.size();
    int m = chunk.size();
    for(int i=n-1;i>=0;i--) {
        bool tmp = chunk[m-1];
        chunk.erase(chunk.begin() + m-1);
        chunk.insert(chunk.begin() + I[i], tmp);
    }
}

int rabin_hash(std::vector<bool>& base) {
    RabinHash rh;
    std::vector<char> cur;
    int i = 0;
    int cnt = 0;
    while(i < base.size()) {
        std::uint8_t byte = 0;
        for(int k=0;k<8;k++,i++) {
            byte <<= 1;
            byte = byte | base[i];
            if(i == base.size()) break;
        }
        cur.push_back(byte);
        cnt++;
    }
    return rh(cur.data(), cnt);
}

std::vector<bool> elias_gamma(int l) {
    std::vector<bool> res;
    if(l == 0) {
        res.push_back(1);
        return res;
    }
    int n = std::floor(std::log2(l));
    for(int i=0;i<n;i++) {
        res.push_back(1);
    }
    res.push_back(0);
    int k = l - std::pow(2, n);
    for(int i=n-1;i>=0;i--) {
        if(k >= (1 << i)) {
            res.push_back(1);
            k -= (1 << i);
        }else res.push_back(0);
    }
    return res;
}
