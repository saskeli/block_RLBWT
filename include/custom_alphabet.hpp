#include <iostream>

namespace bbwt {
template <class dtype>
class custom_alphabet {
  public:
    static const constexpr uint8_t width = 4;
    static constexpr uint8_t convert(uint8_t c) {
        if (c == 65) {
            return 0;
        } else if (c == 84) {
            return 1;
        } else if (c == 67) {
            return 2;
        } else if (c == 71) {
            return 3;
        } else if (c == 78) {
            return 4;
        } else if (c == 89) {
            return 5;
        } else if (c == 82) {
            return 6;
        } else if (c == 83) {
            return 7;
        } else if (c == 77) {
            return 8;
        } else if (c == 75) {
            return 9;
        } else if (c == 87) {
            return 10;
        } else if (c == 66) {
            return 11;
        } else if (c == 72) {
            return 12;
        } else if (c == 68) {
            return 13;
        } else {
            return 14;
        }
    }
    static constexpr uint8_t revert(uint8_t c) {
        if (c == 0) {
            return 65;
        } else if (c == 1) {
            return 84;
        } else if (c == 2) {
            return 67;
        } else if (c == 3) {
            return 71;
        } else if (c == 4) {
            return 78;
        } else if (c == 5) {
            return 89;
        } else if (c == 6) {
            return 82;
        } else if (c == 7) {
            return 83;
        } else if (c == 8) {
            return 77;
        } else if (c == 9) {
            return 75;
        } else if (c == 10) {
            return 87;
        } else if (c == 11) {
            return 66;
        } else if (c == 12) {
            return 72;
        } else if (c == 13) {
            return 68;
        } else {
            return 86;
        }
    }
  private:
    dtype c0 = 0;
    dtype c1 = 0;
    dtype c2 = 0;
    dtype c3 = 0;
    uint32_t c4 = 0;
    uint16_t c5 = 0;
    uint16_t c6 = 0;
    uint16_t c7 = 0;
    uint16_t c8 = 0;
    uint16_t c9 = 0;
    uint16_t c10 = 0;
    uint8_t c11 = 0;
    uint8_t c12 = 0;
    uint8_t c13 = 0;
    uint8_t c14 = 0;
  public:
    custom_alphabet() {}
    custom_alphabet(const custom_alphabet& other) {
        std::memcpy(this, &other, sizeof(custom_alphabet));
    }
    custom_alphabet(custom_alphabet&& other) = delete;
    custom_alphabet& operator=(const custom_alphabet& other) {
        std::memcpy(this, &other, sizeof(custom_alphabet));
    }    custom_alphabet& operator=(custom_alphabet&& other) = delete;
    void add (uint8_t c, dtype v) {
        if (c == 65) {
            c0 += v;
        } else if (c == 84) {
            c1 += v;
        } else if (c == 67) {
            c2 += v;
        } else if (c == 71) {
            c3 += v;
        } else if (c == 78) {
            c4 += v;
        } else if (c == 89) {
            c5 += v;
        } else if (c == 82) {
            c6 += v;
        } else if (c == 83) {
            c7 += v;
        } else if (c == 77) {
            c8 += v;
        } else if (c == 75) {
            c9 += v;
        } else if (c == 87) {
            c10 += v;
        } else if (c == 66) {
            c11 += v;
        } else if (c == 72) {
            c12 += v;
        } else if (c == 68) {
            c13 += v;
        } else {
            c14 += v;
        }
    }

    void clear() {
        std::memset(this, 0, sizeof(custom_alphabet));
    }

    dtype p_sum(uint8_t c) {
        if (c == 65) {
            return c0;
        } else if (c == 84) {
            return c1;
        } else if (c == 67) {
            return c2;
        } else if (c == 71) {
            return c3;
        } else if (c == 78) {
            return c4;
        } else if (c == 89) {
            return c5;
        } else if (c == 82) {
            return c6;
        } else if (c == 83) {
            return c7;
        } else if (c == 77) {
            return c8;
        } else if (c == 75) {
            return c9;
        } else if (c == 87) {
            return c10;
        } else if (c == 66) {
            return c11;
        } else if (c == 72) {
            return c12;
        } else if (c == 68) {
            return c13;
        } else {
            return c14;
        }
    }
};
} // namespace bbwt
