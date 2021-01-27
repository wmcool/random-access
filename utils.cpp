//
// Created by wmcool on 2021/1/20.
//

#include <string>
#include <fstream>
#include <ostream>
#include <vector>

void convert_binary(const std::string& input, const std::string& output) {
    std::ifstream in(input);
    std::ofstream out(output, std::ios::out | std::ios::binary);
    int cur;
    while(in >> cur) {
        out.write((char*)&cur, sizeof(int));
    }
    in.close();
    out.close();
}

void write_binary(std::ofstream& out, std::vector<bool> binary) {
    std::vector<char> bytes;
    int n = binary.size();
    int j = 0;
    int cnt = 0;
    while(j < n) {
        std::uint8_t byte = 0;
        for(int k=0;k<8;k++,j++) {
            byte <<= 1;
            if(j < n) byte = byte | binary[j];
        }
        bytes.push_back(byte);
        cnt++;
    }
    out.write(bytes.data(), cnt);
}

std::vector<bool> int_to_binary(int x, int length) {
    std::vector<bool> binary;
    for(int i=length-1;i>=0;i--) {
        binary.push_back((x>>i) & 1);
    }
    return binary;
}