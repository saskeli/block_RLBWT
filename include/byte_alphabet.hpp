#include <iostream>
#include <cstring>

namespace bbwt {
template <class dtype>
class byte_alphabet {
  private:
    static const constexpr uint16_t A_SIZE = 256;
    dtype counts_[A_SIZE];

  public:
    byte_alphabet() : counts_() {}

    byte_alphabet(const byte_alphabet& other) {
        std::memcpy(counts_, other.counts_, A_SIZE * sizeof(dtype));
    }

    byte_alphabet(byte_alphabet&& other) = delete;

    byte_alphabet& operator=(const byte_alphabet& other) {
        std::memcpy(counts_, other.counts_, A_SIZE * sizeof(dtype));
        return *this;
    }

    byte_alphabet& operator=(byte_alphabet&& other) = delete;

    void add(uint8_t c, dtype v) {
        counts_[c] += v;
    }

    void clear() {
        std::memset(counts_, 0, sizeof(dtype) * A_SIZE);
    }

    void p_sum(uint8_t c) {
        return counts_[c];
    }
};
} // namespace bbwt
