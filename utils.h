//
// Created by wmcool on 2021/1/20.
//

#ifndef RANDOM_ACCESS_UTILS_H
#define RANDOM_ACCESS_UTILS_H

#include <string>

void convert_binary(const std::string& input, const std::string& output);

void write_binary(std::ofstream& out, std::vector<bool> binary);

std::vector<bool> read_binary(std::ifstream& in, int length);

std::vector<bool> int_to_binary(int x, int length);

int binary_to_int(std::vector<bool> binary);

int read_elias_gamma(std::ifstream& in);

void read_all_files(std::vector<std::string>& files, std::string dir_name);

#endif //RANDOM_ACCESS_UTILS_H
