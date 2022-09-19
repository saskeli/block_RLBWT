#pragma once

#include <iostream>
#include <vector>

namespace bbwt {
template <class block_type, uint32_t cap, class bwt_type>
class block_rlbwt_builder {
  private:
    std::vector<uint8_t> heads_;
    std::vector<uint8_t> runs_;
    std::vector<block_type> blocks_;
    uint32_t block_elems_;
  public:
    block_rlbwt_builder() : heads_(), runs_(), blocks_(), block_elems_(0) {}

    void append(uint8_t head, uint32_t length) {
        heads_.push_back(head);
        if (block_elems_ + length > cap) [[unlikely]] {
            uint32_t overflow = block_elems_ + length - cap;
            write(cap - block_elems_);
            commit();
            heads_.push_back(head);
            write(overflow);
            block_elems_ = overflow;
        } else {
            write(length);
            block_elems_ += length;
        }
        if (block_elems_ == cap) [[unlikely]] {
            commit();
        }
    }

    bwt_type compile() {
        commit();
        return bwt_type(blocks_);
    }
  private:
    void write(uint32_t length) {
        while (length) {
            uint8_t b = length & 0b01111111;
            length >>= 7;
            uint8_t c = length ? 0b0 : 0b10000000;
            runs_.push_back(b | c);
        }
    }

    void commit() {
        blocks_.push_back({heads_, runs_});
        heads_.clear();
        runs_.clear();
        block_elems_ = 0;
    }
};    
} // namespace bbwt

