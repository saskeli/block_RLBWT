#pragma once

#include <vector>
#include <random>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <fstream>
#include <utility>

#include "b_heap.hpp"
#include "custom_alphabet.hpp"
#include "alphabet.hpp"

namespace bbwt {
template <class bwt_type>
class run_rlbwt_builder {
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
    run_rlbwt_builder(std::string out_file)
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
        std::cerr << "Writing \"root\" of " << block_type::cap << "-rb-rlbwt to file\n"
                  << " Made " << block_offsets_.size() << " blocks\n"
                  << " containing a total of " << elems_ << " elements." << std::endl;

        std::fstream out;
        out.open(prefix_ + suffix_, std::ios::binary | std::ios::out);
        alphabet_type::write_statics(out);
        block_type::write_statics(out);
        out.write(reinterpret_cast<char*>(&offset_), sizeof(uint64_t));
        out.write(reinterpret_cast<char*>(&elems_), sizeof(uint64_t));
        b_heap<> b_h(block_offsets_.data(), block_offsets_.size());
        b_h.serialize(out, block_offsets_.size());
        out.write(reinterpret_cast<char*>(char_counts_), sizeof(uint64_t) * 257);
        out.close();
    }
};

template <class block_type_, uint64_t f_index = 0>
class run_rlbwt {
   public:
    typedef block_type_ block_type;
    typedef block_type::alphabet_type alphabet_type;
    typedef run_rlbwt_builder<run_rlbwt> builder;

    struct Char_stats {
        uint64_t start_rank;
        uint64_t end_rank;
        uint8_t c;
    };

   private:
    inline static std::vector<uint64_t> scratch;
    inline static std::vector<uint64_t> scratch_b;

    uint64_t size_;
    uint64_t block_count_;
    uint64_t bytes_;
    uint64_t char_counts_[257];
    b_heap<> b_h_;
    uint8_t* data_;
    std::vector<std::pair<uint64_t, uint64_t>> skips;

   public:
    run_rlbwt(std::string path) : bytes_(sizeof(run_rlbwt)) {
        std::fstream in_file;
        in_file.open(path, std::ios::binary | std::ios::in);
        if (in_file.fail()) {
            std::cerr << "Opening " << path << " failed!" << std::endl;
            exit(1);
        }
        bytes_ += alphabet_type::load_statics(in_file);
        bytes_ += block_type::load_statics(in_file);
        if (scratch.size() < alphabet_type::elems()) {
            scratch.resize(alphabet_type::elems());
            scratch_b.resize(alphabet_type::elems());
        }
        uint64_t data_bytes;
        in_file.read(reinterpret_cast<char*>(&data_bytes), sizeof(uint64_t));
        in_file.read(reinterpret_cast<char*>(&size_), sizeof(uint64_t));

        bytes_ += b_h_.load(in_file);

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

        if constexpr (f_index) {
            build_f_index();
        }
    }

    run_rlbwt() = delete;
    run_rlbwt(const run_rlbwt&) = delete;
    run_rlbwt& operator=(const run_rlbwt&) = delete;

    run_rlbwt(run_rlbwt&& other) {
        size_ = std::exchange(other.size_, 0);
        bytes_ = std::exchange(other.bytes_, 0);
        block_count_ = std::exchange(other.block_count_, 0);
        data_ = std::exchange(other.data_, nullptr);
        b_h_ = other.b_h_;
        std::memcpy(char_counts_, other.char_counts_, sizeof(uint64_t) * 257);
    }

    run_rlbwt& operator=(run_rlbwt&& other) {
        size_ = std::exchange(other.size_, 0);
        bytes_ = std::exchange(other.bytes_, 0);
        block_count_ = std::exchange(other.block_count_, 0);
        data_ = std::exchange(other.data_, nullptr);
        b_h_ = other.b_h_;
        std::memcpy(char_counts_, other.char_counts_, sizeof(uint64_t) * 257);
        return *this;
    }

    ~run_rlbwt() {
        if (data_ != nullptr) {
            std::free(data_);
        }
    }

    uint8_t at(uint64_t i) const {
        if (i >= size_) [[unlikely]] {
            return 0;
        }
        auto count = b_h_.find(i);
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
        if (i >= size_) [[unlikely]] {
            return char_counts_[c + 1] - char_counts_[c];
        }
        c = alphabet_type::convert(c);
        auto count = f_index ? b_h_.find(i, skips[i / f_index]) : b_h_.find(i);
        i -= count.first;
        uint64_t res = reinterpret_cast<alphabet_type*>(data_ + count.second - alphabet_type::size())->p_sum(c);
        res += reinterpret_cast<block_type*>(data_ + count.second)->rank(c, i);
        return res;
    }

