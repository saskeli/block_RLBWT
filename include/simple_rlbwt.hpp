#pragma once

#include <cstring>
#include <iostream>
#include <vector>

namespace bbwt {
template <class block_type, uint32_t block_size>
class simple_rlbwt {
  private:
    std::vector<block_type> data_;

  public:
    simple_rlbwt(std::vector<block_type>& blocks) {
        data_ = std::move(blocks);
    }

    uint8_t at(uint64_t location) {
        uint64_t block = location / block_size;
        uint64_t offset = location % block_size;
        return data_[block].at(offset);
    }
};
}  // namespace bbwt
