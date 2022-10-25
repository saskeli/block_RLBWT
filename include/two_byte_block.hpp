#pragma once

#include <cstdint>
#include <immintrin.h>

namespace bbwt {
template <uint32_t block_size, class alphabet_type_, bool avx = false>
class two_byte_block {
   public:
    typedef alphabet_type_ alphabet_type;

   private:
    static_assert(alphabet_type::width < 8);
    static_assert(block_size <= ~uint32_t(0) >> 1);
    static const constexpr uint16_t SHIFT = 16 - alphabet_type::width;
    static const constexpr uint16_t LIMIT = uint16_t(1) << SHIFT;
    static const constexpr uint16_t MASK = LIMIT - 1;
    static const constexpr uint16_t AVX_COUNT = 16;
    inline static const __m256i CMASK = _mm256_set1_epi16((uint16_t(1) << alphabet_type::width) - 1);
    inline static const __m256i VMASK = _mm256_set1_epi16(MASK);
    inline static const __m256i ONES = _mm256_set1_epi16(1);
   public:
    static const constexpr uint32_t cap = block_size;
    static const constexpr uint32_t scratch_blocks = 2;
    static const constexpr uint32_t min_size =
        block_size / LIMIT + (block_size % LIMIT ? 2 : 0);
    static const constexpr uint32_t padding_bytes = avx ? 32 : 0;

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
        if constexpr (LIMIT < cap) {
            while (length > LIMIT) {
                data[offset[0]++] = (head << SHIFT) | MASK;
                length -= LIMIT;
            }
        }
        data[offset[0]++] = (head << SHIFT) | (length - 1);
        return offset[0] * 2;
    }

    uint8_t at(uint32_t location) const {
        if constexpr (avx) {
            avx_at(location);
        }
        const uint16_t* data = reinterpret_cast<const uint16_t*>(this);
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

    uint32_t rank(uint8_t c, uint32_t location) const {
        if constexpr (avx) {
            avx_rank(c, location);
        }
        const uint16_t* data = reinterpret_cast<const uint16_t*>(this);
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

   private:
    inline uint32_t sum32(__m256i a) const {
        __m128i low = _mm256_castsi256_si128(a);
        __m128i high = _mm256_extracti128_si256(a, 1);
        low = _mm_add_epi32(low, high);

        high = _mm_move_epi64(low);
        low = _mm_add_epi32(low, high);
        return _mm_extract_epi32(low, 1) + _mm_extract_epi32(low, 2);
    }

    uint8_t avx_at(uint32_t location) const {
        __m256i* vdata = reinterpret_cast<__m256i*>(this);
        uint32_t i = 0;
        uint32_t length = 0;
        while (true) {
            __m256i v = _mm256_lddqu_si256(vdata + i);
            i++;
            v = _mm256_and_si256(v, VMASK);
            v = _mm256_add_epi16(v, ONES);
            v = _mm256_madd_epi16(v, ONES);
            length += sum32(v);
            if (length > location) [[unlikely]] {
                break;
            }
        }
        i--;
        uint16_t* vu = reinterpret_cast<uint16_t*>(vdata + i);
        for (uint16_t ii = AVX_COUNT - 1; ii < AVX_COUNT; ii--) {
            uint16_t v = vu[ii] & MASK;
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
        c = alphabet_type::convert(c);
        const __m256i ccomp = _mm256_set1_epi16(c);
        
        uint32_t res = 0;
        uint32_t length = 0;
        uint32_t i = 0;
        while (true) {
            __m256i v = _mm256_lddqu_si256(vdata + i);
            i++;
            __m256i cvec = v >> SHIFT;
            cvec = _mm256_and_si256(cvec, CMASK);
            cvec = _mm256_cmpeq_epi16(cvec, ccomp);
            v = _mm256_and_si256(v, VMASK);
            v = _mm256_add_epi16(v, ONES);
            cvec = _mm256_madd_epi16(v, cvec);
            v = _mm256_madd_epi16(v, ONES);
            res += sum32(cvec);
            length += sum32(v);
            if (length >= location) [[unlikely]] {
                break;
            }
        }
        i--;
        const uint16_t* data = reinterpret_cast<const uint16_t*>(vdata + i);
        for (uint32_t ii = AVX_COUNT - 1; ii < AVX_COUNT; ii--) {
            uint16_t v = data[ii] & MASK;
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
};
}  // namespace bbwt
