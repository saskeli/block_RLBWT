#pragma once

#include <cstring>
#include <iostream>
#include <utility>

namespace bbwt {
template <uint32_t block_size, class alphabet_type_>
class byte_block {
   private:
    static_assert(block_size < uint32_t(1) << (32 - alphabet_type_::width));
   public:
    typedef alphabet_type_ alphabet_type;
    static const constexpr bool has_members = false;
    static const constexpr uint32_t cap = block_size;
    static const constexpr uint32_t scratch_blocks = 2;
    static const constexpr uint32_t min_size = 5;
    static const constexpr uint32_t padding_bytes = 0;
    
    static const constexpr uint32_t max_size = 2 * block_size;

    static constexpr uint64_t scratch_size(uint32_t i) {
        if (i == 0) {
            return 8;
        } else {
            return max_size;
        }
    }

  private: 
    static const constexpr uint8_t SHIFT = 8 - alphabet_type::width;
    static const constexpr uint8_t BYTE_MASK = SHIFT > 0 ? (uint8_t(1) << (SHIFT - 1)) - 1 : 0;
  public:
    byte_block() {}

    byte_block(const byte_block& other) = delete;
    byte_block(byte_block&& other) = delete;
    byte_block& operator=(byte_block&& other) = delete;
    byte_block& operator=(const byte_block&) = delete;

    uint32_t append(uint8_t head, uint32_t length, uint8_t** scratch) {
        //std::cerr << "append(" << head << ", " << length << ")" << std::endl;
        uint64_t* offset = reinterpret_cast<uint64_t*>(scratch[0]);
        uint8_t* data = scratch[1];
        length--;
        //std::cerr << length << " in bin is " << std::bitset<32>(length) << std::endl;
        head = alphabet_type::convert(head);
        data[offset[0]] = head << SHIFT;
        //std::cerr << "head as written = " << std::bitset<8>(data[offset[0]]) << std::endl;
        if constexpr (alphabet_type::width < 7) {
            if (length <= BYTE_MASK) {
                data[offset[0]++] |= length;
                return offset[0];
            } else {
                data[offset[0]++] |= (uint8_t(1) << (SHIFT - 1)) | (length & BYTE_MASK);
                length >>= SHIFT - 1;
                //std::cerr << "with continuation = " << std::bitset<8>(data[offset[0] - 1]) << std::endl;
            }
        } else if constexpr (alphabet_type::width == 7) {
            if (length == 1) {
                data[offset[0]++] |= length;
                return offset[0];
            }
        } if constexpr (alphabet_type::width == 8) {
            offset[0]++;
        }
        if constexpr (block_size < uint32_t(1) << (8 + SHIFT - 1)) {
            data[offset[0]++] = length;
            return offset[0];
        }
        if (length <= 0b01111111) {
            data[offset[0]++] = 0b10000000 | length;
            return offset[0];
        }
        data[offset[0]++] = 0b01111111 & length;
        //std::cerr << "first full byte = " << std::bitset<8>(data[offset[0] - 1]) << std::endl;
        length >>= 7;
        if constexpr (block_size <= uint32_t(1) << (15 + SHIFT - 1)) {
            data[offset[0]++] = length;
            //std::cerr << "rest (" << length << ") fit in third word = " << std::bitset<8>(data[offset[0] - 1]) << std::endl;
            return offset[0];
        }
        if (length <= 0b01111111) {
            data[offset[0]++] = 0b10000000 | length;
            return offset[0];
        }
        data[offset[0]++] = 0b01111111 & length;
        length >>= 7;
        data[offset[0]++] = length;
        return offset[0];
    }

    uint8_t at(uint32_t location) const {
        uint32_t i = 0;
        while (true) {
            uint8_t c;
            uint32_t rl;
            read(i, c, rl);
            rl++;
            if (location >= rl) {
                location -= rl;
            } else {
                return alphabet_type::revert(c);
            }
        }
    }

    uint32_t rank(uint8_t c, uint32_t location) const {
        //std::cerr << "rank(" << c << ", " << location << ")" << std::endl;
        c = alphabet_type::convert(c);
        uint32_t res = 0;
        uint32_t i = 0;
        
        while (true) {
            uint8_t current;
            uint32_t rl;
            read(i, current, rl);
            rl++;
            //std::cerr << " run " << alphabet_type::revert(current) << ", " << rl << std::endl;
            if (location >= rl) [[likely]] {
                location -= rl;
                res += current == c ? rl : 0;
            } else {
                res += current == c ? location : 0;
                return res;
            }
        }
    }

    uint64_t commit(uint8_t** scratch) {
        uint64_t* bytes = reinterpret_cast<uint64_t*>(scratch[0]);
        uint8_t* data = reinterpret_cast<uint8_t*>(this);
        std::memcpy(data, scratch[1], bytes[0]);
        //std::cerr << "Data:" << std::endl;
        //for (uint64_t i = 0; i < bytes[0]; i++) {
        //    std::cerr << std::bitset<8>(data[i]) << " ";
        //    if (i % 8 == 7) {
        //        std::cerr << std::endl;
        //    }
        //}
        //std::cerr << std::endl;
        return bytes[0];
    }

    void clear() {}

   private:
    inline void read(uint32_t& i, uint8_t& c, uint32_t& rl) const {
        const uint8_t* data = reinterpret_cast<const uint8_t*>(this);
        if constexpr (alphabet_type::width == 8) {
            c = data[i++];
            rl = 0;
            return get<0>(i, rl);
        } else if constexpr (alphabet_type::width == 7) {
            c = data[i] >> 1;
            if (data[i++] & 0b00000001) {
                rl = 1;
                return;
            } else {
                rl = 0;
                return get<0>(i, rl);
            }
        } else {
            c = data[i] >> SHIFT;
            rl = data[i] & BYTE_MASK;
            //std::cerr << " last bits: " << std::bitset<32>(rl) << std::endl;
            if ((data[i++] >> (SHIFT - 1)) & 1) {
                return get<SHIFT - 1>(i, rl);
            } else {
                return;
            }
        }
    }

    template<uint8_t offset>
    inline void get(uint32_t& i, uint32_t& rl) const {
        const uint8_t* data = reinterpret_cast<const uint8_t*>(this);
        if constexpr (block_size <= (uint32_t(1) << (8 + offset))) {
            rl |= data[i++] << offset;
            return;
        }
        rl |= (data[i] & 0b01111111) << offset;
        //std::cerr << " first additional byte: " << std::bitset<8>(data[i]) << std::endl;
        //std::cerr << "  -> " << std::bitset<32>(rl) << std::endl;
        if (data[i++] & 0b10000000) {
            //std::cerr << " DONE WTF!" << std::endl;
            return;
        }
        if constexpr (block_size <= (uint32_t(1) << (15 + offset))) {
            rl |= data[i++] << (offset + 7);
            //std::cerr << " final byte: " << std::bitset<8>(data[i - 1]) << std::endl;
            //std::cerr << "  -> " << std::bitset<32>(rl) << std::endl;
            return;
        }
        rl |= (data[i] & 0b01111111) << offset;
        if (data[i++] & 0b10000000) {
            return;
        }
        rl |= data[i++] << (offset + 14);
    }
};
}  // namespace bbwt
