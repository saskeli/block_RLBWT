#include <iostream>
#include <cstring>

namespace bbwt {
template <class dtype, uint8_t smallest, uint8_t largest>
class delta_alphabet {
  private:
    static const constexpr uint16_t A_SIZE = 1 + largest - smallest;
    dtype counts_[A_SIZE];

  public:
    static const constexpr uint8_t width = 8 * sizeof(unsigned int) - __builtin_clz(largest - smallest) ;
    static constexpr uint8_t convert(uint8_t c) {
        return c - smallest;
    }
    static constexpr uint8_t revert(uint8_t c) {
        return c + smallest;
    }

    delta_alphabet() : counts_() {}

    delta_alphabet(const delta_alphabet& other) {
        std::memcpy(counts_, other.counts_, A_SIZE * sizeof(dtype));
    }

    delta_alphabet(delta_alphabet&& other) = delete;

    delta_alphabet& operator=(const delta_alphabet& other) {
        std::memcpy(counts_, other.counts_, A_SIZE * sizeof(dtype));
        return *this;
    }

    delta_alphabet& operator=(delta_alphabet&& other) = delete;

    void add(uint8_t c, dtype v) {
        counts_[c] += v;
    }

    void clear() {
        std::memset(counts_, 0, sizeof(dtype) * A_SIZE);
    }

    dtype p_sum(uint8_t c) const {
        return counts_[c];
    }
};
} // namespace bbwt
