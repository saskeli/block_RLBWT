//#define VERB

#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <cassert>
#include <fstream>

#include "include/reader.hpp"
#include "include/types.hpp"


int main(int argc, char const* argv[]) {
    if (argc < 2) {
        std::cerr << "Path required" << std::endl;
        exit(1);
    }
    std::string path(argv[1]);

    std::ifstream bwt_file;
    bwt_file.open(path, std::ios_base::in | std::ios_base::binary);
    bbwt::file_reader<bbwt::custom_alphabet<uint64_t>> reader(&bwt_file);

    uint64_t consumed_runs = 0;
    uint64_t symbols_consumed = 0;
    for (auto r : reader) {
        std::cout << int(r.head) << ", " << r.length << std::endl;
        consumed_runs++;
        symbols_consumed += r.length;
    }
    std::cerr << "Consumed a total of " << symbols_consumed << " symbols in " << consumed_runs << " runs." << std::endl;
    return 0;    
}