    uint64_t i_rank(uint64_t i, uint8_t& c) const {
        std::fill(scratch.begin(), scratch.end(), 0);
        ++i;
        auto count = f_index ? b_h_.find(i, skips[i / f_index]) : b_h_.find(i);
        i -= count.first;
        uint64_t res = reinterpret_cast<const block_type*>(data_ + count.second)->i_rank(c, i, scratch);
        res += reinterpret_cast<const alphabet_type*>(data_ + count.second - alphabet_type::size())->p_sum(c);
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
    void interval_symbols(uint64_t start, uint64_t end, uint8_t& k, c_vec& cs, i_vec& rank_c_i, i_vec& rank_c_j) {
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

    uint64_t select(uint64_t i, uint8_t c) const {
        c = alphabet_type::convert(c);
        uint64_t a_l = 0;
        auto a = f_index ? b_h_.find(a_l, skips[a_l]) : b_h_.find(a_l);
        uint64_t b_l = size_ - 1;
        auto b = f_index ? b_h_.find(b_l, skips[b_l / f_index]) : b_h_.find(b_l);
        while (a.first < b.first) {
            uint64_t m_l = (a_l + b_l + 1) / 2;
            auto m = f_index ? b_h_.find(m_l, skips[m_l / f_index]) : b_h_.find(m_l);
            const alphabet_type* alpha = reinterpret_cast<const alphabet_type*>(data_ + m.second - alphabet_type::size());
            if (alpha->p_sum(c) >= i) {
                b_l = m_l - 1;
                b = f_index ? b_h_.find(b_l, skips[a_l]) : b_h_.find(b_l);
            } else {
                a_l = m_l;
                a = f_index ? b_h_.find(a_l, skips[a_l / f_index]) : b_h_.find(a_l);
            }
        }
        const alphabet_type* alpha = reinterpret_cast<const alphabet_type*>(data_ + a.second - alphabet_type::size());
        const block_type* block = reinterpret_cast<const block_type*>(data_ + a.second);
        return a.first + block->select(i - alpha->p_sum(c), c);
    }

    uint8_t operator[](size_t i) const {
        return at(i);
    }

    uint64_t size() const { return size_; }
    uint64_t bytes() const { return bytes_; }
   private:
    void build_f_index() {
        for (uint64_t i = 0; i < size_; i += f_index) {
            skips.push_back(b_h_.short_cut(i, i + f_index));
        }
        bytes_ += skips.size() + sizeof(std::pair<uint64_t, uint64_t>);
    }

    void calculate_interval(uint64_t start, uint64_t end) const {
        std::fill(scratch.begin(), scratch.end(), 0);
        std::fill(scratch_b.begin(), scratch_b.end(), 0);
        auto s_count = f_index ? b_h_.find(start, skips[start / f_index]) : b_h_.find(start);
        if (end >= size_) {
            for (uint16_t i = 0; i < alphabet_type::elems(); ++i) {
                scratch_b[i] = char_counts_[alphabet_type::revert(i)];
            }
            const block_type* block = reinterpret_cast<const block_type*>(data_ + s_count.second);
            const alphabet_type* alpha = reinterpret_cast<const alphabet_type*>(data_ + s_count.second - alphabet_type::size());
            for (uint16_t i = 0; i < alphabet_type::elems(); ++i) {
                scratch[i] = alpha->p_sum(i);
            }
            block->c_rank(start - s_count.first, scratch);
            return;
        }
        auto e_count = f_index ? b_h_.find(end, skips[end / f_index]) : b_h_.find(end);
        if (s_count.second == e_count.second) {
            const block_type* block = reinterpret_cast<const block_type*>(data_ + s_count.second);
            const alphabet_type* alpha = reinterpret_cast<const alphabet_type*>(data_ + s_count.second - alphabet_type::size());
            for (uint16_t i = 0; i < alphabet_type::elems(); ++i) {
                scratch[i] = alpha->p_sum(i);
                scratch_b[i] = alpha->p_sum(i);
            }
            block->interval_statistics(start - s_count.first, end - s_count.first, scratch, scratch_b);
        } else {
            const block_type* block = reinterpret_cast<const block_type*>(data_ + s_count.second);
            const alphabet_type* alpha = reinterpret_cast<const alphabet_type*>(data_ + s_count.second - alphabet_type::size());
            for (uint16_t i = 0; i < alphabet_type::elems(); ++i) {
                scratch[i] = alpha->p_sum(i);
            }
            block->c_rank(start - s_count.first, scratch);
            alpha = reinterpret_cast<const alphabet_type*>(data_ + e_count.second - alphabet_type::size());
            block = reinterpret_cast<const block_type*>(data_ + e_count.second);
            for (uint16_t i = 0; i < alphabet_type::elems(); ++i) {
                scratch_b[i] = alpha->p_sum(i);
            }
            block->c_rank(end - e_count.first, scratch_b);
        }
    }
};
}  // namespace bbwt
