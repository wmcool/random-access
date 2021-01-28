//
// Created by wmcool on 2021/1/27.
//

#ifndef RANDOM_AACCESS_COMPRESS_H
#define RANDOM_AACCESS_COMPRESS_H

void permute(std::vector<bool>& chunk, std::vector<int> I);

void unpermute(std::vector<bool>& chunk, std::vector<int> I);

int rabin_hash(std::vector<bool>& base);

std::vector<bool> elias_gamma(int l);

#endif //RANDOM_AACCESS_COMPRESS_H
