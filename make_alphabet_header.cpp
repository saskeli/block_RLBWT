#include <iostream>
#include <cstring>
#include <fstream>
#include <unordered_map>
#include <vector>
#include <algorithm>

#include "include/reader.hpp"
#include "include/byte_alphabet.hpp"

static const constexpr uint64_t LIMIT = (uint64_t(1) << 32) - 1;
static const constexpr uint64_t L16 = (uint64_t(1) << 16) - 1;
static const constexpr uint64_t L8 = (uint64_t(1) << 8) - 1;
static uint8_t chars[256];

void help() {
    std::cout << "Create custom alphaber header to be used in block rlbwt structures.\n\n";
    std::cout << "Usage: make_alphabet_header [-i file_name] [-h heads] [-r runs] [-n] > <out_file.hpp>\n";
    std::cout << "   <out_file>     Path to file to write output to.\n";
    std::cout << "   -i file_name   File containing text to encode. (overrides -h and -r.)\n";
    std::cout << "   -h heads       File containing run heads as bytes.\n";
    std::cout << "   -r runs        File containing run lengths as 32-bit integers.\n";
    std::cout << "   -s             Oprimize for space usage rather than lookup speed.\n";
    std::cout << "   -n             Strip new-line characters from input.\n\n";
    std::cout << "If file_name is given, input will be read as a byte-stream from file.\n"
              << "If heads and runs are given, heads are read as byte-stream and runs as stream of 32-bit integers.\n\n";
    std::cout << "Example: make_alphaber_header -i bwt.txt > include/custom_alphabet.hpp" << std::endl;
    exit(0);
}

void output_char_var(uint32_t i, std::vector<std::pair<uint64_t, uint8_t>>& counts) {
    if (counts[i].first > LIMIT) {
        std::cout << "    dtype c" << i << " = 0;\n";
    } else if (counts[i].first > L16) {
        std::cout << "    uint32_t c" << i << " = 0;\n";
    } else if (counts[i].first > L8) {
        std::cout << "    uint16_t c" << i << " = 0;\n";
    } else {
        std::cout << "    uint8_t c" << i << " = 0;\n";
    }
}

