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

    uint64_t loc = (uint64_t(1) << 32) + 2;
    std::cout << "char at " << loc << " = " << char(bwt.at(loc)) << std::endl;
    std::cout << "rank(a, " << loc << ") = " << bwt.rank('a', loc) << std::endl;
    std::cout << "rank(b, " << loc << ") = " << bwt.rank('b', loc) << std::endl;
    std::cout << "rank(c, " << loc << ") = " << bwt.rank('c', loc) << std::endl;
    std::cout << "rank(d, " << loc << ") = " << bwt.rank('d', loc) << std::endl;
}