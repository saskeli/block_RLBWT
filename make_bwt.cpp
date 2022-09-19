#include <iostream>
#include <cstring>
#include <fstream>

#include "include/reader.hpp"
#include "include/block_rlbwt_builder.hpp"
#include "include/byte_block.hpp"
#include "include/simple_rlbwt.hpp"

static const constexpr uint32_t BLOCK_SIZE = 1 << 12;

typedef bbwt::byte_block block;
typedef bbwt::simple_rlbwt<block, BLOCK_SIZE> rlbwt;
typedef bbwt::block_rlbwt_builder<block, BLOCK_SIZE, rlbwt> builder;

void help() {
    std::cout << "Create RLBWT data structure and output it to file.\n\n";
    std::cout << "Usage: make_rlbwt [-i file_name] [-h heads] [-r runs] <out_file>\n";
    std::cout << "   <out_file>     Path to file to write output to.\n";
    std::cout << "   -i file_name   File containing text to encode. (overrides -h and -r.)\n";
    std::cout << "   -h heads       File containing run heads as bytes.\n";
    std::cout << "   -r runs        File containing run lengths as 32-bit integers.\n\n";
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
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-i") == 0) {
            in_file_loc = ++i;
        } else if (strcmp(argv[i], "-h") == 0) {
            heads_loc = ++i;
        } else if (strcmp(argv[i], "-r") == 0) {
            runs_loc = ++i;
        } else {
            out_file_loc = i;
        }
    }
    if (out_file_loc == 0) {
        std::cerr << "output file is required" << std::endl;
    }
    builder b;
    uint64_t size = 0;
    if (in_file_loc) {
        std::ifstream in(argv[in_file_loc]);
        bbwt::file_reader reader(&in);
        for (auto it : reader) {
            b.append(it.head, it.length);
            size += it.length;
        }
    } else if (runs_loc || heads_loc) {
        if (runs_loc == 0 && heads_loc == 0) {
            std::cerr << "Both heads and run lengths are required" << std::endl;
            help();
        }
        bbwt::multi_reader reader(std::fopen(argv[heads_loc], "rb"), std::fopen(argv[runs_loc], "rb"));
        for (auto it : reader) {
            b.append(it.head, it.length);
            size += it.length;
        }
    } else {
        bbwt::file_reader reader(&std::cin);
        for (auto it : reader) {
            b.append(it.head, it.length);
            size += it.length;
        }
    }
    rlbwt bwt = b.compile();
    for (uint64_t i = 0; i < size; i++) {
        bwt.at(i);
    }
    std::cout << std::endl;
    return 0;
}