#pragma once

#include <cstring>
#include <iostream>
#include <utility>
#include <vector>

namespace bbwt {
class byte_block {
   private:
    uint8_t* data_;
    uint32_t head_offset_;
    uint32_t size_;

   public:
    byte_block(std::vector<uint8_t>& heads, std::vector<uint8_t>& runs) {
        size_ = heads.size() + runs.size();
        data_ = new uint8_t[size_];
        head_offset_ = heads.size();
        std::memcpy(data_, heads.data(), heads.size());
        std::memcpy(data_ + heads.size(), runs.data(), runs.size());
    }

    byte_block() : data_(nullptr), head_offset_(0), size_(0) {}

    byte_block(const byte_block& other) = delete;

    byte_block(byte_block&& other)
        : data_(std::exchange(other.data_, nullptr)),
          head_offset_(std::exchange(other.head_offset_, 0)),
          size_(std::exchange(other.size_, 0)) {}

    ~byte_block() {
        delete[] data_; 
    }

    byte_block& operator=(byte_block&& other) {
        data_ = std::exchange(other.data_, nullptr);
        head_offset_ = std::exchange(other.head_offset_, 0);
        size_ = std::exchange(other.size_, 0);
        return *this;
    }

    byte_block& operator=(const byte_block&) =delete;

    uint8_t at(uint32_t location) {
        uint32_t head_i = 0;
        uint32_t run_i = head_offset_;
        while (true) {
            uint8_t c = data_[head_i++];
            uint32_t rl = read(run_i);
            if (location >= rl) {
                location -= rl;
            } else {
                return c;
            }
        }
    }

   private:
    uint32_t read(uint32_t& i) {
        uint32_t res = 0;
        uint32_t offset = 0;
        while ((data_[i] & 0b10000000) == 0) [[unlikely]] {
                res |= data_[i++] << offset;
                offset += 7;
            }
        return res | ((data_[i++] & 0b01111111) << offset);
    }
};
}  // namespace bbwt
