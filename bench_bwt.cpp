//#define VERB

#include <iostream>
#include <vector>
#include <random>
#include <chrono>

#include "include/reader.hpp"
#include "include/types.hpp"

void help() {
    std::cout << "Benchmark RLBWT data structure.\n\n";
    std::cout << "Usage: bench_bwt file_name_a sequence\n";
    std::cout << "   file_name_a    Path to block rlbwt root element.\n";
    std::cout << "   sequence       Path to file containing query sequence.\n\n";
    std::cout << "Input files is required.\n"
              << "Statistics will be output to std::cerr and rank query results sto std::cout.\n\n";
    std::cout << "Example: bench_bwt bwt.bin queries.bin >> /dev/null" << std::endl;
    exit(0);
}

int main(int argc, char const* argv[]) {
    if (argc < 3) {
        std::cerr << "Input files are required\n" << std::endl;
        help();
    }
    std::string in_file_path = "";
    std::string sequence_path = "";
    for (int i = 1; i < argc; i++) {
        if (in_file_path.size() == 0) {
            in_file_path = argv[i];
        } else {
            sequence_path = argv[i];
        }
    }
    if (in_file_path.size() == 0 || sequence_path.size() == 0) {
        std::cerr << "Input files are required\n" << std::endl;
        help();
    }
    bbwt::vbyte<> bwt(in_file_path);

    std::cerr << "Testing " << in_file_path << " with " << sequence_path << std::endl;

    using std::chrono::duration_cast;
    using std::chrono::high_resolution_clock;
    using std::chrono::nanoseconds;
    std::cout << "c\ti\tdense\trank\ttime" << std::endl;

    uint64_t i;
    uint8_t c;
    bool dense;

    std::fstream in_file;
    in_file.open(sequence_path, std::ios::binary | std::ios::in);
    uint64_t total = 0;
    while (in_file.good()) {
        in_file.read(reinterpret_cast<char*>(&i), sizeof(uint64_t));
        in_file.read(reinterpret_cast<char*>(&c), sizeof(uint8_t));
        in_file.read(reinterpret_cast<char*>(&dense), sizeof(bool));
        uint64_t r_a = bwt.rank(i, c);
        total += r_a;
    }

    double nanos_dense = 0;
    uint32_t count_dense = 0;
    double nanos_sparse = 0;
    uint32_t count_sparse = 0;

    in_file.close();
    in_file.open(sequence_path, std::ios::binary | std::ios::in);
    while (in_file.good()) {
        in_file.read(reinterpret_cast<char*>(&i), sizeof(uint64_t));
        in_file.read(reinterpret_cast<char*>(&c), sizeof(uint8_t));
        in_file.read(reinterpret_cast<char*>(&dense), sizeof(bool));
        std::cout << int(c) << "\t" << i << "\t" << dense << "\t";
        auto start = high_resolution_clock::now();
        uint64_t r_a = bwt.rank(i, c);
        auto end = high_resolution_clock::now();
        double time = duration_cast<nanoseconds>(end - start).count();
        std::cout << r_a << "\t" << time << "\n";
        total -= r_a;
        if (dense) {
            count_dense++;
            nanos_dense += time;
        } else {
            count_sparse++;
            nanos_sparse += time;
        }
    }

    assert(total == 0);

    std::cerr << (nanos_dense + nanos_sparse) / (count_dense + count_sparse) << " mean query time\n" 
              << nanos_dense / (count_dense ? count_dense : 1) << " mean dense query time (" << count_dense << ")\n"
              << nanos_sparse / (count_sparse ? count_sparse : 1) << " mean sparse query time (" << count_sparse << ")" << std::endl;
}