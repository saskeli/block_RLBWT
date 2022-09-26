#include <iostream>
#include <cstring>
#include <fstream>

#include "include/reader.hpp"
#include "include/block_rlbwt_builder.hpp"
#include "include/byte_block.hpp"
#include "include/block_rlbwt.hpp"
#include "include/byte_alphabet.hpp"
#include "include/super_block.hpp"

static const constexpr uint32_t BLOCK_SIZE = 1 << 12;

typedef bbwt::byte_alphabet<uint32_t> block_alphabet;
typedef bbwt::byte_alphabet<uint64_t> super_block_alphabet;
typedef bbwt::byte_block<BLOCK_SIZE, block_alphabet> block;
typedef bbwt::super_block<block> s_block;
typedef bbwt::block_rlbwt<s_block, super_block_alphabet> rlbwt;

int main(int argc, char const* argv[]) {
    if (argc < 2) {
        std::cerr << "Path to block_rlwbt is required" << std::endl;
        exit(1);
    }
    
    rlbwt bwt(argv[1]);

    uint64_t s = std::min(uint64_t(100), bwt.size());
    for (uint64_t i = 0; i < s; i++) {
        std::cout << bwt.at(i);
    }
    std::cout << std::endl;
}