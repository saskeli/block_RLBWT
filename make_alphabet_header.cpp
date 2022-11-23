#include <iostream>
#include <cstring>
#include <fstream>
#include <queue>
#include <vector>
#include <algorithm>

#include "include/reader.hpp"
#include "include/byte_alphabet.hpp"

static const constexpr uint64_t LIMIT = (uint64_t(1) << 32) - 1;
static const constexpr uint64_t L16 = (uint64_t(1) << 16) - 1;
static const constexpr uint64_t L8 = (uint64_t(1) << 8) - 1;
static uint64_t cur_chars = 0;
static uint8_t chars[256];
static uint64_t totals[256];
static uint32_t sub_totals[256];
static uint32_t max_sub[256];

void help() {
    std::cout << "Create custom alphaber header to be used in block rlbwt structures.\n\n";
    std::cout << "Usage: make_alphabet_header [-i file_name] [-h heads] [-r runs] [-n] > <out_file.hpp>\n";
    std::cout << "   <out_file>     Path to file to write output to.\n";
    std::cout << "   -i file_name   File containing text to encode. (overrides -h and -r.)\n";
    std::cout << "   -h heads       File containing run heads as bytes.\n";
    std::cout << "   -r runs        File containing run lengths as 32-bit integers.\n";
    std::cout << "   -n             Strip new-line characters from input.\n\n";
    std::cout << "If file_name is given, input will be read as a byte-stream from file.\n"
              << "If heads and runs are given, heads are read as byte-stream and runs as stream of 32-bit integers.\n\n";
    std::cout << "Example: make_alphaber_header -i bwt.txt > include/custom_alphabet.hpp" << std::endl;
    exit(0);
}

typedef std::pair<uint64_t, std::pair<uint32_t, uint32_t>> L_data;
typedef std::pair<uint32_t, std::pair<uint16_t, uint16_t>> S_data;

