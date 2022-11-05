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
    dtype c0 = 0;
    dtype c1 = 0;
    dtype c2 = 0;
    dtype c3 = 0;
    uint32_t c4 = 0;
    uint32_t c5 = 0;
    uint16_t c6 = 0;
    uint16_t c7 = 0;
    uint16_t c8 = 0;
    uint16_t c9 = 0;
    uint16_t c10 = 0;
    uint16_t c11 = 0;
    uint16_t c12 = 0;
    uint16_t c13 = 0;
    uint16_t c14 = 0;
    uint16_t c15 = 0;
  public:
    custom_alphabet() {}
    custom_alphabet(const custom_alphabet& other) {
        std::memcpy(this, &other, sizeof(custom_alphabet));
    }
    custom_alphabet(custom_alphabet&& other) = delete;
    custom_alphabet& operator=(const custom_alphabet& other) {
        std::memcpy(this, &other, sizeof(custom_alphabet));
    }
    custom_alphabet& operator=(custom_alphabet&& other) = delete;
    void add (uint8_t c, dtype v) {
        if (c == 67) {
            c0 += v;
        } else if (c == 71) {
            c1 += v;
        } else if (c == 65) {
            c2 += v;
        } else if (c == 84) {
            c3 += v;
        } else if (c == 78) {
            c4 += v;
        } else if (c == 10) {
            c5 += v;
        } else if (c == 89) {
            c6 += v;
        } else if (c == 82) {
            c7 += v;
        } else if (c == 77) {
            c8 += v;
        } else if (c == 75) {
            c9 += v;
        } else if (c == 87) {
            c10 += v;
        } else if (c == 83) {
            c11 += v;
        } else if (c == 86) {
            c12 += v;
        } else if (c == 72) {
            c13 += v;
        } else if (c == 66) {
            c14 += v;
        } else {
            c15 += v;
        }
    }

    void clear() {
        std::memset(this, 0, sizeof(custom_alphabet));
    }

    dtype p_sum(uint8_t c) const {
        if (c == 67) {
            return c0;
        } else if (c == 71) {
            return c1;
        } else if (c == 65) {
            return c2;
        } else if (c == 84) {
            return c3;
        } else if (c == 78) {
            return c4;
        } else if (c == 10) {
            return c5;
        } else if (c == 89) {
            return c6;
        } else if (c == 82) {
            return c7;
        } else if (c == 77) {
            return c8;
        } else if (c == 75) {
            return c9;
        } else if (c == 87) {
            return c10;
        } else if (c == 83) {
            return c11;
        } else if (c == 86) {
            return c12;
        } else if (c == 72) {
            return c13;
        } else if (c == 66) {
            return c14;
        } else {
            return c15;
        }
    }
};
} // namespace bbwt
