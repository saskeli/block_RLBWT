#pragma once

#include <fstream>
#include <iostream>
#include <vector>
#include <cstdint>
#include <random>

namespace bbwt {
template <class bwt_type>
class naive_run_rlbwt_builder {
   private:
    typedef typename bwt_type::alphabet_type alphabet_type;
    typedef typename bwt_type::block_type block_type;

    uint64_t char_counts_[257];
    uint32_t run_count_;
    std::string prefix_;
    std::string suffix_;
    alphabet_type cumulative_;
    std::vector<std::pair<uint64_t, uint64_t>> block_offsets_;
    uint64_t elems_;
    uint8_t** scratch_;
    block_type current_block_;
    uint64_t block_elems_;
    uint64_t offset_;
    std::fstream out_;

   public:
    naive_run_rlbwt_builder(std::string out_file)
        : char_counts_(),
          run_count_(0),
          cumulative_(),
          block_offsets_(),
          elems_(0),
          current_block_(),
          block_elems_(0),
          offset_(alphabet_type::size()) {
        size_t loc = out_file.find_last_of('.');
        if (loc == std::string::npos) {
            prefix_ = out_file;
            suffix_ = "";
        } else {
            prefix_ = out_file.substr(0, loc);
            suffix_ = out_file.substr(loc);
        }
        out_.open(prefix_ + "_data" + suffix_, std::ios::binary | std::ios::out);
        scratch_ =
            (uint8_t**)malloc(block_type::scratch_blocks * sizeof(uint8_t*));
        for (size_t i = 0; i < block_type::scratch_blocks; i++) {
            scratch_[i] = (uint8_t*)calloc(block_type::scratch_size(i), 1);
        }
        out_.write(reinterpret_cast<char*>(&cumulative_), alphabet_type::size());
    }

    void append(uint8_t head, uint32_t length) {
        char_counts_[head] += length;
        head = alphabet_type::convert(head);
        cumulative_.add(head, length);
        current_block_.append(head, length, scratch_);
        block_elems_ += length;
        ++run_count_;
        if (run_count_ >= block_type::cap) {
            commit();
        }
    }

    void finalize() {
        if (block_elems_) {
            commit(true);
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
            bool dense = false;
            out.write(reinterpret_cast<char*>(&idx), sizeof(uint64_t));
            out.write(reinterpret_cast<char*>(&c), sizeof(uint8_t));
            out.write(reinterpret_cast<char*>(&dense), sizeof(bool));
        }
    }

   private:
    void commit(bool last_block = false) {
        run_count_ = 0;
        block_offsets_.push_back({elems_, offset_});
        elems_ += block_elems_;
        block_elems_ = 0;
        offset_ += current_block_.write(out_, scratch_);
        current_block_.clear();
        for (size_t i = 0; i < block_type::scratch_blocks; i++) {
            std::memset(scratch_[i], 0, block_type::scratch_size(i));
        }
        if (!last_block) {
            out_.write(reinterpret_cast<char*>(&cumulative_), alphabet_type::size());
            offset_ += alphabet_type::size();
        }
    }

    void write_root() {
        std::cerr << "Writing \"root\" to file\n"
                  << " Made " << block_offsets_.size() << " blocks\n"
                  << " containing a total of " << elems_ << " elements." << std::endl;

        std::fstream out;
        out.open(prefix_ + suffix_, std::ios::binary | std::ios::out);
        alphabet_type::write_statics(out);
        block_type::write_statics(out);
        out.write(reinterpret_cast<char*>(&offset_), sizeof(uint64_t));
        out.write(reinterpret_cast<char*>(&elems_), sizeof(uint64_t));
        uint64_t n_blocks = block_offsets_.size();
        out.write(reinterpret_cast<char*>(&n_blocks), sizeof(uint64_t));
        out.write(reinterpret_cast<char*>(block_offsets_.data()), n_blocks * sizeof(std::pair<uint64_t, uint64_t>));
        out.write(reinterpret_cast<char*>(char_counts_), sizeof(uint64_t) * 257);
        out.close();
    }
};
}  // namespace bbwt