void output_exact(std::vector<std::pair<uint64_t, uint8_t>>& counts) {
    std::vector<L_data> tot_data;
    std::vector<S_data> stot_data;
    uint64_t used_bits = 0;
    for (auto e : counts) {
        uint16_t bits = 64 - __builtin_clzll(totals[e.second]);
        int32_t start = used_bits / 8;
        uint16_t start_offset = used_bits % 8;
        int32_t end = (used_bits + bits) / 8;
        uint16_t end_offset = (used_bits + bits) % 8;
        if (start + 7 < end) {
            start++;
            used_bits += 8 - start_offset;
            end_offset += 8 - start_offset;
            start_offset = 0;
        }
        uint16_t shift = 8 - end_offset;
        start = end - 7;
        if (start < 0) {
            shift += -start * 8;
            start = 0;
        }
        tot_data.push_back({(uint64_t(1) << bits) - 1, {start, shift}});
        used_bits += bits;
    }
    uint64_t l_bytes = used_bits / 8 + (used_bits % 8 ? 1 : 0);
    used_bits = 0;
    for (auto e : counts) {
        uint16_t bits = 64 - __builtin_clzll(max_sub[e.second]);
        int32_t start = used_bits / 8;
        uint16_t start_offset = used_bits % 8;
        int32_t end = (used_bits + bits) / 8;
        uint16_t end_offset = (used_bits + bits) % 8;
        if (start + 3 < end) {
            start++;
            used_bits += 8 - start_offset;
            end_offset += 8 - start_offset;
            start_offset = 0;
        }
        uint16_t shift = 8 - end_offset;
        start = end - 3;
        if (start < 0) {
            shift += -start * 8;
            start = 0;
        }
        stot_data.push_back({(uint32_t(1) << bits) - 1, {start, shift}});
        used_bits += bits;
    }
    uint64_t s_bytes = used_bits / 8 + (used_bits % 8 ? 1 : 0);

    std::cout << "#include <cstdint>\n"
              << "#include <utility>\n"
              << "#include <endian.h>\n\n"
              << "namespace bbwt {\n"
              << "template <class dtype>\n"
              << "class custom_alphabet {\n"
              << "   private:\n"
              << "    inline static const uint8_t c_map[] = {\n";
    uint32_t min_index = 0;
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
    for (uint32_t i = 0; i < 16; i++) {
        std::cout << "        ";
        for (uint32_t j = 0; j < 16; j++) {
            if (i * 16 + j >= counts.size()) {
                std::cout << "0";
            } else {
                std::cout << int(counts[i * 16 + j].second);
            }
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
    std::cout << "    typedef std::pair<uint64_t, std::pair<uint32_t, uint32_t>> L;\n"
              << "    typedef std::pair<uint32_t, std::pair<uint16_t, uint16_t>> S;\n\n"
              << "    inline static const L L_map[] = {\n";
    uint16_t c = 0;
    for (auto d : tot_data) {
        std::cout << "        {" << d.first << ", {" << d.second.first << ", " << d.second.second << "}}";
        c++;
        if (c < tot_data.size()) {
            std::cout << ",\n";
        } else {
            std::cout << "};\n\n";
        }
    }
    std::cout << "    inline static const S S_map[] = {\n";
    c = 0;
    for (auto d : stot_data) {
        std::cout << "        {" << d.first << ", {" << d.second.first << ", " << d.second.second << "}}";
        c++;
        if (c < stot_data.size()) {
            std::cout << ",\n";
        } else {
            std::cout << "};\n\n";
        }
    }
    std::cout << "   public:\n"
              << "    static const constexpr uint8_t width = "
              << 8 * sizeof(unsigned int) - __builtin_clz(counts.size() - 1) << ";\n"
              << "    static constexpr uint8_t convert(uint8_t c) {\n"
              << "        return c_map[c];\n"
              << "    }\n"
              << "    static constexpr uint8_t revert(uint8_t c) {\n"
              << "        return r_map[c];\n"
              << "    }\n"
              << "    static constexpr uint16_t size() {\n"
              << "        return sizeof(custom_alphabet);\n"
              << "    }\n"
              << "    template <class o_t>\n"
              << "    static void write_statics(o_t& out) {\n"
              << "        out.write(reinterpret_cast<const char*>(&width), 1);\n"
              << "        uint32_t size = sizeof(custom_alphabet);\n"
              << "        out.write(reinterpret_cast<char*>(&size), 4);\n"
              << "        out.write(reinterpret_cast<const char*>(c_map), 256);\n"
              << "        out.write(reinterpret_cast<const char*>(r_map), 256);\n"
              << "        if constexpr (sizeof(dtype) == 4) {\n"
              << "            uint32_t s = " << stot_data.size() * sizeof(S_data) << ";\n"
              << "            out.write(reinterpret_cast<char*>(&s), 4);\n"
              << "            out.write(reinterpret_cast<const char*>(S_map), " << stot_data.size() * sizeof(S_data) << ");\n"
              << "        } else {\n"
              << "            uint32_t s = " << tot_data.size() * sizeof(L_data) << ";\n"
              << "            out.write(reinterpret_cast<char*>(&s), 4);\n"
              << "            out.write(reinterpret_cast<const char*>(L_map), " << tot_data.size() * sizeof(L_data) << ");\n"
              << "        }\n"
              << "    }\n"
              << "    template <class i_t>\n"
              << "    static uint32_t load_statics(i_t& in_file) {\n"
              << "        uint8_t w;\n"
              << "        in_file.read(reinterpret_cast<char*>(&w), 1);\n"
              << "        uint32_t size;\n"
              << "        in_file.read(reinterpret_cast<char*>(&size), 4);\n"
              << "        uint8_t* buf = (uint8_t*)std::malloc(256);\n"
              << "        in_file.read(reinterpret_cast<char*>(buf), 256);\n"
              << "        in_file.read(reinterpret_cast<char*>(buf), 256);\n"
              << "        uint32_t s;\n"
              << "        in_file.read(reinterpret_cast<char*>(&s), 4);\n"
              << "        if constexpr (sizeof(dtype) == 4) {\n"
              << "            uint8_t* s_buf = (uint8_t*)std::malloc(s);\n"
              << "            in_file.read(reinterpret_cast<char*>(s_buf), s);\n"
              << "            std::free(s_buf);\n"
              << "        } else {\n"
              << "            uint8_t* l_buf = (uint8_t*)std::malloc(s);\n"
              << "            in_file.read(reinterpret_cast<char*>(l_buf), s);\n"
              << "            std::free(l_buf);\n"
              << "        }\n"
              << "        return s + 2 * 256 + sizeof(L*) + sizeof(S*) + 1;\n"
              << "    }\n\n"
              << "   private:\n"
              << "    uint8_t counts[" 
              << "(sizeof(dtype) == 8 ? " << (l_bytes < 8 ? 8 : l_bytes) << " : 0) + "
              << "(sizeof(dtype) == 4 ? " << (s_bytes < 4 ? 4 : s_bytes) << " : 0)];\n";
    std::cout << "   public:\n"
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
              << "        dtype ov = p_sum(c);\n"
              << "        if constexpr (sizeof(dtype) == 8) {\n"
              << "            dtype d = be64toh(reinterpret_cast<dtype*>(counts + L_map[c].second.first)[0]);\n"
              << "            d &= ~(L_map[c].first << L_map[c].second.second);\n"
              << "            d = htobe64(d | ((v + ov) << L_map[c].second.second));\n"
              << "            reinterpret_cast<dtype*>(counts + L_map[c].second.first)[0] = d;\n"
              << "        } else {\n"
              << "            dtype d = be32toh(reinterpret_cast<dtype*>(counts + S_map[c].second.first)[0]);\n"
              << "            d &= ~(S_map[c].first << S_map[c].second.second);\n"
              << "            d = htobe32(d | ((v + ov) << S_map[c].second.second));\n"
              << "            reinterpret_cast<dtype*>(counts + S_map[c].second.first)[0] = d;\n"
              << "        }\n"
              << "    }\n\n"
              << "    void clear() {\n"
              << "        std::memset(this, 0, sizeof(custom_alphabet));\n"
              << "    }\n\n"
              << "    dtype p_sum(uint8_t c) const {\n"
              << "        if constexpr (sizeof(dtype) == 8) {\n"
              << "            dtype d = be64toh(reinterpret_cast<const dtype*>(counts + L_map[c].second.first)[0]);\n"
              << "            return (d >> L_map[c].second.second) & L_map[c].first;\n"
              << "        } else {\n"
              << "            dtype d = be32toh(reinterpret_cast<const dtype*>(counts + S_map[c].second.first)[0]);\n"
              << "            return (d >> S_map[c].second.second) & S_map[c].first;\n"
              << "        }\n"
              << "    }\n\n"
              << "    void print() const {\n"
              << "        for (uint16_t i = 0; i < " << min_index << "; i++) {\n"
              << "            std::cerr << int(revert(i)) << \": \" << p_sum(i) << std::endl;\n"
              << "        }\n"
              << "    }\n"
              << "};\n} // namespace bbwt" << std::endl;
}   

template<class T>
void add(T& it) {
    uint32_t l = it.length;
    totals[it.head] += l;
    if (cur_chars + l > LIMIT) {
        uint64_t delta = cur_chars + l - LIMIT;
        sub_totals[it.head] += delta;
        if (max_sub[it.head] < sub_totals[it.head]) {
            max_sub[it.head] = sub_totals[it.head];
        }
        std::memset(sub_totals, 0, sizeof(uint32_t) * 256);
        cur_chars = 0;
        l -= delta;
    }
    sub_totals[it.head] += l;
    cur_chars += l;
    if (max_sub[it.head] < sub_totals[it.head]) {
        max_sub[it.head] = sub_totals[it.head];
    }
}

int main(int argc, char const* argv[]) {
    std::memset(chars, 0, 256);
    size_t in_file_loc = 0;
    size_t heads_loc = 0;
    size_t runs_loc = 0;
    bool strip_new_line = false;
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
        }
    }
    std::memset(totals, 0, 256 * sizeof(uint64_t));
    std::memset(sub_totals, 0, 256 * sizeof(uint32_t));
    std::memset(max_sub, 0, 256 * sizeof(uint32_t));
    if (in_file_loc) {
        std::ifstream in(argv[in_file_loc]);
        bbwt::file_reader<bbwt::byte_alphabet<uint32_t>> reader(&in);
        for (auto it : reader) {
            if (strip_new_line && (it.head == '\n')) {
                continue;
            }
            add(it);
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
            add(it);
        }
    } else {
        help();
    }
    std::vector<std::pair<uint64_t, uint8_t>> counts;
    std::cerr << "Character counts:\n";
    for (uint16_t i = 0; i < 256; i++) {
        if (totals[i]) {
            counts.push_back({totals[i], i});
            if (i >= 32 && i < 127) {
                std::cerr << "'" << char(i) << "' ";
            }
            std::cerr << "(" << i << "): " << totals[i] << std::endl;
        }
    }
    std::cerr << "Character subs:\n";
    for (uint16_t i = 0; i < 256; i++) {
        if (max_sub[i]) {
            if (i >= 32 && i < 127) {
                std::cerr << "'" << char(i) << "' ";
            }
            std::cerr << "(" << i << "): " << max_sub[i] << std::endl;
        }
    }
    std::sort(counts.begin(), counts.end());
    output_exact(counts);
    
    return 0;
}
