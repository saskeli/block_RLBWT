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
    assert(argc > 4);
    std::string heads_path(argv[1]);
    std::string lens_path(argv[2]);
    uint64_t i;
    std::sscanf(argv[3], "%lu", &i);
    char c = argv[4][0];

    std::ifstream heads;
    std::ifstream runs;
    heads.open(heads_path, std::ios_base::in | std::ios_base::binary);
    runs.open(lens_path, std::ios_base::in | std::ios_base::binary);
    bbwt::multi_reader<bbwt::custom_alphabet<uint64_t>> reader(&heads, &runs);

    uint64_t res = 0;
    uint64_t consumed_runs = 0;
    uint64_t symbols_consumed = 0;
    for (auto r : reader) {
        std::cout << r.head << ", " << r.length << std::endl;
        consumed_runs++;
        symbols_consumed += r.length;
        if (r.length > i) {
            res += r.head == c ? i : 0;
            break;
        } else {
            res += r.head == c ? r.length : 0;
            i -= r.length;
        }
    }
    std::cerr << "Consumed a total of " << symbols_consumed << " symbols in " << consumed_runs << " runs.\n"
              << "Rank(" << i << ", " << c << ") = " << res << std::endl;
    return 0;    
}