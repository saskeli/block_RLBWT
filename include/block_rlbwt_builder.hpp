#pragma once

#include <fstream>
#include <iostream>
#include <vector>
#include <bitset>
#include <cmath>
#include <random>

namespace bbwt {
template <class bwt_type>
class block_rlbwt_builder {
   private:
    static const constexpr uint64_t BLOCKS_IN_SUPER_BLOCK =
        bwt_type::super_block_type::blocks;

    typedef typename bwt_type::alphabet_type alphabet_type;
    typedef typename bwt_type::block_alphabet_type block_alphabet_type;
    typedef typename bwt_type::block_type block_type;

    uint64_t char_counts_[257];
    uint32_t run_count_;
    uint32_t dense_blocks_;
    std::string prefix_;
    std::string suffix_;
    std::vector<alphabet_type> block_counts_;
    std::vector<bool> block_reprs_;
    alphabet_type super_block_cumulative_;
    std::vector<uint64_t> block_offsets_;
    block_alphabet_type block_cumulative_;
    uint64_t super_block_bytes_;
    uint64_t super_block_size_;
    uint64_t elems_;
    uint8_t* current_super_block_;
    uint8_t** scratch_;
    block_type current_block_;
    uint32_t block_elems_;
    uint32_t block_bytes_;
    uint32_t blocks_in_super_block_;
    std::fstream out_;

   public:
    block_rlbwt_builder(std::string out_file)
        : char_counts_(),
          run_count_(0),
          dense_blocks_(0),
          block_counts_(),
          block_reprs_(),
          super_block_cumulative_(),
          block_offsets_(),
          block_cumulative_(),
          super_block_bytes_(sizeof(block_alphabet_type)),
          super_block_size_(
              BLOCKS_IN_SUPER_BLOCK *
              (block_type::min_size + sizeof(block_alphabet_type))),
          elems_(0),
          current_block_(),
          block_elems_(0),
          block_bytes_(0),
          blocks_in_super_block_(0) {
        size_t loc = out_file.find_last_of('.');
        if (loc == std::string::npos) {
            prefix_ = out_file;
            suffix_ = "";
        } else {
            prefix_ = out_file.substr(0, loc);
            suffix_ = out_file.substr(loc);
        }
        out_.open(prefix_ + "_data" + suffix_, std::ios::binary | std::ios::out);
        block_counts_.push_back(super_block_cumulative_);
        current_super_block_ = (uint8_t*)calloc(super_block_size_, 1);
        scratch_ =
            (uint8_t**)malloc(block_type::scratch_blocks * sizeof(uint8_t*));
        for (size_t i = 0; i < block_type::scratch_blocks; i++) {
            scratch_[i] = (uint8_t*)calloc(block_type::scratch_size(i), 1);
        }
    }

    void append(uint8_t head, uint32_t length) {
        char_counts_[head] += length;
        head = alphabet_type::convert(head);
        while (length) {
            run_count_++;
            if (length + block_elems_ < bwt_type::cap) {
                block_cumulative_.add(head, length);
                super_block_cumulative_.add(head, length);
                block_bytes_ = current_block_.append(head, length, scratch_);
                block_elems_ += length;
                elems_ += length;
                return;
            } else if (length + block_elems_ == bwt_type::cap) [[unlikely]] {
                block_cumulative_.add(head, length);
                super_block_cumulative_.add(head, length);
                block_bytes_ = current_block_.append(head, length, scratch_);
                elems_ += length;
                commit();
                return;
            } else {
                uint32_t fill = bwt_type::cap - block_elems_;
                block_cumulative_.add(head, fill);
                super_block_cumulative_.add(head, fill);
                block_bytes_ = current_block_.append(head, fill, scratch_);
                elems_ += fill;
                commit();
                length -= fill;
            }
        }
    }

    void finalize() {
        if (block_elems_) {
            commit(true);
        }
        if (blocks_in_super_block_) {
            write_super_block();
        }
        out_.close();
        uint64_t p_v = 0;
        for (size_t i = 0; i < 257; i++) {
            uint64_t tmp = char_counts_[i];
            char_counts_[i] = p_v;
            p_v += tmp;
        }
        write_root();
    }

    void gen_queries(std::ostream& out, uint32_t n_queries) {
        uint64_t char_count = char_counts_[256];
        std::vector<uint8_t> chars;
        for (uint16_t i = 0; i < 256; i++) {
            if (char_counts_[i + 1] > char_counts_[i]) {
                chars.push_back(uint8_t(i));
            }
        }

        std::mt19937 mt;
        std::uniform_int_distribution<unsigned long long> i_gen(0, char_count - 1);
        std::uniform_int_distribution<uint8_t> c_gen(0, chars.size() - 1);

        for (uint32_t i = 0; i < n_queries; i++) {
            uint64_t idx = i_gen(mt);
            uint8_t c = chars[c_gen(mt)];
            bool dense = block_reprs_[idx / bwt_type::cap];
            out.write(reinterpret_cast<char*>(&idx), sizeof(uint64_t));
            out.write(reinterpret_cast<char*>(&c), sizeof(uint8_t));
            out.write(reinterpret_cast<char*>(&dense), sizeof(bool));
        }
    }

