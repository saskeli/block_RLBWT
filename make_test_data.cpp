//#define VERB

#include <iostream>
#include <vector>
#include <random>
#include <chrono>

#include "include/reader.hpp"
#include "include/types.hpp"


int main(int argc, char const* argv[]) {
    const size_t p_len = 10;
    const size_t n = 10000;
    std::string in_file_path(argv[1]);
    bbwt::two_byte<> bwt(in_file_path);

    std::mt19937 mt(1337);
    std::uniform_int_distribution<unsigned long long> gen(0, bwt.size());
    char chars[p_len + 1];
    chars[p_len] = '\0';
    for (size_t i = 0; i < n; i++) {
        uint64_t idx = gen(mt);
        bool ok = true;
        for (size_t p_idx = p_len - 1; p_idx < p_len; p_idx--) {
            char c = bwt[idx];
            if (c == 'A' || c == 'C' || c == 'G' || c == 'T') {
                chars[p_idx] = bwt[idx];
                idx = bwt.LF(idx);
            } else {
                ok = false;
                break;
            }
        }
        if (ok) {
            std::cout << chars << std::endl;
        } else {
            i--;
        }
    }
    return 0;    
}