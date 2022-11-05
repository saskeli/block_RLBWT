#pragma once

#include <cstdint>
#include <cstring>

#include "debug.hpp"

namespace bbwt {
template <class block_a, class block_b>
class d_block {
   private:
    uint8_t b_type;
    static_assert(block_a::cap == block_b::cap);
   public:
    typedef typename block_a::alphabet_type alphabet_type;
    static const constexpr bool has_members = true;
    static const constexpr uint32_t cap = block_a::cap;
    static const constexpr uint32_t scratch_blocks =
        1 + block_a::scratch_blocks + block_b::scratch_blocks;
    static const constexpr uint32_t min_size =
        1 + (block_a::min_size < block_b::min_size ? block_a::min_size
                                                   : block_b::min_size);
    static const constexpr uint32_t padding_bytes =
        block_a::padding_bytes > block_b::padding_bytes
            ? block_a::padding_bytes
            : block_b::padding_bytes;

    static const constexpr uint32_t max_size =
        1 + (block_a::max_size > block_b::max_size ? block_a::max_size
                                                   : block_b::max_size);
    static constexpr uint64_t scratch_size(uint32_t i) {
        if (i == 0) {
            return sizeof(block_a) + sizeof(block_b);
        }
        if (i > block_a::scratch_blocks) {
            i--;
            i -= block_a::scratch_blocks;
            return block_b::scratch_size(i);
        } else {
            i--;
            return block_a::scratch_size(i);
        }
    }

    d_block() : b_type(0) {}

    d_block(const d_block& other) = delete;
    d_block(d_block&& other) = delete;
    d_block& operator=(d_block&& other) = delete;
    d_block& operator=(const d_block&) = delete;

    uint32_t append(uint8_t head, uint32_t length, uint8_t** scratch) {
        uint32_t a_size = reinterpret_cast<block_a*>(scratch[0])
                              ->append(head, length, scratch + 1);
        uint32_t b_size =
            reinterpret_cast<block_b*>(scratch[0] + sizeof(block_a))
                ->append(head, length, scratch + 1 + block_a::scratch_blocks);
        if (b_size < a_size) {
            b_type = 1;
            return b_size;
        } else {
            b_type = 0;
            return a_size;
        }
    }

    uint8_t at(uint32_t location) const {
        if (b_type) {
            return reinterpret_cast<const block_b*>(&b_type + 1)->at(location);
        } else {
            return reinterpret_cast<const block_a*>(&b_type + 1)->at(location);
        }
    }

    uint32_t rank(uint8_t c, uint32_t location) const {
        if (b_type) {
            return reinterpret_cast<const block_b*>(&b_type + 1)->rank(c, location);
        } else {
            return reinterpret_cast<const block_a*>(&b_type + 1)->rank(c, location);
        }
    }

    uint64_t commit(uint8_t** scratch) {
        uint64_t bytes = 1;
        if (b_type) {
            b_blocks++;
            if constexpr (block_b::has_members) {
                std::memcpy(&b_type + 1, scratch[0] + sizeof(block_a),
                            sizeof(block_b));
            }
            bytes += reinterpret_cast<block_b*>(&b_type + 1)
                         ->commit(scratch + 1 + block_a::scratch_blocks);
        } else {
            a_blocks++;
            if constexpr (block_a::has_members) {
                std::memcpy(&b_type + 1, scratch[0], sizeof(block_a));
            }
            bytes += reinterpret_cast<block_a*>(&b_type + 1)
                         ->commit(scratch + 1);
        }
        return bytes;
    }

    void print(uint32_t sb) {
        if (b_type) {
            reinterpret_cast<const block_b*>(&b_type + 1)->print(sb);
        } else {
            reinterpret_cast<const block_a*>(&b_type + 1)->print(sb);
        }
    }

    void clear() {}
};
}  // namespace bbwt
