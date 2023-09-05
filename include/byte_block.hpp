#pragma once

#include <cstring>
#include <iostream>
#include <utility>
#include <cstdint>

//#define VERB

namespace bbwt {
template <uint32_t block_size, class alphabet_type_>
class byte_block {
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

    byte_block() {}

    byte_block(const byte_block& other) = delete;
    byte_block(byte_block&& other) = delete;
    byte_block& operator=(byte_block&& other) = delete;
    byte_block& operator=(const byte_block&) = delete;

    uint32_t append(uint8_t head, uint32_t length, uint8_t** scratch) {
        const uint8_t SHIFT = 8 - alphabet_type::width;
        const uint8_t BYTE_MASK = SHIFT > 0 ? (uint8_t(1) << (SHIFT - 1)) - 1 : 0;
        #ifdef VERB
        std::cerr << "append(" << head << ", " << length << ")" << std::endl;
        #endif
        uint64_t* offset = reinterpret_cast<uint64_t*>(scratch[0]);
        uint8_t* data = scratch[1];
        length--;
        #ifdef VERB
        std::cerr << length << " in bin is " << std::bitset<32>(length) << std::endl;
        #endif
        data[offset[0]] = head << SHIFT;
        #ifdef VERB
        std::cerr << "head as written = " << std::bitset<8>(data[offset[0]]) << std::endl;
        #endif
        if (alphabet_type::width < 7) {
            if (length <= BYTE_MASK) {
                data[offset[0]++] |= length;
                return offset[0];
            } else {
                data[offset[0]++] |= (uint8_t(1) << (SHIFT - 1)) | (length & BYTE_MASK);
                length >>= SHIFT - 1;
                #ifdef VERB
                std::cerr << "with continuation = " << std::bitset<8>(data[offset[0] - 1]) << std::endl;
                #endif
            }
        } else if (alphabet_type::width == 7) {
            if (length == 1) {
                data[offset[0]++] |= length;
                return offset[0];
            }
        } if (alphabet_type::width == 8) {
            offset[0]++;
            if constexpr (block_size <= uint32_t(1) << 8) {
                data[offset[0]++] = length;
                return offset[0];
            }
        }
        if constexpr (block_size <= uint32_t(1) << (8 + SHIFT - 1)) {
            data[offset[0]++] = length;
            return offset[0];
        }
        if (length <= 0b01111111) {
            data[offset[0]++] = 0b10000000 | length;
            #ifdef VERB
            std::cerr << "first and last full byte = " << std::bitset<8>(data[offset[0] - 1]) << std::endl;
            #endif    
            return offset[0];
        }
        data[offset[0]++] = 0b01111111 & length;
        #ifdef VERB
        std::cerr << "first full byte = " << std::bitset<8>(data[offset[0] - 1]) << std::endl;
        #endif
        length >>= 7;
        if ((block_size <= uint32_t(1) << (15)) && (alphabet_type::width == 8)) {
            data[offset[0]++] = length;
            #ifdef VERB
            std::cerr << "8: rest (" << length << ") fit in third word = " << std::bitset<8>(data[offset[0] - 1]) << std::endl;
            #endif
            return offset[0];
        }
        if constexpr (block_size <= uint32_t(1) << (15 + SHIFT - 1)) {
            data[offset[0]++] = length;
            #ifdef VERB
            std::cerr << "rest (" << length << ") fit in third word = " << std::bitset<8>(data[offset[0] - 1]) << std::endl;
            #endif
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
                return c;
            }
        }
    }

    uint32_t rank(uint8_t c, uint32_t location) const {
        #ifdef VERB
        std::cerr << "rank(" << c << ", " << location << ")" << std::endl;
        #endif
        uint32_t res = 0;
        uint32_t i = 0;
        
        while (true) {
            uint8_t current;
            uint32_t rl;
            read(i, current, rl);
            rl++;
            #ifdef VERB
            std::cerr << " run " << alphabet_type::revert(current) << ", " << rl << std::endl;
            #endif
            if (location >= rl) [[likely]] {
                location -= rl;
                res += current == c ? rl : 0;
            } else {
                res += current == c ? location : 0;
                return res;
            }
        }
    }

    template <class vec>
    uint32_t i_rank(uint8_t& c, uint32_t location, vec& counts) const {
        #ifdef VERB
        std::cerr << "rank(" << c << ", " << location << ")" << std::endl;
        #endif
        uint32_t i = 0;
        
        while (true) {
            uint8_t current;
            uint32_t rl;
            read(i, current, rl);
            rl++;
            #ifdef VERB
            std::cerr << " run " << alphabet_type::revert(current) << ", " << rl << std::endl;
            #endif
            if (location > rl) [[likely]] {
                location -= rl;
                counts[current] += rl;
            } else {
                counts[current] += location;
                c = current;
                return counts[current];
            }
        }
    }

