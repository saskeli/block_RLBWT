#include <endian.h>

#include <cstdint>
#include <utility>

namespace bbwt {
template <class dtype>
class alphabet {
   private:
    typedef std::pair<uint64_t, std::pair<uint32_t, uint32_t>> L;
    typedef std::pair<uint32_t, std::pair<uint16_t, uint16_t>> S;
    inline static uint8_t c_map[256];
    inline static uint8_t r_map[256];
    inline static L* L_map;
    inline static S* S_map;
    inline static uint32_t size_ = 0;

   public:
    inline static uint8_t width;
    static uint8_t convert(uint8_t c) { return c_map[c]; }
    static uint8_t revert(uint8_t c) { return r_map[c]; }
    static uint16_t size() { return size_; }
    template <class i_t>
    static uint32_t load_statics(i_t& in_file) {
        in_file.read(reinterpret_cast<char*>(&width), 1);
        in_file.read(reinterpret_cast<char*>(&size_), 4);
        in_file.read(reinterpret_cast<char*>(c_map), 256);
        in_file.read(reinterpret_cast<char*>(r_map), 256);
        uint32_t s;
        in_file.read(reinterpret_cast<char*>(&s), 4);
        if constexpr (sizeof(dtype) == 4) {
            S_map = (S*)std::malloc(s);
            in_file.read(reinterpret_cast<char*>(S_map), s);
        } else {
            L_map = (L*)std::malloc(s);
            in_file.read(reinterpret_cast<char*>(L_map), s);
        }
        return s + 2 * 256 + sizeof(L*) + sizeof(S*) + 1;
    }

    alphabet() = delete;
    alphabet(const alphabet& other) = delete;
    alphabet(alphabet&& other) = delete;
    alphabet& operator=(const alphabet& other) = delete;
    alphabet& operator=(alphabet&& other) = delete;

    dtype p_sum(uint8_t c) const {
        if constexpr (sizeof(dtype) == 8) {
            dtype d = be64toh(reinterpret_cast<const dtype*>(
                reinterpret_cast<const uint8_t*>(this) +
                L_map[c].second.first)[0]);
            return (d >> L_map[c].second.second) & L_map[c].first;
        } else {
            dtype d = be32toh(reinterpret_cast<const dtype*>(
                reinterpret_cast<const uint8_t*>(this) +
                S_map[c].second.first)[0]);
            return (d >> S_map[c].second.second) & S_map[c].first;
        }
    }

    void print() const {
        for (uint16_t i = 0; i < 6; i++) {
            std::cerr << int(revert(i)) << ": " << p_sum(i) << std::endl;
        }
    }
};


}  // namespace bbwt
