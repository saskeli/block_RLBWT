#include <cstring>
#include <iostream>
#include <string>
#include <bitset>

namespace bbwt {
template <class block_type_>
class super_block {
   public:
    typedef block_type_ block_type;
    typedef typename block_type::alphabet_type alphabet_type;
    static_assert((uint64_t(1) << 32) % block_type::cap == 0);
    static const constexpr uint64_t blocks =
        (uint64_t(1) << 32) / block_type::cap;
    static const constexpr uint32_t cap = block_type::cap;
   private:
    uint64_t offsets_[blocks];
   public:
    

    super_block() = delete;
    super_block(const super_block& other) = delete;
    super_block(super_block&& other) = delete;
    super_block& operator=(const super_block& other) = delete;
    super_block& operator=(super_block&& other) = delete;

    uint8_t at(uint32_t i) {
        uint32_t block_i = i / cap;
        //std::cerr << " block " << block_i << std::endl;
        block_type* block =
            reinterpret_cast<block_type*>(data() + offsets_[block_i]);
        return block->at(i % cap);
    }

    uint32_t rank(uint8_t c, uint32_t i) {
        uint32_t block_i = i / cap;
        __builtin_prefetch(data() + offsets_[block_i]);
        block_type* block =
            reinterpret_cast<block_type*>(data() + offsets_[block_i]);
        alphabet_type* alpha = reinterpret_cast<alphabet_type*>(
            data() + offsets_[block_i] - sizeof(alphabet_type));
        uint32_t res = alpha->p_sum(c);
        res += block->rank(c, i % cap);
        return res;
    }

    template <class dtype>
    void print_block(uint32_t idx, uint32_t n_bytes) {
        dtype* dp = reinterpret_cast<dtype*>(data() + offsets_[idx]);
        for (uint32_t i = 0; i < n_bytes; i++) {
            std::cerr << std::bitset<sizeof(dtype) * 8>(dp[i]) << std::endl;
        }
    }

    alphabet_type* get_psums(uint32_t i) {
        return reinterpret_cast<alphabet_type*>(data() + offsets_[i] - sizeof(alphabet_type));
    }

   private:
    uint8_t* data() {
        return reinterpret_cast<uint8_t*>(this) + sizeof(super_block);
    }
};
}  // namespace bbwt
