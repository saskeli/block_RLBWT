#pragma once

#include <iostream>
#include <vector>

namespace bbwt {
template <class bwt_type>
class block_rlbwt_builder {
  private:
    static const constexpr uint64_t SUPER_BLOCK_LIMIT = uint64_t(1) << 32;
    static const constexpr uint64_t BLOCKS_IN_SUPER_BLOCK = SUPER_BLOCK_LIMIT / bwt_type::cap;

    std::string prefix_;
    std::string suffix_;
    std::vector<bwt_type::alphabet_type> block_counts_;
    bwt_type::alphabet_type super_block_cumulative_;
    std::vector<uint64_t> block_offsets_;
    bwt_type::alphabet_type<bwt_type::cap> block_cumulative_;
    uint64_t super_block_bytes_;
    uint64_t super_block_size_;
    uint64_t elems_;
    uint8_t* current_super_block_;
    uint8_t** scratch_;
    bwt_type::block_type current_block_;
    uint32_t block_elems_;
    uint32_t block_bytes_;
    uint32_t blocks_in_super_block_;

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
        block_counts_ = std::vector<bwt_type::alphabet_type>();
        super_block_cumulative_ = bwt_type::alphabet_type();
        block_counts_.push_back(super_block_cumulative_);
        block_offsets_ = std::vector<uint64_t>();
        block_cumulative_ = bwt_type::alphabet_type<bwt_type::cap>();
        super_block_bytes_ = sizeof(bwt_type::alphabet_type);
        super_block_size_ = BLOCKS_IN_SUPER_BLOCK * (bwt_type::block_type::min_size() + sizeof(bwt_type::alphabet_type<bwt_type::cap>));
        elems_ = 0;
        current_super_block_ = (uint64_t*)calloc(super_block_size_, 1);
        scratch_ = (uint8_t**)malloc(bwt_type::block_type::scratch_blocks() * sizeof(uint8_t*));
        for (size_t i = 0; i < bwt_type::block_type::scratch_blocks(); i++) {
            scratch_[i] = (uint8_t*)calloc(bwt_type::block_type::scratch_size(i), 1);
        }
        current_block_ = bwt_type::block_type();
        block_elems_ = 0;
        block_bytes_ = sizeof(block_type);
        blocks_in_super_block_ = 0;
    }

    void append(uint8_t head, uint32_t length) {
        if (length + block_elems_ < cap) [[likely]] {
            block_cumulative_.add(head, length);
            super_block_cumulative_.add(head, length);
            block_bytes_ += current_block_.append(head, length, scratch_);
            block_elems_ += length;
            elems_ += length;
        } else if (length + block_elems_ == cap) [[unlikely]] {
            block_cumulative_.add(head, length);
            super_block_cumulative_.add(head, length);
            block_bytes_ += current_block_.append(head, length, scratch_);
            elems_ += length;
            commit();
        } else {
            uint32_t fill = bwt_type::cap - block_elems_;
            block_cumulative_.add(head, fill);
            super_block_cumulative_.add(head, fill);
            block_bytes_ += current_block_.append(head, fill, scratch_);
            elems_ += fill;
            commit();
            return append(head, length - fill);
        }
    }

    void finalize() {
        commit(true);
        write_super_block();
        write_root();
    }
  private:
    void write_super_block() {
        std::FILE* out = std::fopen(prefix_ + "_" + block_counts_.size() + suffix_, "wb");
        uint64_t n_blocks = 1;
        std::fwrite(&n_blocks, sizeof(uint64_t), 1, out);
        std::fwrite(&super_block_bytes_, sizeof(uint64_t), 1, out);
        std::fwrite(&current_super_block_, sizeof(uint8_t), super_block_bytes_, out);
        block_counts_.push_back(super_block_cumulative_);
    }

    void commit(bool last_block = false) {
        if (super_block_bytes_ + block_bytes_ > super_block_size_) {
            uint64_t new_size = super_block_bytes_ + block_bytes_;
            if (!last_block) {
                new_size += (BLOCKS_IN_SUPER_BLOCK - blocks_in_super_block_ - 1) * (bwt_type::block_type::min_size() + sizeof(bwt_type::alphabet_type<bwt_type::cap>))
            }
            current_super_block_ = (uint8_t*)realloc(current_super_block_, new_size);
            std::memset(current_super_block_ + super_block_size_, 0, new_size - super_block_size_);
            super_block_size_ = new_size;
        }
        std::memcpy(current_super_block_ + super_block_bytes_, &current_block_, sizeof(bwt_type::block_type));
        bwt_type::block_type* b = reinterpret_cast<bwt_type::block_type*>(current_super_block_ + super_block_bytes_);
        b->commit(scratch_);
        block_offsets_.push_back(super_block_bytes_);
        super_block_bytes_ += block_bytes_;
        for (size_t i = 0; i < bwt_type::block_type::scratch_blocks(); i++) {
            std::memset(scratch_[i], 0, bwt_type::block_type::scratch_size(i));
        }
        std::memset(&current_block_, 0, sizeof(bwt_type::block_type));
        block_elems_ = 0;
        block_bytes_ = sizeof(bwt_type::block_type);
        blocks_in_super_block_++;
        if (!last_block && blocks_in_super_block_ < BLOCKS_IN_SUPER_BLOCK) [[likely]] {
            std::memcpy(current_super_block_ + super_block_bytes_, &block_cumulative_, sizeof(bwt_type::alphabet_type<bwt_type::cap>));
            super_block_bytes_ += sizeof(bwt_type::alphabet_type<bwt_type::cap>);
        } else {
            write_super_block();
        }
    }
};    
} // namespace bbwt