   private:
    void write_super_block() {
        std::cerr << "Writing super block" << std::endl;
        std::cerr << "Total of " << elems_ << " elements read" << std::endl;
        uint64_t tot_bytes =
            sizeof(uint64_t) +
            sizeof(uint64_t) * bwt_type::super_block_type::blocks +
            super_block_bytes_;
        std::cerr << sizeof(uint64_t) << " + "
                  << sizeof(uint64_t) * bwt_type::super_block_type::blocks
                  << " + " << super_block_bytes_ << " = " << tot_bytes
                  << " bytes = " << double(tot_bytes) / (uint64_t(1) << 30)
                  << " GiB" << std::endl;

        uint64_t file_bytes =
            super_block_bytes_ +
            sizeof(uint64_t) * bwt_type::super_block_type::blocks;
        out_.write(reinterpret_cast<char*>(&file_bytes), sizeof(uint64_t));
        out_.write(reinterpret_cast<char*>(block_offsets_.data()),
                  sizeof(uint64_t) * block_offsets_.size());
        for (uint64_t i = block_offsets_.size();
             i < bwt_type::super_block_type::blocks; i++) {
            uint64_t zero = 0;
            out_.write(reinterpret_cast<char*>(&zero), sizeof(uint64_t));
        }
        out_.write(reinterpret_cast<char*>(current_super_block_),
                  super_block_bytes_);
        block_counts_.push_back(super_block_cumulative_);

        block_cumulative_.clear();
        block_offsets_.clear();
        std::memset(current_super_block_, 0,
                    sizeof(uint8_t) * super_block_size_);
        super_block_bytes_ = sizeof(block_alphabet_type);
        blocks_in_super_block_ = 0;
    }

    void commit(bool last_block = false) {
        if (run_count_ >= 4 * std::log2(bwt_type::cap)) {
            block_reprs_.push_back(true);
            dense_blocks_++;
        } else {
            block_reprs_.push_back(false);
        }
        run_count_ = 0;
        if (super_block_bytes_ + block_bytes_ + block_alphabet_type::size() > super_block_size_) {
            uint64_t new_size = super_block_bytes_ + block_bytes_;
            if (!last_block) {
                new_size +=
                    (BLOCKS_IN_SUPER_BLOCK - blocks_in_super_block_ - 1) *
                    (block_type::min_size + sizeof(block_alphabet_type));
            }
            current_super_block_ =
                (uint8_t*)realloc(current_super_block_, new_size);
            //std::memset(current_super_block_ + super_block_size_, 0,
            //            new_size - super_block_size_);
            super_block_size_ = new_size;
        }
        
        block_offsets_.push_back(super_block_bytes_);
        if constexpr (block_type::has_members) {
            std::memcpy(current_super_block_ + super_block_bytes_, &current_block_,
                    sizeof(block_type));
        }
        block_type* b = reinterpret_cast<block_type*>(current_super_block_ +
                                                      super_block_bytes_);
        super_block_bytes_ += b->commit(scratch_);
        for (size_t i = 0; i < block_type::scratch_blocks; i++) {
            std::memset(scratch_[i], 0, block_type::scratch_size(i));
        }
        current_block_.clear();
        block_elems_ = 0;
        block_bytes_ = 0;
        blocks_in_super_block_++;
        if (!last_block && blocks_in_super_block_ < BLOCKS_IN_SUPER_BLOCK)
            [[likely]] {
            std::memcpy(current_super_block_ + super_block_bytes_,
                        &block_cumulative_, block_alphabet_type::size());
            super_block_bytes_ += block_alphabet_type::size();
        } else {
            write_super_block();
        }
    }

    void write_root() {
        uint64_t n_blocks = block_counts_.size() - 1;

        std::cerr << "Writing \"root\" to file\n"
                  << " Seen " << n_blocks << " super blocks\n"
                  << " containing a total of " << elems_ << " elements\n"
                  << " " << dense_blocks_ << " dense blocks out of "
                  << block_reprs_.size() << " total blocks ("
                  << 100.0 * dense_blocks_ / block_reprs_.size() << " %)"
                  << std::endl;

        std::fstream out;
        out.open(prefix_ + suffix_, std::ios::binary | std::ios::out);
        alphabet_type::write_statics(out);
        block_alphabet_type::write_statics(out);
        bwt_type::super_block_type::write_statics(out);
        block_type::write_statics(out);
        uint64_t bytes = sizeof(alphabet_type) * block_counts_.size();
        out.write(reinterpret_cast<char*>(&bytes), sizeof(uint64_t));
        out.write(reinterpret_cast<char*>(&elems_), sizeof(uint64_t));
        out.write(reinterpret_cast<char*>(&n_blocks), sizeof(uint64_t));
        out.write(reinterpret_cast<char*>(block_counts_.data()), bytes);
        out.write(reinterpret_cast<char*>(char_counts_), sizeof(uint64_t) * 257);
        out.close();

        std::cerr << sizeof(uint64_t) << " + " << sizeof(uint64_t) << " + "
                  << sizeof(uint64_t) << " + " << bytes << " = "
                  << bytes + 3 * sizeof(uint64_t) << " bytes = "
                  << double(bytes + 3 * sizeof(uint64_t)) / (uint64_t(1) << 20)
                  << " MiB" << std::endl;
    }
};
}  // namespace bbwt
