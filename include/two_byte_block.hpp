#pragma once

#include <iostream>
#include <bitset>

namespace bbwt {
template <uint32_t block_size, class alphabet_type_>
class two_byte_block {
  public:
    typedef alphabet_type_ alphabet_type;
  private:
    static_assert(alphabet_type::width < 8);
    static_assert(block_size <= ~uint32_t(0) >> 1);
    static const constexpr uint16_t SHIFT = 16 - alphabet_type::width;
    static const constexpr uint16_t LIMIT = uint16_t(1) << SHIFT;
    static const constexpr uint16_t MASK = LIMIT - 1;
   public:
    static const constexpr uint32_t cap = block_size;
    static const constexpr uint32_t scratch_blocks = 2;
    static const constexpr uint32_t min_size =
        block_size / LIMIT + (block_size % LIMIT ? 2 : 0);

    static const constexpr uint32_t max_size = 2 * block_size;
    static constexpr uint64_t scratch_size(uint32_t i) {
        if (i == 0) {
            return 8;
        } else {
            return max_size;
        }
    }

    two_byte_block() {}

    two_byte_block(const two_byte_block& other) = delete;
    two_byte_block(two_byte_block&& other) = delete;
    two_byte_block& operator=(two_byte_block&& other) = delete;
    two_byte_block& operator=(const two_byte_block&) = delete;

    uint32_t append(uint8_t head, uint32_t length, uint8_t** scratch) {
        uint64_t* offset = reinterpret_cast<uint64_t*>(scratch[0]);
        uint16_t* data = reinterpret_cast<uint16_t*>(scratch[1]);
        head = alphabet_type::convert(head);
        uint32_t ret = 2;
        if constexpr (LIMIT < cap) {
            while (length > LIMIT) {
                data[offset[0]++] = (head << SHIFT) | MASK;
                length -= LIMIT;
                ret += 2;
            }
        }
        data[offset[0]++] = (head << SHIFT) | (length - 1);
        return ret;
    }

    uint8_t at(uint32_t location) {
        uint16_t* data = reinterpret_cast<uint16_t*>(this);
        uint32_t i = 0;
        while (true) {
            uint8_t current = data[i] >> SHIFT;
            uint16_t length = 1 + (data[i++] & MASK);
            if (length > location) [[unlikely]] {
                return alphabet_type::revert(current);
            }
            location -= length;
        }
    }

    uint32_t rank(uint8_t c, uint32_t location) {
        uint16_t* data = reinterpret_cast<uint16_t*>(this);
        c = alphabet_type::convert(c);
        uint32_t res = 0;
        uint32_t i = 0;
        while (true) {
            uint8_t current = data[i] >> SHIFT;
            uint16_t length = 1 + (data[i++] & MASK);
            if (location >= length) [[likely]] {
                location -= length;
                res += current == c ? length : 0;
            } else {
                res += current == c ? location : 0;
                return res;
            }
        }
    }

    uint64_t commit(uint8_t** scratch) {
        uint64_t bytes = reinterpret_cast<uint64_t*>(scratch[0])[0];
        bytes *= 2;
        uint16_t* data = reinterpret_cast<uint16_t*>(this);
        uint16_t* src = reinterpret_cast<uint16_t*>(scratch[1]);
        std::memcpy(data, src, bytes);
        return bytes;
    }

    void clear() {}
};
}  // namespace bbwt
