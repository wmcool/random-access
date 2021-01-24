//
// Created by wmcool on 2021/1/20.
//

#ifndef RANDOM_ACCESS_PREPROCESS_H
#define RANDOM_ACCESS_PREPROCESS_H

#include <fstream>
#include <vector>

int compute_sample_num(std::ifstream& in, int ns, double d);

std::vector<std::vector<bool>> extract_chunks(std::ifstream& in, int ns, int c);

double** estimate_correlation_matrix(const std::vector<std::vector<bool>>& chunks);

#endif //RANDOM_ACCESS_PREPROCESS_H
