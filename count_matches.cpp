//#define VERB

#include <iostream>
#include <vector>
#include <random>
#include <chrono>

#include "include/reader.hpp"
#include "include/types.hpp"

void help() {
    std::cout << "count matches in RLBWT data structure.\n\n";
    std::cout << "Usage: count_matches [-s] bwt_file patterns\n";
    std::cout << "   bwt_file   Path to block rlbwt root element.\n";
    std::cout << "   patterns   Space delimited sequence of patterns.\n";
    std::cout << "   -s         Block rlbwt is space optimized.\n";
    std::cout << "Bwt file and at least one pattern are required.\n\n";
    std::cout << "Example: count_matches bwt.bin Einstein" << std::endl;
    exit(0);
}

template <class bwt_type>
double bench(const std::string& in_file_path, const std::vector<std::string>& patterns) {
    using std::chrono::duration_cast;
    using std::chrono::high_resolution_clock;
    using std::chrono::nanoseconds;

    bwt_type bwt(in_file_path);
    double total = 0;

    for (auto p : patterns) {
        auto start = high_resolution_clock::now();
        uint64_t count = bwt.count(p);
        auto end = high_resolution_clock::now();
        double time = duration_cast<nanoseconds>(end - start).count();
        std::cout << p << "\t" << count << "\t" << time << std::endl;
        total += time;
    }
    return total;
}

int main(int argc, char const* argv[]) {
    if (argc < 3) {
        std::cerr << "Input file and pattern are required\n" << std::endl;
        help();
    }
    std::string in_file_path = "";
    std::vector<std::string> patterns;
    bool space_op = false;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-s") == 0) {
            space_op = true;
        } else if (in_file_path.size() == 0) {
            in_file_path = argv[i];
        } else {
            patterns.push_back({argv[i]});
        }
    }
    std::cerr << "looking for " << patterns.size() << " patterns in " << in_file_path << std::endl;
    std::cout << "Pattern\tcount\ttime" << std::endl;
    double t;
    if (space_op) {
        t = bench<bbwt::vbyte<>>(in_file_path, patterns);
    } else {
        t = bench<bbwt::two_byte<>>(in_file_path, patterns);
    }
    std::cerr << "Mean query time: " << (t / patterns.size()) << "ns" << std::endl;
}