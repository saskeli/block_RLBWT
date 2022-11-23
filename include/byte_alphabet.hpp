#include <iostream>
#include <cstring>

namespace bbwt {
template <class dtype>
class byte_alphabet {
  public:
    static const constexpr uint8_t width = 8;
    static constexpr uint8_t convert(uint8_t c) {
        return c;
    }
    static constexpr uint8_t revert(uint8_t c) {
        return c;
    }

  private:
    static const constexpr uint16_t A_SIZE = 256;
    dtype counts_[A_SIZE];

  public:
    byte_alphabet() : counts_() {}

    byte_alphabet(const byte_alphabet& other) {
        std::memcpy(counts_, other.counts_, sizeof(byte_alphabet<dtype>));
    }

    byte_alphabet(byte_alphabet&& other) = delete;

    byte_alphabet& operator=(const byte_alphabet& other) {
        std::memcpy(counts_, other.counts_, sizeof(byte_alphabet<dtype>));
        return *this;
    }

    byte_alphabet& operator=(byte_alphabet&& other) = delete;

    void add(uint8_t c, dtype v) {
        counts_[c] += v;
    }

    void clear() {
        std::memset(counts_, 0, sizeof(dtype) * A_SIZE);
    }

    dtype p_sum(uint8_t c) const {
        return counts_[c];
    }

    void print() const {
        for (uint16_t i = 0; i < 256; i++) {
            std::cerr << i << ": " << counts_[i] << std::endl;
        }
    }

    static void write_statics(std::fstream&) {return; }
    static uint64_t load_statics(std::fstream&) {return 0; }
};
} // namespace bbwt
