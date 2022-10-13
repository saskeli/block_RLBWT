#include <iostream>
#include <cstring>
#include <fstream>

#include "include/reader.hpp"
#include "include/types.hpp"

static const constexpr uint32_t BLOCK_SIZE = 1 << 12;
static const constexpr uint32_t BUFFER_SIZE = 100;

int main(int argc, char const* argv[]) {
    if (argc < 3) {
        std::cerr << "Path to block_rlwbt and plaintext file are required" << std::endl;
        exit(1);
    }
    
    bbwt::rlbwt<BLOCK_SIZE> bwt(argv[1]);

    std::ifstream in_file;
    in_file.open(argv[2], std::ios::binary | std::ios::in);
    if (in_file.fail()) {
        std::cerr << " -> Failed" << std::endl;
        exit(1);
    }
    uint64_t counts[256];
    std::memset(counts, 0, 256 * sizeof(uint64_t));
    char data_bytes[BUFFER_SIZE];
    uint64_t read = 0;
    while (true) {
        in_file.read(data_bytes, BUFFER_SIZE);
        read += in_file.gcount();
        for (uint32_t i = 0; i < in_file.gcount(); i++) {
            counts[uint8_t(data_bytes[i])]++;
        }
        uint64_t r = bwt.rank('a', read);
        if (counts['a'] != r) {
            std::cerr << "rank('a', " << read << ") = " << r << " != " << counts['a'] << std::endl;
            in_file.close();
            exit(1);
        }
        r = bwt.rank('b', read);
        if (counts['b'] != r) {
            std::cerr << "rank('b', " << read << ") = " << r << " != " << counts['b'] << std::endl;
            in_file.close();
            exit(1);
        }
        r = bwt.rank('c', read);
        if (counts['c'] != r) {
            std::cerr << "rank('c', " << read << ") = " << r << " != " << counts['c'] << std::endl;
            in_file.close();
            exit(1);
        }
        r = bwt.rank('d', read);
        if (counts['d'] != r) {
            std::cerr << "rank('d', " << read << ") = " << r << " != " << counts['d'] << std::endl;
            in_file.close();
            exit(1);
        }
        char a = bwt.at(read - 1);
        if (data_bytes[in_file.gcount() - 1] != a) {
            std::cerr << "at(" << read - 1 << ") = " << a << " != " << data_bytes[in_file.gcount() - 1] << std::endl;
            in_file.close();
            exit(1);
        }
        if (in_file.eof()) {
            break;
        }
        if (read % 1000000 == 0) {
            std::cout << "Ok to " << read << "\r";
        }
    }
    std::cout << "Read " << read << " characters. All OK." << std::endl;
    in_file.close();
}