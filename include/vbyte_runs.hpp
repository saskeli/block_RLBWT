#pragma once

#include <cstdint>
#include <cstring>
#include <vector>

namespace bbwt {
template <uint32_t block_size, class alphabet_type_>
class vbyte_runs {
   public:
    typedef alphabet_type_ alphabet_type;
    static const constexpr uint32_t cap = block_size;
    static const constexpr uint32_t scratch_blocks = 2;
    static const constexpr uint32_t min_size = block_size;
    static const constexpr uint32_t padding_bytes = 0;
    
    static const constexpr uint32_t max_size = 6 * block_size;

    static constexpr uint64_t scratch_size(uint32_t i) {
        if (i == 0) {
            return 8;
        } else {
            return max_size;
        }
    }

    vbyte_runs() {}

    vbyte_runs(const vbyte_runs& other) = delete;
    vbyte_runs(vbyte_runs&& other) = delete;
    vbyte_runs& operator=(vbyte_runs&& other) = delete;
    vbyte_runs& operator=(const vbyte_runs&) = delete;

    uint32_t append(uint8_t head, uint32_t length, uint8_t** scratch) {
        const uint8_t SHIFT = 8 - alphabet_type::width;
        const uint8_t BYTE_MASK = SHIFT > 0 ? (uint8_t(1) << (SHIFT - 1)) - 1 : 0;
        uint64_t* offset = reinterpret_cast<uint64_t*>(scratch[0]);
        uint8_t* data = scratch[1];
        --length;
        data[offset[0]] = head << SHIFT;
        if (alphabet_type::width < 7) {
            if (length <= BYTE_MASK) {
                data[offset[0]++] |= length;
                return offset[0];
            } else {
                data[offset[0]++] |= (uint8_t(1) << (SHIFT - 1)) | (length & BYTE_MASK);
                length >>= SHIFT - 1;
            }
        } else if (alphabet_type::width == 7) {
            if (length == 0) {
                offset[0]++;
                return offset[0];
            } else {
                data[offset[0]++] |= uint8_t(1) << (SHIFT - 1);
            }
        } if (alphabet_type::width == 8) {
            offset[0]++;
        }
        const constexpr uint8_t mask = 0b01111111;
        const constexpr uint8_t c_flag = 0b10000000;
        while (length > mask) {
            data[offset[0]++] = c_flag | (length & mask);
            length >>= 7;
        }
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
        uint32_t res = 0;
        uint32_t i = 0;
        
        while (true) {
            uint8_t current;
            uint32_t rl;
            read(i, current, rl);
            rl++;
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
    uint64_t i_rank(uint8_t& c, uint32_t location, vec& counts) const {
        uint32_t i = 0;
        
        while (true) {
            uint8_t current;
            uint32_t rl;
            read(i, current, rl);
            rl++;
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
    void interval_statistics(uint32_t start, uint32_t end, vec& s_counts, vec& e_counts) const {
        uint32_t i = 0;
        end = end - start;
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
                end -= rl - start;
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

    uint64_t select(uint64_t q, uint8_t c) const {
        uint64_t ret = 0;
        uint32_t i = 0;
        while (true) {
            uint8_t current;
            uint32_t rl;
            read(i, current, rl);
            rl++;
            if (current == c) {
                if (rl >= q) {
                    return ret + q;
                } else {
                    q -= rl;
                }
            }
            ret += rl;
        }
    }

    uint64_t commit(uint8_t** scratch) {
        uint64_t bytes = reinterpret_cast<uint64_t*>(scratch[0])[0];
        uint8_t* data = reinterpret_cast<uint8_t*>(this);
        std::memcpy(data, scratch[1], bytes);
        return bytes;
    }

    template<class T>
    uint64_t write(T& out, uint8_t** scratch) {
        uint64_t bytes = reinterpret_cast<uint64_t*>(scratch[0])[0];
        out.write(reinterpret_cast<char*>(scratch[1]), bytes);
        return bytes;
    }

    void clear() {}
    static void write_statics(std::fstream&) {return; }
    static uint64_t load_statics(std::fstream&) {return 0; }

    void print(uint32_t syms) const {
        uint32_t i = 0;
        while  (true) {
            uint8_t current;
            uint32_t rl;
            read(i, current, rl);
            rl++;
            std::cerr << " run " << int(alphabet_type::revert(current)) << ", " << rl << std::endl;
            if (rl < syms) {
                syms -= rl;
            } else {
                break;
            }
        }
    }

   private:
    inline void read(uint32_t& i, uint8_t& c, uint32_t& rl) const {
        const uint8_t SHIFT = 8 - alphabet_type::width;
        const uint8_t BYTE_MASK = SHIFT > 0 ? (uint8_t(1) << (SHIFT - 1)) - 1 : 0;
        const uint8_t* data = reinterpret_cast<const uint8_t*>(this);
        uint8_t offset = 0;
        if (alphabet_type::width == 8) {
            c = data[i++];
            rl = 0;
        } else if (alphabet_type::width == 7) {
            c = data[i] >> 1;
            rl = 0;
            if ((data[i++] & 0b00000001) == 0) {
                return;
            }
        } else {
            c = data[i] >> SHIFT;
            rl = data[i] & BYTE_MASK;
            if (((data[i++] >> (SHIFT - 1)) & 1) == 0) {
                return;
            }
            offset = SHIFT - 1;
        }
        const constexpr uint8_t c_flag = 0b10000000;
        const constexpr uint8_t mask = 0b01111111;
        while (data[i] & c_flag) {
            rl |= (data[i++] & mask) << offset;
            offset += 7;
        }
        rl |= (data[i++] & mask) << offset;
    }
};
}  // namespace bbwt