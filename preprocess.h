//
// Created by wmcool on 2021/1/20.
//

#ifndef RANDOM_ACCESS_PREPROCESS_H
#define RANDOM_ACCESS_PREPROCESS_H

#include <fstream>
#include <vector>

int compute_sample_num(std::ifstream& in, int ns, double d);

std::vector<std::vector<bool>> extract_chunks(std::ifstream& in, int ns, int c, int& tail);

double** estimate_correlation_matrix(const std::vector<std::vector<bool>>& chunks);

double* get_mean_correlation(double** correlation_matrix, int n);

int* sort_bits_of_chunks(std::vector<std::vector<bool>>& chunks, double* correlation_array);

int compute_deviation_bits_num(std::vector<std::vector<bool>>& chunks);

std::vector<int> compute_move_index_set(const int* indexs, int deviation, int n);



#endif //RANDOM_ACCESS_PREPROCESS_H
