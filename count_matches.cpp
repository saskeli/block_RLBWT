//#define VERB

#include <iostream>
#include <fstream>
#include <vector>
#include <random>
#include <chrono>

#include "include/reader.hpp"
#include "include/types.hpp"

void help() {
    std::cout << "count matches in RLBWT data structure.\n\n";
    std::cout << "Usage: count_matches [options] <bwt_file> <patterns> <p_len>\n";
    std::cout << "   bwt_file   Path to block rlbwt element.\n";
    std::cout << "   patterns   Path to file containing patterns.\n";
    std::cout << "   p_len      Length of patterns.\n";
    std::cout << "   -s         Block rlbwt is space optimized.\n";
    std::cout << "   -c         Blocks contains a constant number of runs.\n";
    std::cout << "   -t         Don't include query times in std::cout\n";
    std::cout << "Bwt and pattern files are required.\n\n";
    std::cout << "Example: count_matches bwt.bin Einstein.txt >> /dev/null" << std::endl;
    exit(0);
}

template <class bwt_type>
std::pair<double, size_t> bench(const std::string& in_file_path, std::ifstream& patterns, bool o_t, double& bps, uint16_t p_len) {
    using std::chrono::duration_cast;
    using std::chrono::high_resolution_clock;
    using std::chrono::nanoseconds;

    bwt_type bwt(in_file_path);
    bps = 8 * double(bwt.bytes()) / bwt.size();
    double total = 0;
    std::string p(p_len, '\0');
    size_t i = 0;
    while (patterns.read(p.data(), p_len)) {
        auto start = high_resolution_clock::now();
        uint64_t count = bwt.count(p);
        auto end = high_resolution_clock::now();
        double time = duration_cast<nanoseconds>(end - start).count();
        if (o_t) {
            std::cout << p << "\t" << count << "\t" << time << std::endl;
        } else {
            std::cout << p << "\t" << count << std::endl;
        }
        total += time;
        i++;
    }
    return {total, i};
}

int main(int argc, char const* argv[]) {
    if (argc < 4) {
        std::cerr << "Input and pattern files, and pattern length are required\n" << std::endl;
        help();
    }
    std::string in_file_path = "";
    std::string patterns = "";
    uint16_t p_len = 0;
    bool space_op = false;
    bool run_block = false;
    bool output_time = true;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-s") == 0) {
            space_op = true;
        } else if (strcmp(argv[i], "-c") == 0) {
            run_block = true;
        } else if (strcmp(argv[i], "-t") == 0) {
            output_time = false;
        } else if (in_file_path.size() == 0) {
            in_file_path = argv[i];
        } else if (patterns.size() == 0) {
            patterns = argv[i];
        } else {
            std::sscanf(argv[i], "%u", &p_len);
        }
    }
    if (p_len < 1) {
        std::cerr << "invalid pattern length" << std::endl;
        exit(1);
    }
    std::ifstream p(patterns);
    std::cerr << "looking for patterns from " << patterns << " in " << in_file_path << std::endl;
    std::cout << "Pattern\tcount\ttime" << std::endl;
    std::pair<double, size_t> res;
    double bps = 0;
    if (run_block) {
        res = bench<bbwt::run<>>(in_file_path, p, output_time, bps, p_len);
    } else if (space_op) {
        res = bench<bbwt::vbyte<>>(in_file_path, p, output_time, bps, p_len);
    } else {
        res = bench<bbwt::two_byte<>>(in_file_path, p, output_time, bps, p_len);
    }
    std::cerr << "Mean query time: " << res.first << " / " << res.second << " = " << (res.first / res.second) << "ns\n" 
              << " with " << bps << " bits per symbol" << std::endl;
}