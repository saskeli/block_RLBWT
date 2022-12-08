#pragma once

#include <cassert>
#include <cstring>
#include <iostream>
#include <fstream>
#include <utility>
#include <vector>

#include "block_rlbwt_builder.hpp"

namespace bbwt {
template <class super_block_type_, class alphabet_type_>
class block_rlbwt {
   public:
    typedef super_block_type_ super_block_type;
    typedef alphabet_type_ alphabet_type;
    typedef block_rlbwt_builder<block_rlbwt> builder;

   private:
    static const constexpr uint64_t SUPER_BLOCK_ELEMS = uint64_t(1) << 32;
    uint64_t size_;
    uint64_t block_count_;
    uint64_t bytes_;
    uint64_t char_counts_[257];
    uint8_t* p_sums_;
    std::vector<super_block_type*> s_blocks_;

   public:
    static const constexpr uint32_t cap = super_block_type::cap;

    static_assert(SUPER_BLOCK_ELEMS % cap == 0);

    typedef typename super_block_type_::block_type block_type;
    typedef typename block_type::alphabet_type block_alphabet_type;

    block_rlbwt(std::string path) : bytes_(sizeof(block_rlbwt)) {
        std::fstream in_file;
        in_file.open(path, std::ios::binary | std::ios::in);
        if (in_file.fail()) {
            std::cerr << " -> Failed" << std::endl;
            exit(1);
        }
        bytes_ += alphabet_type::load_statics(in_file);
        bytes_ += block_alphabet_type::load_statics(in_file);
        bytes_ += super_block_type::load_statics(in_file);
        bytes_ += block_type::load_statics(in_file);
        uint64_t data_bytes;
        in_file.read(reinterpret_cast<char*>(&data_bytes), sizeof(uint64_t));
        in_file.read(reinterpret_cast<char*>(&size_), sizeof(uint64_t));
        in_file.read(reinterpret_cast<char*>(&block_count_), sizeof(uint64_t));
#ifdef VERB
        std::cerr << data_bytes << " bytes of data\n"
                  << size_ << " logical elements\n"
                  << "in " << block_count_ << " super blocks" << std::endl;
#endif
        p_sums_ = (uint8_t*)std::malloc(data_bytes);
        in_file.read(reinterpret_cast<char*>(p_sums_), data_bytes);
        bytes_ += data_bytes;
        in_file.read(reinterpret_cast<char*>(char_counts_), sizeof(uint64_t) * 257);
        in_file.close();

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
        in_file.open(prefix + "_data" + suffix, std::ios::binary | std::ios::in);
        if (in_file.fail()) {
            std::cerr << " -> Failed" << std::endl;
            exit(1);
        }
        for (uint64_t i = 1; i <= block_count_; i++) {
            s_blocks_.push_back(read_super_block(in_file));
        }
        bytes_ += s_blocks_.size() * sizeof(super_block_type*);
        in_file.close();
    }

    block_rlbwt() = delete;
    block_rlbwt(const block_rlbwt& other) = delete;
    block_rlbwt& operator=(const block_rlbwt& other) = delete;

    block_rlbwt(block_rlbwt&& other) {
        size_ = std::exchange(other.size_, 0);
        bytes_ = std::exchange(other.bytes_, 0);
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

    uint8_t at(uint64_t i) const {
        if (i >= size_) [[unlikely]] {
            return 0;
        }
        uint64_t s_block_i = i / SUPER_BLOCK_ELEMS;
        return alphabet_type::revert(s_blocks_[s_block_i]->at(i % SUPER_BLOCK_ELEMS));
    }

    uint64_t count(const std::string& pattern) const {
        uint8_t c = pattern[pattern.size() - 1];
        uint64_t a = char_counts_[c];
        uint64_t b = char_counts_[uint16_t(c) + 1];
        uint64_t ret = b  - a;
        for (size_t i = pattern.size() - 2; i < pattern.size() && ret > 0; i--) {
            c = pattern[i];
            a = rank(a, c);
            b = rank(b, c);
            ret = b - a;
            a += char_counts_[c];
            b += char_counts_[c];
        }
        return ret;
    }

    uint64_t LF(const uint64_t& i) const {
        uint8_t c = at(i);
        return char_counts_[c] + rank(i, c);
    }

    uint64_t rank(uint64_t i, uint8_t c) const {
        c = alphabet_type::convert(c);
        if (i >= size_) [[unlikely]] {
            return reinterpret_cast<alphabet_type*>(p_sums_ + alphabet_type::size() * block_count_)->p_sum(c);
        }
        uint64_t s_block_i = i / SUPER_BLOCK_ELEMS;
        uint64_t res = reinterpret_cast<alphabet_type*>(p_sums_ + alphabet_type::size() * s_block_i)->p_sum(c);
        res += s_blocks_[s_block_i]->rank(c, i % SUPER_BLOCK_ELEMS);
        return res;
    }

    uint8_t operator[](size_t i) const {
        return at(i);
    }

    uint64_t size() const { return size_; }
    uint64_t bytes() const { return bytes_; }

    void print() const {
        for (uint32_t i = 0; i < block_count_; i++) {
            uint64_t s = (i + 1) * SUPER_BLOCK_ELEMS;
            s = s > size_ ? size_ % SUPER_BLOCK_ELEMS : SUPER_BLOCK_ELEMS;
            std::cerr << "S block " << i << ":" << std::endl;
            reinterpret_cast<alphabet_type*>(p_sums_ * alphabet_type::size() * i)->print();
            s_blocks_[i]->print(s);
        }
    }

   private:
    super_block_type* read_super_block(std::fstream& in_file) {
        uint64_t in_bytes = 0;
        in_file.read(reinterpret_cast<char*>(&in_bytes), sizeof(uint64_t));
        in_bytes += block_type::padding_bytes;
        uint8_t* data = (uint8_t*)std::malloc(in_bytes);
        if constexpr (block_type::padding_bytes) {
            std::memset(data + (in_bytes - block_type::padding_bytes), 0, block_type::padding_bytes);
        }
        in_file.read(reinterpret_cast<char*>(data), in_bytes);
        bytes_ += in_bytes;
        return reinterpret_cast<super_block_type*>(data);
    }
};
}  // namespace bbwt
