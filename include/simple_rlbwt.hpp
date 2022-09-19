#pragma once

#include <iostream>
#include <vector>
#include <cstring>

namespace bbwt {
template <class block_type, uint32_t block_size>
class simple_rlbwt {
  private:
    block_type* data_;

  public:
    simple_rlbwt(std::vector<block_type>& blocks) {
        data_ = (block_type*)malloc(sizeof(block_type) * blocks.size());
        std::move(blocks.data(), blocks.data() + blocks.size(), data_);
    }

    uint8_t at(uint64_t location) {
        uint64_t block = location / block_size;
        uint64_t offset = location % block_size;
        return data_[block].at(offset);
    }
};
} // namespace bbwt
