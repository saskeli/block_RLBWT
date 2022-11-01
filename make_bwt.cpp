#include <iostream>
#include <cstring>
#include <fstream>

#include "include/debug.hpp"

#include "include/reader.hpp"
#include "include/types.hpp"

void help() {
    std::cout << "Create RLBWT data structure and output it to file.\n\n";
    std::cout << "Usage: make_rlbwt [-i file_name] [-h heads] [-r runs] <out_file>\n";
    std::cout << "   <out_file>     Path to file to write output to.\n";
    std::cout << "   -i file_name   File containing text to encode. (overrides -h and -r.)\n";
    std::cout << "   -h heads       File containing run heads as bytes.\n";
    std::cout << "   -r runs        File containing run lengths as 32-bit integers.\n";
    std::cout << "   -n             Strip new line characters from input.\n\n";
    std::cout << "Ouput file is required.\n"
              << "If no input file is given, input will be read from standard input.\n"
              << "If file_name is given, input will be read as a byte-stream from file.\n"
              << "If heads and runs are given, heads are read as byte-stream and runs as stream of 32-bit integers.\n\n";
    std::cout << "Example: make_rlbwt -i bwt.txt rlbwt.bin" << std::endl;
    exit(0);
}

int main(int argc, char const* argv[]) {
    if (argc < 2) {
        help();
    }
    
    size_t in_file_loc = 0;
    size_t heads_loc = 0;
    size_t runs_loc = 0;
    size_t out_file_loc = 0;
    bool strip_new_line = false;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-i") == 0) {
            in_file_loc = ++i;
        } else if (strcmp(argv[i], "-h") == 0) {
            heads_loc = ++i;
        } else if (strcmp(argv[i], "-r") == 0) {
            runs_loc = ++i;
        } else if (strcmp(argv[i], "-n") == 0) { 
            strip_new_line = true;
        } else {
            out_file_loc = i;
        }
    }
    if (out_file_loc == 0) {
        std::cerr << "output file is required" << std::endl;
    }
    bbwt::custom_rlbwt::builder b(argv[out_file_loc]);
    if (in_file_loc) {
        std::ifstream in(argv[in_file_loc]);
        bbwt::file_reader reader(&in);
        for (auto it : reader) {
            if (strip_new_line && (it.head == '\n')) {
                continue;
            }
            b.append(it.head, it.length);
        }
    } else if (runs_loc || heads_loc) {
        if (runs_loc == 0 || heads_loc == 0) {
            std::cerr << "Both heads and run lengths are required" << std::endl;
            help();
        }
        std::ifstream heads;
        std::ifstream runs;
        heads.open(argv[heads_loc], std::ios_base::in | std::ios_base::binary);
        runs.open(argv[runs_loc], std::ios_base::in | std::ios_base::binary);
        bbwt::multi_reader reader(&heads, &runs);
        for (auto it : reader) {
            if (strip_new_line && (it.head == '\n')) {
                continue;
            }
            b.append(it.head, it.length);
        }
    } else {
        bbwt::file_reader reader(&std::cin);
        for (auto it : reader) {
            if (strip_new_line && (it.head == '\n')) {
                continue;
            }
            b.append(it.head, it.length);
        }
    }
    b.finalize();
    std::cout << "a_blocks: " << bbwt::a_blocks << ", b_blocks: " << bbwt::b_blocks << std::endl;
    return 0;
}