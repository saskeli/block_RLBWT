#pragma once

#include <immintrin.h>
#include <cstdint>

namespace bbwt {
template <uint32_t block_size, class alphabet_type_, bool avx = false>
class one_byte_block {
  public:
    typedef alphabet_type_ alphabet_type;
  private:
    static_assert(block_size <= ~uint32_t(0) >> 1);
#ifndef __AVX2__
    static_assert(avx == false);
#endif
#ifdef __AVX2__
    static const constexpr uint32_t AVX_COUNT = 32;
    inline static const __m256i ONES = _mm256_set1_epi8(1);
    inline static const __m256i ZEROS = _mm256_setzero_si256();
#endif
   public:
    static const constexpr bool has_members = false;
    static const constexpr uint32_t cap = block_size;
    static const constexpr uint32_t scratch_blocks = 2;
    static const constexpr uint32_t min_size = 2;
    static const constexpr uint32_t padding_bytes = avx ? 32 : 0;

    static const constexpr uint32_t max_size = block_size;
    static constexpr uint64_t scratch_size(uint32_t i) {
        if (i == 0) {
            return 8;
        } else {
            return max_size;
        }
    }

    one_byte_block() {}

    one_byte_block(const one_byte_block& other) = delete;
    one_byte_block(one_byte_block&& other) = delete;
    one_byte_block& operator=(one_byte_block&& other) = delete;
    one_byte_block& operator=(const one_byte_block&) = delete;

    uint32_t append(uint8_t head, uint32_t length, uint8_t** scratch) {
        const uint16_t SHIFT = 8 - alphabet_type::width;
        const uint16_t LIMIT = uint16_t(1) << SHIFT;
        const uint16_t MASK = LIMIT - 1;
        uint64_t* offset = reinterpret_cast<uint64_t*>(scratch[0]);
        uint8_t* data = reinterpret_cast<uint8_t*>(scratch[1]);
        if (LIMIT < cap) {
            while (length > LIMIT) {
                data[offset[0]++] = (head << SHIFT) | MASK;
                length -= LIMIT;
            }
        }
        data[offset[0]++] = (head << SHIFT) | (length - 1);
        return offset[0];
    }

    uint8_t at(uint32_t location) const {
#ifdef __AVX2__
        if  constexpr (avx) {
            return avx_at(location);
        }
#endif  
        const uint16_t SHIFT = 8 - alphabet_type::width;
        const uint16_t LIMIT = uint16_t(1) << SHIFT;
        const uint16_t MASK = LIMIT - 1;
        const uint8_t* data = reinterpret_cast<const uint8_t*>(this);
        uint32_t i = 0;
        while (true) {
            uint8_t current = data[i] >> SHIFT;
            uint8_t length = 1 + (data[i++] & MASK);
            if (length > location) [[unlikely]] {
                return current;
            }
            location -= length;
        }
    }

