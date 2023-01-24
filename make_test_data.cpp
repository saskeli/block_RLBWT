//#define VERB

#include <iostream>
#include <vector>
#include <random>
#include <chrono>

#include "include/reader.hpp"
#include "include/types.hpp"

void help() {
    std::cout << "Create query patterns by LF mapping bwt.\n\n"
        << "Usage: make_test_data <bwt_path> <n> <k>\n"
        << "   <bwt_path>     Path to file with bwt.\n"
        << "   <n>            Number of patterns to generate\n"
        << "   <k>            Pattern length.\n\n";
    std::cout 
        << "All parameters are required.\n"
        << "bwt file is expected to be two-byte encoded.\n"
        << "Patterns will be composed of ACGT symbols.\n"
        << "First line of pattern file will be the number of patterns\n"
        << "If bwt contains insufficient substring matching alphabet and "
        << "pattern length the process will no terminate.\n\n";
    std::cout << "Example: make_test_data covid.tb_bwt 10000 10 > patterns.txt" << std::endl;
}

int main(int argc, char const* argv[]) {
    if (argc < 4) {
        help();
        exit(1);
    }
    size_t p_len, n;
    std::string in_file_path(argv[1]);
    std::sscanf(argv[2], "%lu", &n);
    std::sscanf(argv[3], "%lu", &p_len);
    bbwt::two_byte<> bwt(in_file_path);

    std::mt19937 mt(1337);
    std::uniform_int_distribution<unsigned long long> gen(0, bwt.size());
    char* chars = (char*)std::malloc(p_len + 1);
    chars[p_len] = '\0';
    std::cout << n << std::endl;
    for (size_t i = 0; i < n; i++) {
        uint64_t idx = gen(mt);
        for (size_t p_idx = p_len - 1; p_idx < p_len; p_idx--) {
            chars[p_idx] = bwt[idx];
            idx = bwt.LF(idx);
            
        }
        std::cout << chars << std::endl;
        std::cerr << "\r" << i + 1 << " patterns created";
    }
    std::cerr << std::endl;
    return 0;    
}