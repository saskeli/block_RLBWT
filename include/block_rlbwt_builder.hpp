#pragma once

#include <iostream>
#include <vector>

namespace bbwt {
template <class partial_block_type, class alphabet_type, uint32_t cap, class bwt_type>
class block_rlbwt_builder {
  private:
    typedef partial_block_type<alphabet_type<uint32_t>> block_type;
    static const constexpr uint64_t BLOCKS_IN_SUPER_BLOCK = (uint64_t(1) << 32) / cap;

    std::string prefix_;
    std::string suffix_;
    std::vector<alphabet_type<uint64_t>> block_counts_;
    alphabet_type<uint64_t> super_block_cumulative_;
    uint64_t super_block_count_;
    uint64_t super_block_bytes_;
    uint64_t super_block_size_;
    uint64_t super_block_elems_;
    uint8_t* current_super_block_;
    uint8_t** scratch_;
    block_type current_block_;
    uint32_t block_elems_;
    uint32_t block_bytes_;

  public:
    block_rlbwt_builder(std::string out_file) {
        size_t loc = out_file.find_last_of('.');
        if (loc == std::string::npos) {
            prefix_ = out_file;
            suffix_ = "";
        } else {
            prefix_ = out_file.substr(0, loc);
            suffix_ = out_file.substr(loc);
        }
        block_counts_ = std::vector<alphabet_type<uint64_t>>();
        super_block_count_ = alphabet_type<uint64_t>();
        super_block_count_ = 0;
        super_block_bytes_ = sizeof(alphabet_type);
        super_block_size_ = BLOCKS_IN_SUPER_BLOCK * (block_type::min_size() + sizeof(alphabet_type));
        super_block_elems_ = 0;
        current_super_block_ = (uint64_t*)calloc(super_block_size_, 1);
        scratch_ = (uint8_t**)malloc(block_type::scratch_blocks() * sizeof(uint8_t*));
        for (size_t i = 0; i < block_type::scratch_blocks(); i++) {
            scratch_[i] = (uint8_t*)malloc(block_type::scratch_size(i));
        }
        current_block_ = block_type();
        block_elems_ = 0;
        block_bytes_ = sizeof(block_type);
    }

    void append(uint8_t head, uint32_t length) {
        if (length + block_elems_ < cap) [[likely]] {
            alphabet()->add(head, length);
            block_bytes_ += current_block_->append(head, length, scratch_);
        } else if (length + block_elems_ == cap) [[unlikely]] {
            alphabet()->add(head, length);
            block_bytes_ += current_block_->append(head, length, scratch_);
            commit();
        } else {
            uint32_t fill = cap - block_elems_;
            alphabet()->add(head, fill);
            block_bytes_ += current_block_->append(head, fill, scratch_);
            commit();
            return append(head, length - fill);
        }
    }

    void finalize() {
        commit();
    }
  private:

    void commit() {
        if ()
    }

    alphabet_type* alphabet() {
        return reinterpret_cast<alphabet_type*>(current_super_block_ + (super_block_bytes_ - sizeof(alphabet_type)));
    }
};    
} // namespace bbwt

