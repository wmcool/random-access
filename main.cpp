#include "cmdline.h"
#include "preprocess.h"
#include <iostream>
#include "compress.h"
#include "utils.h"
#include <unordered_map>
#include <cmath>
#include <ostream>

using namespace std;

int main(int argc, char *argv[]) {
    cmdline::parser parser;
    parser.add<string>("target", 't', "compression target", true, "",
                       cmdline::oneof<string>("file", "directory"));
    parser.add<string>("name", 'n', "file or directory name", true, "");
    parser.add<int>("ns", 's', "bits per sample", false, 32);
    parser.add<double>("ratio", 'd', "overlap ratio", false, 0.01);
    parser.parse_check(argc, argv);
    string target = parser.get<string>("target");
    string name = parser.get<string>("name");
    int ns = parser.get<int>("ns");
    double d = parser.get<double>("ratio");
    if(target == "file") {
        ofstream out(name + ".rac", ios::out | ios::binary);

        // preprocess
        ifstream in(name, ios::in | ios::binary);
        int c = compute_sample_num(in, ns, d);
        cout << "c: " << c << endl;
        vector<vector<bool>> chunks = extract_chunks(in, ns, c);
        double** correlation_matrix = estimate_correlation_matrix(chunks);
        double* correlation_array = get_mean_correlation(correlation_matrix, chunks[0].size());
        int* indexs = sort_bits_of_chunks(chunks, correlation_array);
        int k = compute_deviation_bits_num(chunks);
        cout << "deviation bits num: " << k << endl;
        vector<int> original_index = compute_move_index_set(indexs, k, chunks[0].size());
        cout << "deviation index: ";
        for(int i=0;i<original_index.size();i++) {
            cout << original_index[i] << " ";
        }
        cout << endl;

        // write params to file
        ofstream out_param(name + ".par", ios::out | ios::binary);
        write_binary(out_param, elias_gamma(ns));
        write_binary(out_param, elias_gamma(c));
        for(int i=0;i<original_index.size();i++) write_binary(out_param, elias_gamma(original_index[i]));
        out_param.close();

        // compress
        vector<vector<bool>> original_chunks = extract_chunks(in, ns, c);
        unordered_map<int, int> dic;
        int base_id = 0;
        vector<vector<bool>> dic_content;
        for(int i=0;i<original_chunks.size();i++) {
            permute(original_chunks[i], original_index);
            vector<bool> base(original_chunks[i].begin(), original_chunks[i].end()-k);
            int rh = rabin_hash(base);
            if(dic.count(rh)) continue;
            dic.emplace(rh, base_id++);
            dic_content.push_back(base);
        }

        // write dictionary to file
        ofstream out_dic(name + ".dic", ios::out | ios::binary);
        for(int i=0;i<dic_content.size();i++) {
            write_binary(out_dic, dic_content[i]);
        }
        out_dic.close();

        int l = ceil(log2(dic_content.size()));
        if(l == 0) l = 1; // if all bases are identical, we need 1 bit wo code base
        vector<bool> l_elias = elias_gamma(l);
        write_binary(out, l_elias);
        for(int i=0;i<original_chunks.size();i++) {
            vector<bool> base(original_chunks[i].begin(), original_chunks[i].end()-k);
            int rh = rabin_hash(base);
            int id =dic[rh];
            vector<bool> base_binary = int_to_binary(id, l);
            write_binary(out, base_binary);
            vector<bool> deviation(original_chunks[i].end()-k, original_chunks[i].end());
            write_binary(out, deviation);
        }
        out.close();
        in.close();
    } else if(target == "directory") {
        // TODO
    }
    return 0;
}