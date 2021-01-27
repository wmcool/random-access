#include "cmdline.h"
#include "preprocess.h"
#include <iostream>
#include "utils.h"

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
        // preprocess
        ifstream in(name, ios::in | ios::binary);
        int c = compute_sample_num(in, ns, d);
        cout << "c: " << c << endl;
        vector<vector<bool>> chunks = extract_chunks(in, 32, c);
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
    }else if(target == "directory") {
        // TODO
    }
    return 0;
}
