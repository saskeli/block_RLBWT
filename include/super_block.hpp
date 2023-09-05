#pragma once

#include <cstring>
#include <cstdint>
#include <iostream>
#include <string>
#include <bitset>
#include <vector>

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

    uint8_t at(uint32_t i) const {
        uint32_t block_i = i / cap;
        //std::cerr << " block " << block_i << std::endl;
        const block_type* block =
            reinterpret_cast<const block_type*>(data() + offsets_[block_i]);
        return block->at(i % cap);
    }

    uint32_t rank(uint8_t c, uint32_t i) const {
        uint32_t block_i = i / cap;
        //std::cerr << "rank(" << int(c) << ", " << i << ")" << std::endl;
        __builtin_prefetch(data() + offsets_[block_i]);
        const alphabet_type* alpha = reinterpret_cast<const alphabet_type*>(
            data() + offsets_[block_i] - alphabet_type::size());
        uint32_t res = alpha->p_sum(c);
        //std::cerr << res << " from previous blocks " << std::endl;
        const block_type* block =
            reinterpret_cast<const block_type*>(data() + offsets_[block_i]);
        res += block->rank(c, i % cap);
        return res;
    }

    template <class vec>
    uint32_t i_rank(uint8_t& c, uint32_t i, vec& counts) const {
        uint32_t block_i = i / cap;
        __builtin_prefetch(data() + offsets_[block_i]);
        const alphabet_type* alpha = reinterpret_cast<const alphabet_type*>(
            data() + offsets_[block_i] - alphabet_type::size());
        const block_type* block = reinterpret_cast<const block_type*>(data() + offsets_[block_i]);
        uint32_t res = block->i_rank(c, i % cap, counts);
        res += alpha->p_sum(c);
        return res;
    }

    template <class vec>
    void c_rank(uint32_t loc, vec& counts) const {
        uint32_t block_i = loc / cap;
        __builtin_prefetch(data() + offsets_[block_i]);
        const alphabet_type* alpha = reinterpret_cast<const alphabet_type*>(
            data() + offsets_[block_i] - alphabet_type::size());
        const block_type* block = reinterpret_cast<const block_type*>(data() + offsets_[block_i]);
        for (uint16_t i = 0; i < alphabet_type::elemes(); ++i) {
            counts[i] += alpha->p_sum(i);
        }
        block->c_rank(loc % cap, counts);
    }

    template <class vec>
    void interval_statistics(uint32_t start, uint32_t end, vec& s_counts, vec& e_counts) {
        uint32_t s_block_i = start / cap;
        uint32_t e_block_i = end / cap;
        __builtin_prefetch(data() + offsets_[s_block_i]);
        const alphabet_type* alpha = reinterpret_cast<const alphabet_type*>(data() + offsets_[s_block_i] - alphabet_type::size());
        const block_type* block = reinterpret_cast<const block_type*>(data() + offsets_[s_block_i]);
        if (s_block_i == e_block_i) {
            for (uint16_t i = 0; i < alphabet_type::elems(); ++i) {
                s_counts[i] += alpha->p_sum(i);
                e_counts[i] += alpha->p_sum(i);
            }
            block->interval_statistics(start % cap, end % cap, s_counts, e_counts);
        } else {
            __builtin_prefetch(data() + offsets_[e_block_i]);
            for (uint16_t i = 0; i < alphabet_type::elems(); ++i) {
                s_counts[i] += alpha->p_sum(i);
            }
            block->c_rank(start % cap, s_counts);
            alpha = reinterpret_cast<const alphabet_type*>(data() + offsets_[e_block_i] - alphabet_type::size());
            for (uint16_t i = 0; i < alphabet_type::elems(); ++i) {
                e_counts[i] += alpha->p_sum(i);
            }
            block = reinterpret_cast<const block_type*>(data() + offsets_[e_block_i]);
            block->c_rank(end % cap, e_counts);
        }
    }

    uint32_t select(uint64_t x, uint8_t c, uint64_t size) const {
        uint32_t a = 0;
        uint32_t b = size / cap;
        while (a < b) {
            uint32_t m = (a + b + 1) / 2;
            uint32_t ps = reinterpret_cast<const alphabet_type*>(data() + offsets_[m] - alphabet_type::size())->p_sum(c);
            if (ps > x) {
                b = m - 1;
            } else {
                a = m;
            }
        }
        uint32_t res = a * cap;
        x -= reinterpret_cast<const alphabet_type*>(data() + offsets_[a] - alphabet_type::size())->p_sum(c);
        res += reinterpret_cast<const block_type*>(data() + offsets_[a])->select(x, c);
        return res;
    }

    template <class dtype>
    void print_block(uint32_t idx, uint32_t n_bytes) const {
        dtype* dp = reinterpret_cast<dtype*>(data() + offsets_[idx]);
        for (uint32_t i = 0; i < n_bytes; i++) {
            std::cerr << std::bitset<sizeof(dtype) * 8>(dp[i]) << std::endl;
        }
    }

    alphabet_type* get_psums(uint32_t i) const {
        return reinterpret_cast<alphabet_type*>(data() + offsets_[i] - alphabet_type::size());
    }

    void print(uint64_t s) const {
        bool done = false;
        for (uint32_t i = 0; i < blocks; i++) {
            std::cerr << "sub-block " << i << ": " << std::endl;
            uint64_t sb = cap * (i + 1);
            if (sb > s) {
                sb = s % cap;
                done = true;
            }
            sb = cap;
            uint32_t block_i = (sb - 1) / cap;
            const alphabet_type* alpha = reinterpret_cast<const alphabet_type*>(
                data() + offsets_[block_i] - alphabet_type::size());
            alpha->print();
            const block_type* block =
                reinterpret_cast<const block_type*>(data() + offsets_[block_i]);
            block->print(sb);
            if (done) break;
        }
    }

    static uint64_t write_statics(std::fstream&) {return 0; }
    static uint64_t load_statics(std::fstream&) {return 0; }

   private:
    const uint8_t* data() const {
        return reinterpret_cast<const uint8_t*>(this) + sizeof(super_block);
    }
};
}  // namespace bbwt
 