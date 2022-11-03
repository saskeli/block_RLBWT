#include <cstdint>
#include <bit>

namespace bbwt {
template <class dtype>
class acgt_alphabet {
  private:
    
  public:
    static const constexpr uint8_t width = 3;
    static constexpr uint8_t convert(uint8_t c) {
        switch (c) {
            case 'A':
                return 1;
            case 'C':
                return 2;
            case 'G':
                return 3;
            case 'T':
                return 4;
            default:
                return 0;
        }
    }
    static constexpr uint8_t revert(uint8_t c) {
        switch (c) {
            case 1:
                return 'A';
            case 2:
                return 'C';
            case 3:
                return 'G';
            case 4:
                return 'T';
            default:
                return 'N';
        }
    }
  private:
    dtype counts[5];
  public:
    acgt_alphabet() : counts() {}

    acgt_alphabet(const acgt_alphabet& other) {
        std::memcpy(this, &other, sizeof(acgt_alphabet));
    }

    acgt_alphabet(acgt_alphabet&& other) = delete;

    acgt_alphabet& operator=(const acgt_alphabet& other) {
        std::memcpy(this, &other, sizeof(acgt_alphabet));
    }

    acgt_alphabet& operator=(acgt_alphabet&& other) = delete;

    void add (uint8_t c, dtype v) {
        counts[c] += v;
    }

    void clear() {
        std::memset(this, 0, sizeof(acgt_alphabet));
    }

    dtype p_sum(uint8_t c) const {
        return counts[c];
    }
};
} // namespace bbwt