void output_space_optimized_code(std::vector<std::pair<uint64_t, uint8_t>>& counts) {
    std::cout << "#include <cstdint>\n\n"
              << "namespace bbwt {\n"
              << "template <class dtype>\n"
              << "class custom_alphabet {\n"
              << "  private:\n"
              << "    inline static const uint8_t c_map[] = {\n";
    uint32_t min_index = 0;
    if (__builtin_popcount(counts.size()) > 1) {
        min_index++;
    }
    for (auto p : counts) {
        chars[p.second] = min_index++;
    }
    for (uint32_t i = 0; i < 16; i++) {
        std::cout << "          ";
        for (uint32_t j = 0; j < 16; j++) {
            std::cout << int(chars[i * 16 + j]);
            if (j < 15) {
                std::cout << ", ";
            }
        }
        if (i < 15) {
            std::cout << ",\n";
        } else {
            std::cout << "};\n";
        }
    }
    std::cout << "    inline static const uint8_t r_map[] = {\n";
    if (__builtin_popcount(counts.size()) > 1) {
        std::cout << "        0, ";
    } else {
        std::cout << "        ";
    }
    for (uint32_t i = 0; i < counts.size(); i++) {
        std::cout << int(counts[i].second);
        if (i < counts.size() - 1) {
            std::cout << ", ";
        } else {
            std::cout << "};\n";
        }
    }
    std::cout << "  public:\n"
              << "    static const constexpr uint8_t width = "
              << 8 * sizeof(unsigned int) - __builtin_clz(counts.size() - 1) << ";\n"
              << "    static constexpr uint8_t convert(uint8_t c) {\n"
              << "        return c_map[c];\n"
              << "    }\n"
              << "    static constexpr uint8_t revert(uint8_t c) {\n"
              << "        return r_map[c];\n"
              << "    }\n"
              << "  private:\n";
    for (uint32_t i = 0; i < counts.size(); i++) {
        output_char_var(i, counts);
    }
    std::cout << "  public:\n"
              << "    custom_alphabet() {}\n"
              << "    custom_alphabet(const custom_alphabet& other) {\n"
              << "        std::memcpy(this, &other, sizeof(custom_alphabet));\n"
              << "    }\n"
              << "    custom_alphabet(custom_alphabet&& other) = delete;\n"
              << "    custom_alphabet& operator=(const custom_alphabet& other) {\n"
              << "        std::memcpy(this, &other, sizeof(custom_alphabet));\n"
              << "    }\n"
              << "    custom_alphabet& operator=(custom_alphabet&& other) = delete;\n"
              << "    void add (uint8_t c, dtype v) {\n"
              << "        if (c == " << int(counts[0].second) << ") {\n"
              << "            c0 += v;\n";
    for (uint32_t i = 1; i < counts.size() - 1; i++) {
        std::cout << "        } else if (c == " << int(counts[i].second) << ") {\n"
                  << "            c" << i << " += v;\n";
    }
    std::cout << "        } else {\n"
              << "            c" << counts.size() - 1 << " += v;\n"
              << "        }\n"
              << "    }\n\n"
              << "    void clear() {\n"
              << "        std::memset(this, 0, sizeof(custom_alphabet));\n"
              << "    }\n\n"
              << "    dtype p_sum(uint8_t c) const {\n"
              << "        if (c == " << int(counts[0].second) << ") {\n"
              << "            return c0;\n";
    for (uint32_t i = 1; i < counts.size() - 1; i++) {
        std::cout << "        } else if (c == " << int(counts[i].second) << ") {\n"
                  << "            return c" << i << ";\n";
    }
    std::cout << "        } else {\n"
              << "            return c" << counts.size() - 1 << ";\n"
              << "        }\n"
              << "    }\n\n"
              << "    void print() {\n";
    for (uint32_t i = 0; i < counts.size(); i++) {
        std::cout << "        std::cerr << " << i << " << c" << i << " << std::endl;\n";
    }
    std::cout << "    }\n"
              << "};\n} // namespace bbwt" << std::endl;
}

void output_code(std::vector<std::pair<uint64_t, uint8_t>>& counts) {
    std::cout << "#include <cstdint>\n\n"
              << "namespace bbwt {\n"
              << "template <class dtype>\n"
              << "class custom_alphabet {\n"
              << "  private:\n"
              << "    inline static const uint8_t c_map[] = {\n";
    uint32_t min_index = 0;
    if (__builtin_popcount(counts.size()) > 1) {
        min_index++;
    }
    for (auto p : counts) {
        chars[p.second] = min_index++;
    }
    for (uint32_t i = 0; i < 16; i++) {
        std::cout << "          ";
        for (uint32_t j = 0; j < 16; j++) {
            std::cout << int(chars[i * 16 + j]);
            if (j < 15) {
                std::cout << ", ";
            }
        }
        if (i < 15) {
            std::cout << ",\n";
        } else {
            std::cout << "};\n";
        }
    }
    std::cout << "    inline static const uint8_t r_map[] = {\n";
    if (__builtin_popcount(counts.size()) > 1) {
        std::cout << "        0, ";
    } else {
        std::cout << "        ";
    }
    for (uint32_t i = 0; i < counts.size(); i++) {
        std::cout << int(counts[i].second);
        if (i < counts.size() - 1) {
            std::cout << ", ";
        } else {
            std::cout << "};\n";
        }
    }
    std::cout << "  public:\n"
              << "    static const constexpr uint8_t width = "
              << 8 * sizeof(unsigned int) - __builtin_clz(counts.size() - 1) << ";\n"
              << "    static constexpr uint8_t convert(uint8_t c) {\n"
              << "        return c_map[c];\n"
              << "    }\n"
              << "    static constexpr uint8_t revert(uint8_t c) {\n"
              << "        return r_map[c];\n"
              << "    }\n"
              << "  private:\n"
              << "    dtype counts[" << min_index << "];\n";
    std::cout << "  public:\n"
              << "    custom_alphabet() : counts() {}\n\n"
              << "    custom_alphabet(const custom_alphabet& other) {\n"
              << "        std::memcpy(this, &other, sizeof(custom_alphabet));\n"
              << "    }\n\n"
              << "    custom_alphabet(custom_alphabet&& other) = delete;\n\n"
              << "    custom_alphabet& operator=(const custom_alphabet& other) {\n"
              << "        std::memcpy(this, &other, sizeof(custom_alphabet));\n"
              << "    }\n\n"
              << "    custom_alphabet& operator=(custom_alphabet&& other) = delete;\n\n"
              << "    void add (uint8_t c, dtype v) {\n"
              << "        counts[c] += v;\n"
              << "    }\n\n"
              << "    void clear() {\n"
              << "        std::memset(this, 0, sizeof(custom_alphabet));\n"
              << "    }\n\n"
              << "    dtype p_sum(uint8_t c) const {\n"
              << "        return counts[c];\n"
              << "    }\n\n"
              << "    void print() {\n"
              << "        for (uint16_t i = 0; i < " << min_index << "; i++) {\n"
              << "            std::cerr << i << \": \" << counts[i] << std::endl;\n"
              << "        }\n"
              << "    }\n"
              << "};\n} // namespace bbwt" << std::endl;
}

