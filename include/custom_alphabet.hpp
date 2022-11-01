#include <cstdint>

namespace bbwt {
template <class dtype>
class custom_alphabet {
  private:
    inline static const uint8_t c_map[] = {
          0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
          0, 2, 14, 0, 15, 0, 0, 1, 13, 0, 0, 9, 0, 8, 4, 0,
          0, 0, 7, 11, 3, 0, 12, 10, 0, 6, 0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    inline static const uint8_t r_map[] = {
        67, 71, 65, 84, 78, 10, 89, 82, 77, 75, 87, 83, 86, 72, 66, 68};
  public:
    static const constexpr uint8_t width = 4;
    static constexpr uint8_t convert(uint8_t c) {
        return c_map[c];
    }
    static constexpr uint8_t revert(uint8_t c) {
        return r_map[c];
    }
  private:
    dtype counts[16];
  public:
    custom_alphabet() : counts() {}

    custom_alphabet(const custom_alphabet& other) {
        std::memcpy(this, &other, sizeof(custom_alphabet));
    }

    custom_alphabet(custom_alphabet&& other) = delete;

    custom_alphabet& operator=(const custom_alphabet& other) {
        std::memcpy(this, &other, sizeof(custom_alphabet));
    }

    custom_alphabet& operator=(custom_alphabet&& other) = delete;

    void add (uint8_t c, dtype v) {
        counts[c_map[c]] += v;
    }

    void clear() {
        std::memset(this, 0, sizeof(custom_alphabet));
    }

    dtype p_sum(uint8_t c) const {
        return counts[c_map[c]];
    }
};
} // namespace bbwt
