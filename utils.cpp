//
// Created by wmcool on 2021/1/20.
//

#include <string>
#include <fstream>
#include <ostream>

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