int main(int argc, char const* argv[]) {
    std::memset(chars, 0, 256);
    size_t in_file_loc = 0;
    size_t heads_loc = 0;
    size_t runs_loc = 0;
    bool strip_new_line = false;
    bool space_optimize = false;
    if (argc < 2) {
        help();
    }

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-i") == 0) {
            in_file_loc = ++i;
        } else if (strcmp(argv[i], "-h") == 0) {
            heads_loc = ++i;
        } else if (strcmp(argv[i], "-r") == 0) {
            runs_loc = ++i;
        } else if (strcmp(argv[i], "-n") == 0) {
            strip_new_line = true;
        } else if (strcmp(argv[i], "-s") == 0) {
            space_optimize = true;
        }
    }
    std::unordered_map<uint8_t, uint64_t> map;
    if (in_file_loc) {
        std::ifstream in(argv[in_file_loc]);
        bbwt::file_reader<bbwt::byte_alphabet<uint32_t>> reader(&in);
        for (auto it : reader) {
            if (strip_new_line && (it.head == '\n')) {
                continue;
            }
            if (map.contains(it.head)) {
                map[it.head] += it.length;
            } else {
                map[it.head] = it.length;
            }
        }
    } else if (runs_loc || heads_loc) {
        if (runs_loc == 0 || heads_loc == 0) {
            std::cerr << "Both heads and run lengths are required" << std::endl;
            help();
        }
        std::ifstream heads;
        std::ifstream runs;
        heads.open(argv[heads_loc], std::ios_base::in | std::ios_base::binary);
        runs.open(argv[runs_loc], std::ios_base::in | std::ios_base::binary);
        bbwt::multi_reader<bbwt::byte_alphabet<uint32_t>> reader(&heads, &runs);
        for (auto it : reader) {
            if (strip_new_line && (it.head == '\n')) {
                continue;
            }
            if (map.contains(it.head)) {
                map[it.head] += it.length;
            } else {
                map[it.head] = it.length;
            }
        }
    } else {
        help();
    }
    std::vector<std::pair<uint64_t, uint8_t>> counts;
    for (auto it : map) {
        counts.push_back({it.second, it.first});
    }
    std::sort(counts.rbegin(), counts.rend());
    std::cerr << "Character counts:\n";
    for (auto it: counts) {
        if (it.second >= 32 && it.second < 127) {
            std::cerr << "'" << char(it.second) << "' ";
        }
        std::cerr << "(" << int(it.second) << "): " << it.first << std::endl;
    }
    if (space_optimize) {
        output_space_optimized_code(counts);
    } else {
        output_code(counts);
    }
    
    return 0;
}