    template <class vec>
    void c_rank(uint32_t loc, vec& counts) const {
        uint32_t i = 0;
        while (true) {
            uint8_t current;
            uint32_t rl;
            read(i, current, rl);
            rl++;
            if (loc > rl) [[likely]] {
                loc -= rl;
                counts[current] += rl;
            } else {
                counts[current] += loc;
                return;
            }
        }
    }

    template <class vec>
    void interval_statistics(uint32_t start, uint32_t end, vec& s_counts, vec& e_counts) {
        uint32_t i = 0;
        end -= start;
        while (true) {
            uint8_t current;
            uint32_t rl;
            read(i, current, rl);
            rl++;
            if (start > rl) [[likely]] {
                start -= rl;
                s_counts[current] += rl;
                e_counts[current] += rl;
            } else {
                s_counts[current] += start;
                if (start + end <= rl) {
                    e_counts[current] += start + end;
                    return;
                }
                e_counts[current] += rl;
                break;
            }
        }
        while (true) {
            uint8_t current;
            uint32_t rl;
            read(i, current, rl);
            rl++;
            if (end > rl) [[likely]] {
                end -= rl;
                e_counts[current] += rl;
            } else {
                e_counts[current] += end;
                return;
            }
        }
    }

    uint32_t select(uint32_t x, uint8_t c) const {
        uint32_t i = 0;
        uint32_t res = 0;
        while (true) {
            uint8_t current;
            uint32_t rl;
            read(i, current, rl);
            ++rl;
            if (c == current) {
                if (rl >= x) {
                    return res + x;
                } else {
                    x -= rl;
                }
            }
            res += rl;
        }
    }

    uint64_t commit(uint8_t** scratch) {
        uint64_t* bytes = reinterpret_cast<uint64_t*>(scratch[0]);
        uint8_t* data = reinterpret_cast<uint8_t*>(this);
        std::memcpy(data, scratch[1], bytes[0]);
        #ifdef VERB
        std::cerr << "Data:" << std::endl;
        for (uint64_t i = 0; i < bytes[0]; i++) {
            std::cerr << std::bitset<8>(data[i]) << " ";
            if (i % 8 == 7) {
                std::cerr << std::endl;
            }
        }
        std::cerr << std::endl;
        #endif
        return bytes[0];
    }

    void clear() {}
    static void write_statics(std::fstream&) {return; }
    static uint64_t load_statics(std::fstream&) {return 0; }

    void print(uint32_t sb) const {
        uint32_t i = 0;
        while (true) {
            uint8_t current;
            uint32_t rl;
            read(i, current, rl);
            rl++;
            std::cerr << " run " << alphabet_type::revert(current) << ", " << rl << std::endl;
            if (sb > rl) [[likely]] {
                sb -= rl;
            } else {
                return;
            }
        }
    }

   private:
    inline void read(uint32_t& i, uint8_t& c, uint32_t& rl) const {
        const uint8_t SHIFT = 8 - alphabet_type::width;
        const uint8_t BYTE_MASK = SHIFT > 0 ? (uint8_t(1) << (SHIFT - 1)) - 1 : 0;
        const uint8_t* data = reinterpret_cast<const uint8_t*>(this);
        if (alphabet_type::width == 8) {
            c = data[i++];
            rl = 0;
            return get(i, rl, 0);
        } else if (alphabet_type::width == 7) {
            c = data[i] >> 1;
            if (data[i++] & 0b00000001) {
                rl = 1;
                return;
            } else {
                rl = 0;
                return get(i, rl, 0);
            }
        } else {
            c = data[i] >> SHIFT;
            rl = data[i] & BYTE_MASK;
            #ifdef VERB
            std::cerr << " last bits: " << std::bitset<32>(rl) << std::endl;
            #endif
            if ((data[i++] >> (SHIFT - 1)) & 1) {
                return get(i, rl, SHIFT - 1);
            } else {
                return;
            }
        }
    }

    inline void get(uint32_t& i, uint32_t& rl, uint8_t offset) const {
        const uint8_t* data = reinterpret_cast<const uint8_t*>(this);
        if (block_size <= (uint32_t(1) << (8 + offset))) {
            rl |= data[i++] << offset;
            return;
        }
        rl |= (data[i] & 0b01111111) << offset;
        #ifdef VERB
        std::cerr << " first additional byte: " << std::bitset<8>(data[i]) << std::endl;
        std::cerr << "  -> " << std::bitset<32>(rl) << std::endl;
        #endif
        if (data[i++] & 0b10000000) {
            #ifdef VERB
            std::cerr << " DONE WTF!" << std::endl;
            #endif
            return;
        }
        if (block_size <= (uint32_t(1) << (15 + offset))) {
            rl |= data[i++] << (offset + 7);
            #ifdef VERB
            std::cerr << " final byte: " << std::bitset<8>(data[i - 1]) << std::endl;
            std::cerr << "  -> " << std::bitset<32>(rl) << std::endl;
            #endif
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