    uint32_t rank(uint8_t c, uint32_t location) const {
#ifdef __AVX2__
        if constexpr (avx) {
            return avx_rank(c, location);
        }
#endif
        const uint16_t SHIFT = 8 - alphabet_type::width;
        const uint16_t LIMIT = uint16_t(1) << SHIFT;
        const uint16_t MASK = LIMIT - 1;
        const uint8_t* data = reinterpret_cast<const uint8_t*>(this);
        uint32_t res = 0;
        uint32_t i = 0;
        while (true) {
            uint8_t current = data[i] >> SHIFT;
            uint8_t length = 1 + (data[i++] & MASK);
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
        uint8_t* data = reinterpret_cast<uint8_t*>(this);
        uint8_t* src = reinterpret_cast<uint8_t*>(scratch[1]);
        std::memcpy(data, src, bytes);
        return bytes;
    }

    void clear() {}
    static void write_statics(std::fstream&) {return; }
    static uint64_t load_statics(std::fstream&) {return 0; }

    void print(uint32_t sb) const {
        const uint8_t* data = reinterpret_cast<const uint8_t*>(this);
        const uint16_t SHIFT = 8 - alphabet_type::width;
        const uint16_t LIMIT = uint16_t(1) << SHIFT;
        const uint16_t MASK = LIMIT - 1;
        uint32_t i = 0;
        while (true) {
            uint8_t current = data[i] >> SHIFT;
            uint8_t length = 1 + (data[i++] & MASK);
            std::cerr << "run " << int(current) << ", " << length << std::endl;
            if (sb > length) [[likely]] {
                sb -= length;
            } else {
                return;
            }
        }
    }
#ifdef __AVX2__
  private:

    inline uint32_t sum_epi8(__m256i a) const {
        a = _mm256_sad_epu8(a, ZEROS);
        __m128i low = _mm256_castsi256_si128(a);
        __m128i high = _mm256_extracti128_si256(a, 1);
        low = _mm_add_epi64(low, high);
        return _mm_extract_epi64(low, 0) + _mm_extract_epi64(low, 1);
    }

    uint8_t avx_at(uint32_t location) const {
        const uint16_t SHIFT = 8 - alphabet_type::width;
        const uint16_t LIMIT = uint16_t(1) << SHIFT;
        const uint16_t MASK = LIMIT - 1;
        const __m256i VMASK = _mm256_set1_epi8(MASK);
        const __m256i* vdata = reinterpret_cast<const __m256i*>(this);
        uint32_t i = 0;
        uint32_t length = 0;
        while (true) {
            __m256i v = _mm256_lddqu_si256(vdata + i);
            i++;
            v = _mm256_and_si256(v, VMASK);
            v = _mm256_add_epi16(v, ONES);
            length += sum_epi8(v);
            if (length > location) [[unlikely]] {
                break;
            }
        }
        i--;
        const uint8_t* vu = reinterpret_cast<const uint8_t*>(vdata + i);
        for (uint16_t ii = AVX_COUNT - 1; ii < AVX_COUNT; ii--) {
            uint8_t v = vu[ii] & MASK;
            v++;
            length -= v;
            if (length <= location) [[unlikely]] {
                return vu[ii] >> SHIFT;
            }
        }
        return 0;
    }

    uint32_t avx_rank(uint8_t c, uint32_t location) const {
        const __m256i* vdata = reinterpret_cast<const __m256i*>(this);
        const __m256i ccomp = _mm256_set1_epi8(c);
        const uint16_t SHIFT = 8 - alphabet_type::width;
        const uint16_t LIMIT = uint16_t(1) << SHIFT;
        const uint16_t MASK = LIMIT - 1;
        const __m256i CMASK = _mm256_set1_epi8((uint16_t(1) << alphabet_type::width) - 1);
        const __m256i VMASK = _mm256_set1_epi8(MASK);

        uint32_t res = 0;
        uint32_t length = 0;
        uint32_t i = 0;
        while (true) {
            __m256i v = _mm256_lddqu_si256(vdata + i);
            i++;
            __m256i cvec = v >> SHIFT;
            cvec = _mm256_and_si256(cvec, CMASK);
            cvec = _mm256_cmpeq_epi8(cvec, ccomp);
            v = _mm256_and_si256(v, VMASK);
            v = _mm256_add_epi8(v, ONES);
            cvec = _mm256_blendv_epi8(ZEROS, v, cvec);
            res += sum_epi8(cvec);
            length += sum_epi8(v);
            if (length >= location) [[unlikely]] {
                break;
            }
        }
        i--;
        const uint8_t* data = reinterpret_cast<const uint8_t*>(vdata + i);
        for (uint32_t ii = AVX_COUNT - 1; ii < AVX_COUNT; ii--) {
            uint8_t v = data[ii] & MASK;
            v++;
            length -= v;
            uint8_t current = data[ii] >> SHIFT;
            res -= (current == c) ? v : 0;
            if (length < location) [[unlikely]] {
                if (current == c) [[unlikely]] {
                    res += location - length;
                }
                break;
            }
        }
        return res;
    }
#endif
};
}  // namespace bbwt
