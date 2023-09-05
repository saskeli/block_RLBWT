
#include <iostream>
#include <cstdint>

#include "include/custom_alphabet.hpp"
#include "include/vbyte_runs.hpp"
#include "include/run_rlbwt.hpp"

#ifndef RUN_COUNT
#define RUN_COUNT 32
#endif

/**
 * Assumes that the following has been done given a bwt in "simple.txt" with
 * (A^{1000}C^{1000}G^{1000}T^{1000})^{2500}
 * 
 *   & make make_alphabet_header
 *   & ./make_alphabet_header -i simple.txt > include/custom_alphabet.hpp
 *   & make make_bwt
 *   & ./make_bwt -i simple.txt -c corpus/simple.runs
 * 
*/


typedef bbwt::run_rlbwt<bbwt::vbyte_runs<RUN_COUNT, bbwt::custom_alphabet<uint64_t>>, 0> bwt_t;

int main() {
    bwt_t bwt("corpus/simple.runs");
    std::string alpha = "ACGT";
    uint64_t runs = 1000;
    uint64_t run_lengths = 1000;
    for (uint64_t run = 0; run < runs; ++run) {
        for (uint64_t run_c = 0; run_c < run_lengths; ++run_c) {
            uint8_t c;
            uint64_t loc = run * run_lengths + run_c;
            auto res = bwt.i_rank(loc, c);
            uint64_t ex = (run / 4) * 1000;
            ex += run_c + 1;
            if (c != alpha[run % 4] || res != ex) {
                std::cout << "expected i_rank(" << loc << ") = (" 
                          << alpha[run % 4] << ", " << ex << "), was (" << c << ", " 
                          << res << ")\n" 
                          << "at(" << loc << ") = " << bwt.at(loc) << std::endl;
                exit(1);             
            }
        }
    }

    for (uint32_t off = 0; off < 4; ++off) {
        for (uint64_t x = 1; x <= runs * run_lengths / 4; ++x) {
            auto res = bwt.select(x, alpha[off]);
            uint64_t ex = (x - 1) / 1000;
            ex *= 4000;
            ex += 1000 * off;
            ex += (x - 1) % 1000;
            ++ex;
            if (res != ex) {
                std::cout << "expected select(" << x << ", " << alpha[off] << ") = "
                          << ex << " but was " << res << std::endl;
                exit(1);
            }
        }
    }

    for (uint64_t end = 0; end < runs * run_lengths; end += 10000) {
        uint64_t start = end / 2;
        std::cout << "interval_symbols(" << start << ", " << end << "):" << std::endl;
        for (auto s : bwt.interval_symbols(start, end)) {
            std::cout << "  " << s.c << ", " << s.start_rank << ", " << s.end_rank << std::endl;
        }
    }
}
