#include "cmdline.h"
#include "preprocess.h"
#include <iostream>
#include "compress.h"
#include "utils.h"
#include <unordered_map>
#include <cmath>
#include <ostream>
#include <sys/stat.h>

using namespace std;

int main(int argc, char *argv[]) {
    cmdline::parser parser;
    parser.add<string>("action", 'a', "compress of decompress", true, "",
                       cmdline::oneof<string>("compress", "decompress", "access", "convert"));
    parser.add<string>("target", 't', "file or directory", true, "",
            cmdline::oneof<string>("file", "directory", "chunk", "bit"));
    parser.add<string>("name", 'n', "file or directory name", true, "");
    parser.add<int>("order", '\0', "the order of chunk or bit in the original file", false, 0);
    parser.add<string>("output", 'o', "output file or directory name", false, "");
    parser.add<int>("ns", 's', "bits per sample", false, 32);
    parser.add<double>("ratio", 'd', "overlap ratio", false, 0.01);
    parser.parse_check(argc, argv);
    string action = parser.get<string>("action");
    string target = parser.get<string>("target");
    string name = parser.get<string>("name");
    if(action == "convert") {
        string output_name = parser.get<string>("output");
        convert_binary(name, output_name);
    }
    if(action == "compress") {
        int ns = parser.get<int>("ns");
        double d = parser.get<double>("ratio");
        if(target == "file") {
            ofstream out(name + ".rac", ios::out | ios::binary);

            // preprocess
            ifstream in(name, ios::in | ios::binary);
            int c = compute_sample_num(in, ns, d);
            cout << "c: " << c << endl;
            int tail = 0;
            vector<vector<bool>> chunks = extract_chunks(in, ns, c, tail);
            double** correlation_matrix = estimate_correlation_matrix(chunks);
            double* correlation_array = get_mean_correlation(correlation_matrix, chunks[0].size());
            int* indexs = sort_bits_of_chunks(chunks, correlation_array);
            int k = compute_deviation_bits_num(chunks);
            cout << "deviation bits num: " << k << endl;
            vector<int> move_from = compute_move_index_set(indexs, k, chunks[0].size());
            cout << "deviation index: ";
            for(int i=0; i < move_from.size(); i++) {
                cout << move_from[i] << " ";
            }
            cout << endl;

            // write params to file
            ofstream out_param(name + ".par", ios::out | ios::binary);
            write_binary(out_param, elias_gamma(ns));
            write_binary(out_param, elias_gamma(c));
            // elias-gamma can't code 0, so all index plus 1
            for(int i=0; i < move_from.size(); i++) write_binary(out_param, elias_gamma(move_from[i] + 1));
            out_param.close();

            // compress
            vector<vector<bool>> original_chunks = extract_chunks(in, ns, c, tail);
            unordered_map<int, int> dic;
            int base_id = 0;
            vector<vector<bool>> dic_content;
            for(int i=0;i<original_chunks.size();i++) {
                permute(original_chunks[i], move_from);
                vector<bool> base(original_chunks[i].begin(), original_chunks[i].end()-k);
                int rh = rabin_hash(base);
                if(dic.count(rh)) continue;
                dic.emplace(rh, base_id++);
                dic_content.push_back(base);
            }

            int l = ceil(log2(dic_content.size()));
            if(l == 0) l = 1; // if all bases are identical, we need 1 bit wo code base
            vector<bool> l_elias = elias_gamma(l);
            write_binary(out, elias_gamma(tail + 1));
            write_binary(out, l_elias);
            for(int i=0;i<original_chunks.size();i++) {
                vector<bool> base(original_chunks[i].begin(), original_chunks[i].end()-k);
                int rh = rabin_hash(base);
                int id = dic[rh];
                vector<bool> base_binary = int_to_binary(id, l);
                write_binary(out, base_binary);
                vector<bool> deviation(original_chunks[i].end()-k, original_chunks[i].end());
                write_binary(out, deviation);
            }

            // write dictionary to file
            ofstream out_dic(name + ".dic", ios::out | ios::binary);
            for(int i=0;i<dic_content.size();i++) {
                write_binary(out_dic, dic_content[i]);
            }
            out_dic.close();
            out.close();
            in.close();
        } else if(target == "directory") {
            vector<string> files;
            read_all_files(files, name);
            string output_dir_name = parser.get<string>("output");
            const int dir_err = mkdir(output_dir_name.data(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
            if (-1 == dir_err)
            {
                printf("Error creating directory!n");
                exit(1);
            }

            // preprocess
            ifstream in_pre(name + "/" + files[0], ios::in | ios::binary);
            int c = compute_sample_num(in_pre, ns, d);
            cout << "c: " << c << endl;
            int tail = 0;
            vector<vector<bool>> chunks = extract_chunks(in_pre, ns, c, tail);
            double** correlation_matrix = estimate_correlation_matrix(chunks);
            double* correlation_array = get_mean_correlation(correlation_matrix, chunks[0].size());
            int* indexs = sort_bits_of_chunks(chunks, correlation_array);
            int k = compute_deviation_bits_num(chunks);
            cout << "deviation bits num: " << k << endl;
            vector<int> move_from = compute_move_index_set(indexs, k, chunks[0].size());
            cout << "deviation index: ";
            for(int i=0; i < move_from.size(); i++) {
                cout << move_from[i] << " ";
            }
            cout << endl;
            in_pre.close();

            // write params to file
            ofstream out_param(output_dir_name + "/" + output_dir_name + ".par", ios::out | ios::binary);
            write_binary(out_param, elias_gamma(ns));
            write_binary(out_param, elias_gamma(c));
            // elias-gamma can't code 0, so all index plus 1
            for(int i=0; i < move_from.size(); i++) write_binary(out_param, elias_gamma(move_from[i] + 1));
            out_param.close();

            // compress
            unordered_map<int, int> dic;
            int base_id = 0;
            vector<vector<bool>> dic_content;
            for(const string& filename : files) {
                ifstream in(name + "/" + filename, ios::in | ios::binary);
                ofstream out(output_dir_name + "/" + filename + ".rac", ios::out | ios::binary);

                vector<vector<bool>> original_chunks = extract_chunks(in, ns, c, tail);
                for(int i=0;i<original_chunks.size();i++) {
                    permute(original_chunks[i], move_from);
                    vector<bool> base(original_chunks[i].begin(), original_chunks[i].end()-k);
                    int rh = rabin_hash(base);
                    if(dic.count(rh)) continue;
                    dic.emplace(rh, base_id++);
                    dic_content.push_back(base);
                }

                int l = ceil(log2(dic_content.size()));
                if(l == 0) l = 1; // if all bases are identical, we need 1 bit wo code base
                vector<bool> l_elias = elias_gamma(l);
                write_binary(out, elias_gamma(tail + 1));
                write_binary(out, l_elias);
                for(int i=0;i<original_chunks.size();i++) {
                    vector<bool> base(original_chunks[i].begin(), original_chunks[i].end()-k);
                    int rh = rabin_hash(base);
                    int id = dic[rh];
                    vector<bool> base_binary = int_to_binary(id, l);
                    write_binary(out, base_binary);
                    vector<bool> deviation(original_chunks[i].end()-k, original_chunks[i].end());
                    write_binary(out, deviation);
                }
            }
            // write dictionary to file
            ofstream out_dic(output_dir_name + "/" + output_dir_name + ".dic", ios::out | ios::binary);
            for(int i=0;i<dic_content.size();i++) {
                write_binary(out_dic, dic_content[i]);
            }
            out_dic.close();
        }
    }else if(action == "decompress") {
        if(target == "file") {
            // restore param
            ifstream in_param(name + ".par", ios::in | ios::binary);
            in_param.seekg(0, ios::end);
            int size_param = in_param.tellg();
            in_param.seekg(0, ios::beg);
            int ns = read_elias_gamma(in_param);
            int c = read_elias_gamma(in_param);
            vector<int> move_from;
            while(in_param.tellg() != size_param) move_from.push_back(read_elias_gamma(in_param) - 1);
            in_param.close();
            int k = move_from.size();
            int base_length = c*ns - k;
            int chunk_size = c*ns / 8;

            // restore dictionary
            ifstream in_dic(name + ".dic", ios::in | ios::binary);
            in_dic.seekg(0, ios::end);
            int size_dic = in_dic.tellg();
            in_dic.seekg(0, ios::beg);
            vector<vector<bool>> dic_content;
            while(in_dic.tellg() != size_dic) dic_content.push_back(read_binary(in_dic, base_length));
            in_dic.close();

            // decompressing files
            ifstream in(name + ".rac", ios::in | ios::binary);
            ofstream  out(name + ".dec", ios::out | ios::binary);
            in.seekg(0, ios::end);
            int size = in.tellg();
            in.seekg(0, ios::beg);
            int tail = read_elias_gamma(in) - 1;
            int l = read_elias_gamma(in);
//            int compressed_chunk_size = ceil((double)l / 8) + ceil(k / 8);
            while(in.tellg() != size) {
                int base_id = binary_to_int(read_binary(in, l));
                vector<bool> base = dic_content[base_id];
                vector<bool> deviation = read_binary(in, k);
                base.insert(base.end(), deviation.begin(), deviation.end());
                unpermute(base, move_from);
                if(in.tellg() == size && tail != 0) {
                    vector<bool> chunks_tail(base.begin(), base.begin() + (chunk_size - tail) * 8);
                    write_binary(out, chunks_tail);
                    break;
                }
                write_binary(out, base);
            }
            in.close();
            out.close();
        }else if(target == "directory") {
            vector<string> files;
            read_all_files(files, name);
            auto iter = files.begin();
            while(iter != files.end()) {
                string s = (*iter);
                string suffix = s.substr(s.size() - 4, 4);
                if(suffix == ".par" || suffix == ".dic") iter = files.erase(iter);
                else iter++;
            }
            string output_dir_name = parser.get<string>("output");
            const int dir_err = mkdir(output_dir_name.data(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
            if (-1 == dir_err)
            {
                printf("Error creating directory!n");
                exit(1);
            }

            // restore param
            ifstream in_param(name + "/" + name + ".par", ios::in | ios::binary);
            in_param.seekg(0, ios::end);
            int size_param = in_param.tellg();
            in_param.seekg(0, ios::beg);
            int ns = read_elias_gamma(in_param);
            int c = read_elias_gamma(in_param);
            vector<int> move_from;
            while(in_param.tellg() != size_param) move_from.push_back(read_elias_gamma(in_param) - 1);
            in_param.close();
            int k = move_from.size();
            int base_length = c*ns - k;
            int chunk_size = c*ns / 8;

            // restore dictionary
            ifstream in_dic(name + "/" + name + ".dic", ios::in | ios::binary);
            in_dic.seekg(0, ios::end);
            int size_dic = in_dic.tellg();
            in_dic.seekg(0, ios::beg);
            vector<vector<bool>> dic_content;
            while(in_dic.tellg() != size_dic) dic_content.push_back(read_binary(in_dic, base_length));
            in_dic.close();

            for(const string& filename : files) {
                // decompressing files
                ifstream in(name + "/" + filename, ios::in | ios::binary);
                ofstream  out(output_dir_name + "/" + filename.substr(0, filename.size()-4) , ios::out | ios::binary);
                in.seekg(0, ios::end);
                int size = in.tellg();
                in.seekg(0, ios::beg);
                int tail = read_elias_gamma(in) - 1;
                int l = read_elias_gamma(in);
//            int compressed_chunk_size = ceil((double)l / 8) + ceil(k / 8);
                while(in.tellg() != size) {
                    int base_id = binary_to_int(read_binary(in, l));
                    vector<bool> base = dic_content[base_id];
                    vector<bool> deviation = read_binary(in, k);
                    base.insert(base.end(), deviation.begin(), deviation.end());
                    unpermute(base, move_from);
                    if(in.tellg() == size && tail != 0) {
                        vector<bool> chunks_tail(base.begin(), base.begin() + (chunk_size - tail) * 8);
                        write_binary(out, chunks_tail);
                        break;
                    }
                    write_binary(out, base);
                }
                in.close();
                out.close();
            }
        }
    }else if(action == "access") {
        int order = parser.get<int>("order");

        // restore param
        ifstream in_param(name + ".par", ios::in | ios::binary);
        in_param.seekg(0, ios::end);
        int size_param = in_param.tellg();
        in_param.seekg(0, ios::beg);
        int ns = read_elias_gamma(in_param);
        int c = read_elias_gamma(in_param);
        int n = c*ns;
        vector<int> move_from;
        while(in_param.tellg() != size_param) move_from.push_back(read_elias_gamma(in_param) - 1);
        int k = move_from.size();
        in_param.close();

        if(target == "chunk") {
            ifstream in(name + ".rac", ios::in | ios::binary);
            int tail = read_elias_gamma(in) - 1;
            int l = read_elias_gamma(in);
            int compressed_chunk_size = (int)ceil((double)l / 8) + (int)ceil((double)k / 8);
            in.seekg(order * compressed_chunk_size, ios::cur);
            int base_id = binary_to_int(read_binary(in, l));

            ifstream in_dic(name + ".dic", ios::in | ios::binary);
            int origin_base_size = ceil((double)(n - k)/8);
            in_dic.seekg(base_id * origin_base_size, ios::beg);
            vector<bool> base = read_binary(in_dic, n-k);
            vector<bool> deviation = read_binary(in, k);
            base.insert(base.end(), deviation.begin(), deviation.end());
            unpermute(base, move_from);
            cout << "chunk " << order << ": ";
            for(int i=0;i<base.size();i++) {
                cout << (int)base[i] << " ";
            }
            cout << endl;
            in.close();
            in_dic.close();
        }else if(target == "bit") {
            int order_in_chunk = order % n;
            int chunk_id = order_in_chunk / n;
            int j = -1;
            int padding_size = 0;
            for(int i=0;i<move_from.size();i++) {
                if(move_from[i] == order_in_chunk) {
                    j = i;
                    break;
                }
                if(move_from[i] < order_in_chunk) {
                    padding_size = k - i;
                    break;
                }

            }
            if(j != -1) { // deviation
                ifstream in(name + ".rac", ios::in | ios::binary);
                int tail = read_elias_gamma(in) - 1;
                int l = read_elias_gamma(in);
                int compressed_chunk_size = (int)ceil((double)l / 8) + (int)ceil((double)k / 8);
                in.seekg((chunk_id + 1) * compressed_chunk_size, ios::cur);
                in.seekg(((j - k + 1) / 8) - 1, ios::cur);
                vector<bool> bits = read_binary(in, 8);
                cout << "bit " << order << ":" << bits[j%8] << endl;
                in.close();
            }else { // base
                ifstream in(name + ".rac", ios::in | ios::binary);
                int tail = read_elias_gamma(in) - 1;
                int l = read_elias_gamma(in);
                int compressed_chunk_size = (int)ceil((double)l / 8) + (int)ceil((double)k / 8);
                in.seekg(chunk_id * compressed_chunk_size, ios::beg);
                int base_id = binary_to_int(read_binary(in, l));
                in.close();

                ifstream in_dic(name + ".dic", ios::in | ios::binary);
                int origin_base_size = ceil((double)(n - k)/8);
                in_dic.seekg(base_id * origin_base_size, ios::beg);
                int base_order = order_in_chunk - padding_size;
                in_dic.seekg(base_order / 8, ios::cur);
                vector<bool> bits = read_binary(in_dic, 8);
                cout << "bit " << order << ":" << bits[base_order % 8] << endl;
                in_dic.close();
            }
        }
    }
    return 0;
}