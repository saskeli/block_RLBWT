#pragma once

#include <cstring>
#include <iostream>
#include <utility>
#include <vector>

namespace bbwt {
template <class super_block_type_, class alphabet_type_>
class block_rlbwt {
   public:
    typedef super_block_type_ super_block_type;
    typedef alphabet_type_ alphabet_type;
   private:
    static const constexpr uint64_t SUPER_BLOCK_ELEMS = uint64_t(1) << 32;
    uint64_t size_;
    uint64_t block_count_;
    alphabet_type* p_sums_;
    std::vector<super_block_type*> s_blocks_;

   public:
    static const constexpr uint32_t cap = super_block_type::cap;

    typedef super_block_type::block_type block_type;
    typedef block_type::alphabet_type block_alphabet_type;

    block_rlbwt(std::string path) {
        std::FILE* in_file = std::fopen(path.c_str(), "rb");

        uint64_t data_bytes, blocks;
        std::fread(&data_bytes, sizeof(uint64_t), 1, in_file);
        std::fread(&size_, sizeof(uint64_t), 1, in_file);
        std::fread(block_count_, sizeof(uint64_t), 1, in_file);
        p_sums_ = (alphabet_type*)std::malloc(data_bytes);
        std::fread(p_sums_, sizeof(uint8_t), data_bytes, in_file);

        std::string prefix;
        std::string suffix;
        size_t loc = path.find_last_of('.');
        if (loc == std::string::npos) {
            prefix = path;
            suffix = "";
        } else {
            prefix = path.substr(0, loc);
            suffix = path.substr(loc);
        }

        for (uint64_t i = 0; i < blocks; i++) {
            s_blocks_.push_back(read_super_block(prefix + "_" + std::to_string(i) + suffix));
        }
        std::fclose(in_file);
    }

    block_rlbwt() = delete;
    block_rlbwt(const block_rlbwt& other) = delete;
    block_rlbwt& operator=(const block_rlbwt& other) = delete;

    block_rlbwt(block_rlbwt&& other) {
        size_ = std::exchange(other.size_, 0);
        block_count_ = std::exchange(other.block_count_, 0);
        p_sums_ = std::exchange(other.p_sums_, nullptr);
        s_blocks_ =
            std::exchange(other.s_blocks_, std::vector<super_block_type*>());
    }

    block_rlbwt& operator=(block_rlbwt&& other) {
        size_ = std::exchange(other.size_, 0);
        block_count_ = std::exchange(other.block_count_, 0);
        p_sums_ = std::exchange(other.p_sums_, nullptr);
        s_blocks_ =
            std::exchange(other.s_blocks_, std::vector<super_block_type*>());
        return *this;
    }

    ~block_rlbwt() {
        for (uint64_t i = 0; i < block_count_; i++) {
            std::free(s_blocks_[i]);
        }
        std::free(p_sums_);
    }

    uint8_t at(uint64_t i) {
        if (i >= size_) [[unlikely]] {
            return 0;
        }
        uint64_t s_block_i = i / SUPER_BLOCK_ELEMS;
        return s_blocks_[s_block_i].at(i % SUPER_BLOCK_ELEMS);
    }

    uint64_t rank(uint8_t c, uint64_t i) {
        if (i >= size_) [[unlikely]] {
            return p_sums_[block_count_].at(c);
        }
        uint64_t s_block_i = i / SUPER_BLOCK_ELEMS;
        uint64_t res = p_sums_[s_block_i].at(c);
        res += s_blocks_[s_block_i].rank(c, i % SUPER_BLOCK_ELEMS);
        return res;
    }

   private:
    super_block_type* read_super_block(std::string& path) {
        std::FILE* in_file = std::fopen(path.c_str(), "br");
        uint64_t in_bytes;
        std::fread(&in_bytes, sizeof(uint64_t), 1, in_file);
        uint8_t* data = (uint8_t*)std::malloc(in_bytes);
        std::fread(data, sizeof(uint8_t), in_bytes, in_file);
        std::fclose(in_file);
        return reinterpret_cast<super_block_type*>(data);
    }
};
}  // namespace bbwt
