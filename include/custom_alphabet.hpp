#pragma once

#include <cstdint>
#include <utility>
#include <endian.h>

namespace bbwt {
template <class dtype>
class custom_alphabet {
   private:
    inline static const uint8_t c_map[] = {
          0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 45, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
          88, 2, 14, 0, 0, 0, 57, 24, 35, 34, 0, 0, 69, 40, 89, 10,
          27, 26, 25, 19, 20, 22, 17, 23, 18, 21, 13, 6, 7, 0, 5, 0,
          0, 76, 49, 62, 61, 68, 48, 50, 59, 67, 44, 46, 55, 78, 66, 60,
          64, 30, 65, 72, 56, 51, 42, 41, 16, 43, 37, 8, 0, 9, 0, 0,
          4, 83, 54, 73, 71, 86, 79, 63, 70, 87, 36, 47, 77, 74, 85, 84,
          53, 33, 82, 80, 81, 75, 52, 39, 29, 58, 38, 0, 0, 0, 0, 0,
          31, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 15, 0, 0, 0, 3, 28, 0, 0, 11, 12, 0, 0,
          0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
          0, 0, 32, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    inline static const uint8_t r_map[] = {
        0, 203, 33, 152, 96, 62, 59, 60, 91, 93, 47, 156, 157, 58, 34, 148,
        88, 54, 56, 51, 52, 57, 53, 55, 39, 50, 49, 48, 153, 120, 81, 128,
        226, 113, 41, 40, 106, 90, 122, 119, 45, 87, 86, 89, 74, 10, 75, 107,
        70, 66, 71, 85, 118, 112, 98, 76, 84, 38, 121, 72, 79, 68, 67, 103,
        80, 82, 78, 73, 69, 44, 104, 100, 83, 99, 109, 117, 65, 108, 77, 102,
        115, 116, 114, 97, 111, 110, 101, 105, 32, 46, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    typedef std::pair<uint64_t, std::pair<uint32_t, uint32_t>> L;
    typedef std::pair<uint32_t, std::pair<uint16_t, uint16_t>> S;

    inline static const L L_map[] = {
        {1, {0, 63}},
        {1, {0, 62}},
        {3, {0, 60}},
        {3, {0, 58}},
        {7, {0, 55}},
        {31, {0, 50}},
        {63, {0, 44}},
        {63, {0, 38}},
        {127, {0, 31}},
        {127, {0, 24}},
        {511, {0, 15}},
        {1023, {0, 5}},
        {1023, {1, 3}},
        {1023, {2, 1}},
        {1023, {4, 7}},
        {1023, {5, 5}},
        {4095, {6, 1}},
        {4095, {8, 5}},
        {4095, {9, 1}},
        {4095, {11, 5}},
        {4095, {12, 1}},
        {4095, {14, 5}},
        {4095, {15, 1}},
        {4095, {17, 5}},
        {4095, {18, 1}},
        {8191, {20, 4}},
        {8191, {22, 7}},
        {8191, {23, 2}},
        {16383, {25, 4}},
        {16383, {27, 6}},
        {16383, {29, 8}},
        {16383, {30, 2}},
        {16383, {32, 4}},
        {16383, {34, 6}},
        {16383, {36, 8}},
        {16383, {37, 2}},
        {32767, {39, 3}},
        {65535, {41, 3}},
        {65535, {43, 3}},
        {65535, {45, 3}},
        {65535, {47, 3}},
        {65535, {49, 3}},
        {131071, {51, 2}},
        {131071, {53, 1}},
        {131071, {56, 8}},
        {131071, {58, 7}},
        {131071, {60, 6}},
        {131071, {62, 5}},
        {131071, {64, 4}},
        {131071, {66, 3}},
        {131071, {68, 2}},
        {262143, {71, 8}},
        {262143, {73, 6}},
        {262143, {75, 4}},
        {262143, {77, 2}},
        {262143, {80, 8}},
        {262143, {82, 6}},
        {262143, {84, 4}},
        {262143, {86, 2}},
        {262143, {89, 8}},
        {262143, {91, 6}},
        {262143, {93, 4}},
        {262143, {95, 2}},
        {262143, {98, 8}},
        {262143, {100, 6}},
        {262143, {102, 4}},
        {262143, {104, 2}},
        {262143, {107, 8}},
        {524287, {109, 5}},
        {524287, {111, 2}},
        {524287, {114, 7}},
        {524287, {116, 4}},
        {524287, {118, 1}},
        {524287, {121, 6}},
        {524287, {123, 3}},
        {524287, {126, 8}},
        {1048575, {128, 4}},
        {1048575, {131, 8}},
        {1048575, {133, 4}},
        {1048575, {136, 8}},
        {1048575, {138, 4}},
        {1048575, {141, 8}},
        {2097151, {143, 3}},
        {2097151, {146, 6}},
        {2097151, {148, 1}},
        {2097151, {151, 4}},
        {2097151, {154, 7}},
        {2097151, {156, 2}},
        {4194303, {159, 4}},
        {33554431, {162, 3}}};

    inline static const S S_map[] = {
        {1, {0, 31}},
        {1, {0, 30}},
        {3, {0, 28}},
        {3, {0, 26}},
        {7, {0, 23}},
        {31, {0, 18}},
        {63, {0, 12}},
        {63, {0, 6}},
        {127, {1, 7}},
        {127, {2, 8}},
        {511, {3, 7}},
        {1023, {4, 5}},
        {1023, {5, 3}},
        {1023, {6, 1}},
        {1023, {8, 7}},
        {1023, {9, 5}},
        {4095, {10, 1}},
        {4095, {12, 5}},
        {4095, {13, 1}},
        {4095, {15, 5}},
        {4095, {16, 1}},
        {4095, {18, 5}},
        {4095, {19, 1}},
        {4095, {21, 5}},
        {4095, {22, 1}},
        {8191, {24, 4}},
        {8191, {26, 7}},
        {8191, {27, 2}},
        {16383, {29, 4}},
        {16383, {31, 6}},
        {16383, {33, 8}},
        {16383, {34, 2}},
        {16383, {36, 4}},
        {16383, {38, 6}},
        {16383, {40, 8}},
        {16383, {41, 2}},
        {32767, {43, 3}},
        {65535, {45, 3}},
        {65535, {47, 3}},
        {65535, {49, 3}},
        {65535, {51, 3}},
        {65535, {53, 3}},
        {131071, {55, 2}},
        {131071, {57, 1}},
        {131071, {60, 8}},
        {131071, {62, 7}},
        {131071, {64, 6}},
        {131071, {66, 5}},
        {131071, {68, 4}},
        {131071, {70, 3}},
        {131071, {72, 2}},
        {262143, {75, 8}},
        {262143, {77, 6}},
        {262143, {79, 4}},
        {262143, {81, 2}},
        {262143, {84, 8}},
        {262143, {86, 6}},
        {262143, {88, 4}},
        {262143, {90, 2}},
        {262143, {93, 8}},
        {262143, {95, 6}},
        {262143, {97, 4}},
        {262143, {99, 2}},
        {262143, {102, 8}},
        {262143, {104, 6}},
        {262143, {106, 4}},
        {262143, {108, 2}},
        {262143, {111, 8}},
        {524287, {113, 5}},
        {524287, {115, 2}},
        {524287, {118, 7}},
        {524287, {120, 4}},
        {524287, {122, 1}},
        {524287, {125, 6}},
        {524287, {127, 3}},
        {524287, {130, 8}},
        {1048575, {132, 4}},
        {1048575, {135, 8}},
        {1048575, {137, 4}},
        {1048575, {140, 8}},
        {1048575, {142, 4}},
        {1048575, {145, 8}},
        {2097151, {147, 3}},
        {2097151, {150, 6}},
        {2097151, {152, 1}},
        {2097151, {155, 4}},
        {2097151, {158, 7}},
        {2097151, {160, 2}},
        {4194303, {163, 4}},
        {33554431, {166, 3}}};

   public:
    static const constexpr uint8_t width = 7;
    static constexpr uint8_t convert(uint8_t c) {
        return c_map[c];
    }
    static constexpr uint8_t revert(uint8_t c) {
        return r_map[c];
    }
    static constexpr uint16_t size() {
        return sizeof(custom_alphabet);
    }
    template <class o_t>
    static void write_statics(o_t& out) {
        out.write(reinterpret_cast<const char*>(&width), 1);
        uint32_t size = sizeof(custom_alphabet);
        out.write(reinterpret_cast<char*>(&size), 4);
        out.write(reinterpret_cast<const char*>(c_map), 256);
        out.write(reinterpret_cast<const char*>(r_map), 256);
        if constexpr (sizeof(dtype) == 4) {
            uint32_t s = 720;
            out.write(reinterpret_cast<char*>(&s), 4);
            out.write(reinterpret_cast<const char*>(S_map), 720);
        } else {
            uint32_t s = 1440;
            out.write(reinterpret_cast<char*>(&s), 4);
            out.write(reinterpret_cast<const char*>(L_map), 1440);
        }
    }
    template <class i_t>
    static uint32_t load_statics(i_t& in_file) {
        uint8_t w;
        in_file.read(reinterpret_cast<char*>(&w), 1);
        uint32_t size;
        in_file.read(reinterpret_cast<char*>(&size), 4);
        uint8_t* buf = (uint8_t*)std::malloc(256);
        in_file.read(reinterpret_cast<char*>(buf), 256);
        in_file.read(reinterpret_cast<char*>(buf), 256);
        uint32_t s;
        in_file.read(reinterpret_cast<char*>(&s), 4);
        if constexpr (sizeof(dtype) == 4) {
            uint8_t* s_buf = (uint8_t*)std::malloc(s);
            in_file.read(reinterpret_cast<char*>(s_buf), s);
            std::free(s_buf);
        } else {
            uint8_t* l_buf = (uint8_t*)std::malloc(s);
            in_file.read(reinterpret_cast<char*>(l_buf), s);
            std::free(l_buf);
        }
        return s + 2 * 256 + sizeof(L*) + sizeof(S*) + 1;
    }

   private:
    uint8_t counts[(sizeof(dtype) == 8 ? 170 : 0) + (sizeof(dtype) == 4 ? 170 : 0)];
   public:
    custom_alphabet() : counts() {}

    custom_alphabet(const custom_alphabet& other) {
        std::memcpy(this, &other, sizeof(custom_alphabet));
    }

    custom_alphabet(custom_alphabet&& other) = delete;

    custom_alphabet& operator=(const custom_alphabet& other) {
        std::memcpy(this, &other, sizeof(custom_alphabet));
    }

    custom_alphabet& operator=(custom_alphabet&& other) = delete;

    void add (uint8_t c, dtype v) {
        dtype ov = p_sum(c);
        if constexpr (sizeof(dtype) == 8) {
            dtype d = be64toh(reinterpret_cast<dtype*>(counts + L_map[c].second.first)[0]);
            d &= ~(L_map[c].first << L_map[c].second.second);
            d = htobe64(d | ((v + ov) << L_map[c].second.second));
            reinterpret_cast<dtype*>(counts + L_map[c].second.first)[0] = d;
        } else {
            dtype d = be32toh(reinterpret_cast<dtype*>(counts + S_map[c].second.first)[0]);
            d &= ~(S_map[c].first << S_map[c].second.second);
            d = htobe32(d | ((v + ov) << S_map[c].second.second));
            reinterpret_cast<dtype*>(counts + S_map[c].second.first)[0] = d;
        }
    }

    void clear() {
        std::memset(this, 0, sizeof(custom_alphabet));
    }

    dtype p_sum(uint8_t c) const {
        if constexpr (sizeof(dtype) == 8) {
            dtype d = be64toh(reinterpret_cast<const dtype*>(counts + L_map[c].second.first)[0]);
            return (d >> L_map[c].second.second) & L_map[c].first;
        } else {
            dtype d = be32toh(reinterpret_cast<const dtype*>(counts + S_map[c].second.first)[0]);
            return (d >> S_map[c].second.second) & S_map[c].first;
        }
    }

    void print() const {
        for (uint16_t i = 0; i < 90; i++) {
            std::cerr << int(revert(i)) << ": " << p_sum(i) << std::endl;
        }
    }
};
} // namespace bbwt
