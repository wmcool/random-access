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

int binary_to_int(std::vector<bool> binary) {
    int x = 0;
    for(int i=0;i<binary.size();i++) {
        x <<= 1;
        x |= binary[i];
    }
    return x;
}

int read_elias_gamma(std::ifstream& in) {
    std::uint8_t byte = 0;
    int n = 0;
    int x = 0;
    int y = 0;
    bool readx = false;
    while(true) {
        in.read((char*)&byte, 1);
        for(int k=7;k>=0;k--) {
            if(!readx) {
                if(((byte >> k) & 1) == 1) n++;
                else {
                    y = 1 << n;
                    readx = true;
                }
            }else if(n > 0 && readx){
                x <<= 1;
                x |= (byte >> k) & 1;
                n--;
                if(n == 0) break;
            }
        }
        if(n == 0) break;
    }
    return y + x;
}

std::vector<bool> read_binary(std::ifstream& in, int length) {
    std::uint8_t byte = 0;
    std::vector<bool> binary;
    while(true) {
        in.read((char*)&byte, 1);
        for(int k=7;k>=0;k--) {
            binary.push_back((byte >> k) & 1);
            length--;
            if(length == 0) break;
        }
        if(length == 0) break;
    }
    return binary;
}