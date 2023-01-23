#pragma once

#include <cstdint>
#include <utility>
#include <endian.h>

namespace bbwt {
template <class dtype>
class custom_alphabet {
   private:
    inline static const uint8_t c_map[] = {
          0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
          0, 4, 0, 2, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 1, 0,
          0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
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
    inline static const uint8_t r_map[] = {
        10, 78, 67, 71, 65, 84, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
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
        {524287, {0, 45}},
        {268435455, {0, 17}},
        {4294967295, {2, 1}},
        {4294967295, {6, 1}},
        {4294967295, {10, 1}},
        {4294967295, {14, 1}}};

    inline static const S S_map[] = {
        {262143, {0, 14}},
        {268435455, {2, 2}},
        {1073741823, {6, 2}},
        {1073741823, {10, 2}},
        {2147483647, {14, 1}},
        {2147483647, {18, 1}}};

   public:
    static const constexpr uint8_t width = 3;
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
            uint32_t s = 48;
            out.write(reinterpret_cast<char*>(&s), 4);
            out.write(reinterpret_cast<const char*>(S_map), 48);
        } else {
            uint32_t s = 96;
            out.write(reinterpret_cast<char*>(&s), 4);
            out.write(reinterpret_cast<const char*>(L_map), 96);
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
    uint8_t counts[(sizeof(dtype) == 8 ? 22 : 0) + (sizeof(dtype) == 4 ? 22 : 0)];
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
        for (uint16_t i = 0; i < 6; i++) {
            std::cerr << int(revert(i)) << ": " << p_sum(i) << std::endl;
        }
    }
};
} // namespace bbwt
