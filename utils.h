//
// Created by wmcool on 2021/1/20.
//

#ifndef RANDOM_ACCESS_UTILS_H
#define RANDOM_ACCESS_UTILS_H

#include <string>

void convert_binary(const std::string& input, const std::string& output);

void write_binary(std::ofstream& out, std::vector<bool> binary);

std::vector<bool> int_to_binary(int x, int length);

#endif //RANDOM_ACCESS_UTILS_H
