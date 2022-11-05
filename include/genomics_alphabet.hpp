#include <bit>
#include <cstdint>

namespace bbwt {
template <class dtype>
class genomics_alphabet {
   private:
    static const constexpr uint8_t ONE = 0b00000001;
    static const constexpr uint8_t ZERO = 0b00000000;
    static const constexpr uint8_t MASK = 0b00011111;

   public:
    static const constexpr uint8_t width = 5;
    static constexpr uint8_t convert(uint8_t c) {
        uint8_t v = (~c >> 6) & (c >> 5) & ONE;
        c |= (v << 4) | (v << 2);
        v = ((c >> 6) | (c >> 5)) & 1;
        return (c * v) & MASK;
    }
    static constexpr uint8_t revert(uint8_t c) {
        uint8_t is_zero = c == 0 ? ONE : ZERO;
        uint8_t is_not_zero = is_zero ^ ONE;
        uint8_t is_star_underscore = std::popcount(c) == 4 ? ONE : ZERO;
        is_star_underscore =
            is_star_underscore & ((c & ONE) ^ ((c >> 1) & ONE));
        uint8_t is_cap = is_star_underscore ^ ONE;
        uint8_t is_star = (c >> 1) & is_star_underscore;
        c |= is_cap << 6;
        c |= is_star_underscore << 5;
        c ^= is_star_underscore << 4;
        c ^= is_star << 2;
        return (is_not_zero * c) | (is_zero * '\n');
    }

   private:
    dtype counts_[32];

   public:
    genomics_alphabet() : counts_() {}

    genomics_alphabet(const genomics_alphabet& other) {
        std::memcpy(this, &other, sizeof(genomics_alphabet));
    }

    genomics_alphabet(genomics_alphabet&& other) = delete;

    genomics_alphabet& operator=(const genomics_alphabet& other) {
        std::memcpy(this, &other, sizeof(genomics_alphabet));
    }

    genomics_alphabet& operator=(genomics_alphabet&& other) = delete;

    void add(uint8_t c, dtype v) { counts_[c] += v; }

    void clear() { std::memset(this, 0, sizeof(genomics_alphabet)); }

    dtype p_sum(uint8_t c) const { return counts_[c]; }

    void print() const {
        for (uint32_t i = 0; i < 32; i++) {
            std::cerr << int(revert(i)) << ": " << counts_[i] << std::endl;
        }
    }
};
}  // namespace bbwt
