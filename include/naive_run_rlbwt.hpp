#pragma once

#include <cstdint>
#include <cstring>
#include <iostream>
#include <fstream>
#include <utility>

#include "naive_run_rlbwt_builder.hpp"

namespace bbwt {
template <class block_type_>
class naive_run_rlbwt {
   public:
    typedef block_type_ block_type;
    typedef block_type::alphabet_type alphabet_type;
    typedef naive_run_rlbwt_builder<naive_run_rlbwt> builder;

   private:
    uint64_t size_;
    uint64_t block_count_;
    uint64_t bytes_;
    uint64_t char_counts_[257];
    std::pair<uint64_t, uint64_t>* counts_;
    uint8_t* data_;

   public:
    naive_run_rlbwt(std::string path) : bytes_(sizeof(naive_run_rlbwt)) {
        std::fstream in_file;
        in_file.open(path, std::ios::binary | std::ios::in);
        if (in_file.fail()) {
            std::cerr << "Opening " << path << " failed!" << std::endl;
            exit(1);
        }
        bytes_ += alphabet_type::load_statics(in_file);
        bytes_ += block_type::load_statics(in_file);
        uint64_t data_bytes;
        in_file.read(reinterpret_cast<char*>(&data_bytes), sizeof(uint64_t));
        in_file.read(reinterpret_cast<char*>(&size_), sizeof(uint64_t));
        in_file.read(reinterpret_cast<char*>(&block_count_), sizeof(uint64_t));
        
        uint64_t count_bytes = sizeof(std::pair<uint64_t, uint64_t>) * block_count_;
        counts_ = (std::pair<uint64_t, uint64_t>*)std::malloc(count_bytes);
        in_file.read(reinterpret_cast<char*>(counts_), count_bytes);
        bytes_ += count_bytes;
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
            std::cerr << "opening " << prefix << "_data" << suffix << " failed!" << std::endl;
            exit(1);
        }
        data_ = (uint8_t*)std::malloc(data_bytes);
        in_file.read(reinterpret_cast<char*>(data_), data_bytes);
        bytes_ += data_bytes;
        in_file.close();
    }

    naive_run_rlbwt() = delete;
    naive_run_rlbwt(const naive_run_rlbwt&) = delete;
    naive_run_rlbwt& operator=(const naive_run_rlbwt&) = delete;

    naive_run_rlbwt(naive_run_rlbwt&& other) {
        size_ = std::exchange(other.size_, 0);
        bytes_ = std::exchange(other.bytes_, 0);
        block_count_ = std::exchange(other.block_count_, 0);
        data_ = std::exchange(other.data_, nullptr);
        counts_ = std::exchange(other.counts_, nullptr);
        std::memcpy(char_counts_, other.char_counts_, sizeof(uint64_t) * 257);
    }

    naive_run_rlbwt& operator=(naive_run_rlbwt&& other) {
        size_ = std::exchange(other.size_, 0);
        bytes_ = std::exchange(other.bytes_, 0);
        block_count_ = std::exchange(other.block_count_, 0);
        data_ = std::exchange(other.data_, nullptr);
        counts_ = std::exchange(other.counts_, nullptr);
        std::memcpy(char_counts_, other.char_counts_, sizeof(uint64_t) * 257);
        return *this;
    }

    ~naive_run_rlbwt() {
        if (counts_ != nullptr) {
            std::free(counts_);
        }
        if (data_ != nullptr) {
            std::free(data_);
        }
    }

    uint8_t at(uint64_t i) const {
        if (i >= size_) [[unlikely]] {
            return 0;
        }
        auto count = counts_[find(i)];
        i -= count.first;
        block_type* block = reinterpret_cast<block_type*>(data_ + count.second);
        return alphabet_type::revert(block->at(i));
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
            return char_counts_[c + 1] - char_counts_[c];
        }
        auto count = counts_[find(i)];
        i -= count.first;
        uint64_t res = reinterpret_cast<alphabet_type*>(data_ + count.second - alphabet_type::size())->p_sum(c);
        res += reinterpret_cast<block_type*>(data_ + count.second)->rank(c, i);
        return res;
    }

    uint8_t operator[](size_t i) const {
        return at(i);
    }

    uint64_t size() const { return size_; }
    uint64_t bytes() const { return bytes_; }

   private:
    uint64_t find(uint64_t i) const {
        uint64_t a = 0;
        uint64_t b = block_count_ - 1;
        while (a < b) {
            uint64_t m = (a + b + 1) / 2;
            if (counts_[m].first < i) {
                a = m;
            } else {
                b = m - 1;
            }
        }
        return a;
    }
};
}  // namespace bbwt
