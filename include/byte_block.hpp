#pragma once

#include <cstring>
#include <iostream>
#include <utility>
#include <vector>

namespace bbwt {
template <uint32_t block_size, class alphabet_type_>
class byte_block {
   public:
    typedef alphabet_type_ alphabet_type;
    static const constexpr uint32_t cap = block_size;
    static const constexpr uint32_t scratch_blocks = 2;
    static const constexpr uint32_t min_size =
        2 + (block_size >= (uint32_t(1) << 7) ? 1 : 0) +
        (block_size >= (uint32_t(1) << 14) ? 1 : 0) +
        (block_size >= (uint32_t(1) << 21) ? 1 : 0) +
        (block_size >= (uint32_t(1) << 28) ? 1 : 0);

    static const constexpr uint32_t max_size = 2 * block_size;

    static constexpr uint64_t scratch_size(uint32_t i) {
        if (i == 0) {
            return 8;
        } else {
            return max_size;
        }
    }

    byte_block() {}

    byte_block(const byte_block& other) = delete;
    byte_block(byte_block&& other) = delete;
    byte_block& operator=(byte_block&& other) = delete;
    byte_block& operator=(const byte_block&) = delete;

    uint32_t append(uint8_t head, uint32_t length, uint8_t** scratch) {
        uint32_t ret = 2;
        uint64_t* offset = reinterpret_cast<uint64_t*>(scratch[0]);
        scratch[1][offset[0]++] = head;
        while (length >= 0b10000000) {
            scratch[1][offset[0]++] = length & 0b01111111;
            length >>= 7;
            ret++;
        }
        scratch[1][offset[0]++] = length | 0b10000000;
        return ret;
    }

    uint8_t at(uint32_t location) {
        uint8_t* data = reinterpret_cast<uint8_t*>(this);
        uint32_t i = 0;
        while (true) {
            uint8_t c = data[i++];
            uint32_t rl = read(i, data);
            if (location >= rl) {
                location -= rl;
            } else {
                return c;
            }
        }
    }

    uint32_t rank(uint8_t c, uint32_t location) {
        uint8_t* data = reinterpret_cast<uint8_t*>(this);
        uint32_t res = 0;
        uint32_t i = 0;
        while (true) {
            uint8_t current = data[i++];
            uint32_t rl = read(i, data);
            if (location >= rl) {
                location -= rl;
                res += current == c ? rl : 0;
            } else {
                res += current == c ? location : 0;
                return res;
            }
        }
    }

    void commit(uint8_t** scratch) {
        uint64_t* bytes = reinterpret_cast<uint64_t*>(scratch[0]);
        uint8_t* data = reinterpret_cast<uint8_t*>(this);
        std::memcpy(data, scratch[1], bytes[0]);
    }

   private:
    uint32_t read(uint32_t& i, uint8_t* data) {
        uint32_t res = 0;
        uint32_t offset = 0;
        while ((data[i] & 0b10000000) == 0) [[unlikely]] {
            res |= data[i++] << offset;
            offset += 7;
        }
        return res | ((data[i++] & 0b01111111) << offset);
    }
};
}  // namespace bbwt
