#pragma once

#include <cmath>
#include <random>
#include <cassert>
#include <cstring>
#include <iostream>
#include <fstream>
#include <utility>
#include <vector>
#include <cstdint>

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

        std::cerr << "Writing \"root\" of " << block_type::cap << "-sb-rlbwt to file\n"
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
    }
};

template <class super_block_type_, class alphabet_type_>
class block_rlbwt {
   public:
    typedef super_block_type_ super_block_type;
    typedef alphabet_type_ alphabet_type;
    typedef block_rlbwt_builder<block_rlbwt> builder;

    struct Char_stats {
        uint64_t start_rank;
        uint64_t end_rank;
        uint8_t c;
    };

   private:
    static const constexpr uint64_t SUPER_BLOCK_ELEMS = uint64_t(1) << 32;
    inline static std::vector<uint64_t> scratch;
    inline static std::vector<uint64_t> scratch_b;
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
        if (scratch.size() < alphabet_type::elems()) {
            scratch.resize(alphabet_type::elems());
            scratch_b.resize(alphabet_type::elems());
        }
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
        std::memcpy(char_counts_, other.char_counts_, sizeof(uint64_t) * 257);
    }

    block_rlbwt& operator=(block_rlbwt&& other) {
        size_ = std::exchange(other.size_, 0);
        block_count_ = std::exchange(other.block_count_, 0);
        p_sums_ = std::exchange(other.p_sums_, nullptr);
        s_blocks_ =
            std::exchange(other.s_blocks_, std::vector<super_block_type*>());
        std::memcpy(char_counts_, other.char_counts_, sizeof(uint64_t) * 257);
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
            if (ret == 0) [[unlikely]] {
                break;
            }
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

    uint64_t i_rank(u_int64_t i, uint8_t& c) const {
        ++i;
        uint64_t s_block_i = i / SUPER_BLOCK_ELEMS;
        std::fill(scratch.begin(), scratch.end(), 0);
        uint64_t res = s_blocks_[s_block_i]->i_rank(c, i % SUPER_BLOCK_ELEMS, scratch);
        res += reinterpret_cast<alphabet_type*>(p_sums_ + alphabet_type::size() * s_block_i)->p_sum(c);
        c = alphabet_type::revert(c);
        return res;
    }

    std::vector<Char_stats> interval_symbols(uint64_t start, uint64_t end) const {
        calculate_interval(start, end);
        std::vector<Char_stats> stats;
        for (uint16_t i = 0; i < alphabet_type::elems(); ++i) {
            if (scratch_b[i] > scratch[i]) {
                stats.push_back({scratch[i], scratch_b[i], alphabet_type::revert(i)});
            }
        }
        return stats;
    }

    template <class c_vec, class i_vec>
    void interval_symbols(uint64_t start, uint64_t end, uint8_t& k, c_vec& cs, i_vec& rank_c_i, i_vec& rank_c_j) const {
        calculate_interval(start, end);
        cs.clear();
        rank_c_i.clear();
        rank_c_j.clear();
        for (uint16_t i = 0; i < alphabet_type::elems(); ++i) {
            if (scratch_b[i] > scratch[i]) {
                cs.push_back(alphabet_type::revert(i));
                rank_c_i.push_back(scratch[i]);
                rank_c_j.push_back(scratch_b[i]);
            }
        }
    }

    uint64_t select(uint64_t x, uint8_t c) const {
        c = alphabet_type::convert(c);
        uint64_t s_b_count = size_ / SUPER_BLOCK_ELEMS;
        uint64_t l_ps = 0;
        uint64_t sb_trg = 0;
        for (uint64_t sb_idx = 1; sb_idx < s_b_count; ++sb_idx) {
            uint64_t ps = reinterpret_cast<alphabet_type*>(p_sums_ + alphabet_type::size() * sb_idx)->p_sum(c);
            if (ps > x) {
                break;
            }
            sb_trg++;
            l_ps = ps;
        }
        uint64_t res = sb_trg * SUPER_BLOCK_ELEMS;
        x -= l_ps;
        uint64_t s_b_s = res + SUPER_BLOCK_ELEMS > size_ ? size_ % SUPER_BLOCK_ELEMS : SUPER_BLOCK_ELEMS;
        res += s_blocks_[sb_trg]->select(x, c, s_b_s);
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
        uint8_t* data = (uint8_t*)std::malloc(in_bytes + block_type::padding_bytes);
        if constexpr (block_type::padding_bytes) {
            std::memset(data + in_bytes, 0, block_type::padding_bytes);
        }
        in_file.read(reinterpret_cast<char*>(data), in_bytes);
        bytes_ += in_bytes;
        return reinterpret_cast<super_block_type*>(data);
    }

    void calculate_interval(uint64_t start, uint64_t end) const {
        std::fill(scratch.begin(), scratch.end(), 0);
        std::fill(scratch_b.begin(), scratch_b.end(), 0);


        uint64_t s_block_i = start / SUPER_BLOCK_ELEMS;
        uint64_t e_block_i = end / SUPER_BLOCK_ELEMS;
        if (s_block_i == e_block_i) {
            const alphabet_type* alpha = reinterpret_cast<alphabet_type*>(p_sums_ + alphabet_type::size() * s_block_i);
            for (uint16_t i = 0; i < alphabet_type::elems(); ++i) {
                scratch[i] = alpha->p_sum(i);
                scratch_b[i] = alpha->p_sum(i);
            }
            s_blocks_[s_block_i]->interval_statistics(start % SUPER_BLOCK_ELEMS, end % SUPER_BLOCK_ELEMS, scratch, scratch_b);
        } else {
            const alphabet_type* alpha = reinterpret_cast<alphabet_type*>(p_sums_ + alphabet_type::size() * s_block_i);
            for (uint16_t i = 0; i < alphabet_type::elems(); ++i) {
                scratch[i] = alpha->p_sum(i);
            }
            s_blocks_[s_block_i]->c_rank(start % SUPER_BLOCK_ELEMS, scratch);

            alpha = reinterpret_cast<alphabet_type*>(p_sums_ + alphabet_type::size() * e_block_i);
            for (uint16_t i = 0; i < alphabet_type::elems(); ++i) {
                scratch_b[i] = alpha->p_sum(i);
            }
            s_blocks_[e_block_i]->c_rank(end % SUPER_BLOCK_ELEMS, scratch_b);
        }
    }
};
}  // namespace bbwt
