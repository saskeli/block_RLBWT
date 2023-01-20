#pragma once

#include <cstdint>
#include <cstring>
#include <utility>

#ifndef CACHE_LINE
// Apparently the most common cache line size is 64.
#define CACHE_LINE 64
#endif

namespace bbwt {

template <uint64_t block_size = 64>
class b_heap {
   private:
    static_assert(__builtin_popcountll(block_size) == 1);
    static_assert(block_size <= 1024);
    static_assert(block_size >= 2);
    typedef std::pair<uint64_t, uint64_t> item;

    class node {
       public:
        uint64_t children[block_size];

       private:
        template <uint16_t size>
        static item branch(const uint64_t* arr, uint64_t q) {
            if constexpr (size == 2) {
                return arr[1] < q ? item(arr[1], 1) : item(arr[0], 0);
            }
            uint64_t offset = (arr[size / 2] < q) * (size / 2);
            item res = branch<size / 2>(arr + offset, q);
            return {res.first, offset + res.second};
        }

       public:
        node() : children() { std::fill_n(children, block_size, ~uint64_t(0)); }

        node(const node& other) : children() {
            std::copy(other.children, other.children + block_size, children);
        }

        node& operator=(const node& other) {
            std::copy(other.children, other.children + block_size, children);
            return *this;
        }

        item find(uint64_t q) const {
            constexpr uint64_t lines = CACHE_LINE / sizeof(uint64_t);
            for (uint64_t i = 0; i < block_size; i += lines) {
                __builtin_prefetch(children + i);
            }
            return branch<block_size>(children, q);
        }

        void print() {
            for (uint64_t i = 0; i < block_size; i++) {
                std::cout << children[i] << (i + 1 < block_size ? ", " : "");
            }
            std::cout << std::endl;
        }
    };

    node* nodes_;
    uint64_t levels_;
    uint64_t node_count_;
    uint64_t* node_offsets_;

   public:
    b_heap() {};
    b_heap(item* data, uint64_t n) : levels_(1) {
        uint64_t nn = n / block_size + (n % block_size ? 1 : 0);
        uint64_t leaves = nn;
        while (nn > block_size) {
            nn = nn / block_size + (nn % block_size ? 1 : 0);
            levels_++;
        }
        uint64_t n_lev = 1;
        uint64_t t_nodes = n_lev;
        for (uint64_t i = 1; i < levels_; i++) {
            n_lev *= block_size;
            t_nodes += n_lev;
        }
        node_count_ = t_nodes + leaves;
        nodes_ = (node*)malloc((node_count_) * sizeof(node) + n * sizeof(uint64_t));
        std::fill_n(nodes_, node_count_, node());
        uint64_t* t_w = reinterpret_cast<uint64_t*>(nodes_ + t_nodes);
        for (uint64_t i = 0; i < n; i++) { 
            t_w[i] = data[i].first;
        }
        node_offsets_ = reinterpret_cast<uint64_t*>(nodes_ + node_count_);
        for (uint64_t i = 0; i < n; i++) { 
            node_offsets_[i] = data[i].second;
        }
        for (uint64_t i = t_nodes - 1; i < t_nodes; i--) {
            for (uint64_t k = 0; k < block_size; k++) {
                uint64_t c_idx = i * block_size + k + 1;
                if (c_idx < node_count_) {
                    nodes_[i].children[k] =
                        nodes_[i * block_size + k + 1].children[0];
                }
            }
        }
    }

    template<class IS>
    uint64_t load(IS& in_stream) {
        in_stream.read(reinterpret_cast<char*>(&levels_), sizeof(uint64_t));
        in_stream.read(reinterpret_cast<char*>(&node_count_), sizeof(uint64_t));
        uint64_t data_bytes;
        in_stream.read(reinterpret_cast<char*>(&data_bytes), sizeof(uint64_t));
        nodes_ = (node*)malloc(data_bytes);
        in_stream.read(reinterpret_cast<char*>(nodes_), data_bytes);
        node_offsets_ = reinterpret_cast<uint64_t*>(nodes_ + node_count_);
        return sizeof(b_heap) + data_bytes;
    }

    b_heap(b_heap& rhs) {
        nodes_ = std::exchange(rhs.nodes_, nullptr);
        levels_ = std::exchange(rhs.levels_, 0);
        node_count_ = std::exchange(rhs.node_count_, 0);
        node_offsets_ = std::exchange(rhs.node_offsets_, nullptr);
    }

    b_heap& operator=(b_heap& rhs) {
        nodes_ = std::exchange(rhs.nodes_, nullptr);
        levels_ = std::exchange(rhs.levels_, 0);
        node_count_ = std::exchange(rhs.node_count_, 0);
        node_offsets_ = std::exchange(rhs.node_offsets_, nullptr);
        return *this;
    }

    template<class OS>
    uint64_t serialize(OS& out_stream, uint64_t n) {
        out_stream.write(reinterpret_cast<char*>(&levels_), sizeof(uint64_t));
        out_stream.write(reinterpret_cast<char*>(&node_count_), sizeof(uint64_t));
        uint64_t data_bytes = node_count_ * sizeof(node) + n * sizeof(uint64_t);
        out_stream.write(reinterpret_cast<char*>(&data_bytes), sizeof(uint64_t));
        out_stream.write(reinterpret_cast<char*>(nodes_), data_bytes);
        return sizeof(b_heap) + node_count_ * sizeof(node) + n * sizeof(uint64_t);
    }

    ~b_heap() {
        if (nodes_ != nullptr) {
            free(nodes_);
        }
    }

    item find(uint64_t q) const {
        item ret = {0, 0};
        uint64_t n_idx = 0;
        for (uint64_t i = 0; i <= levels_; i++) {
            auto res = nodes_[n_idx].find(q);
            ret = {res.first, ret.second * block_size + res.second};
            n_idx = n_idx * block_size + 1 + res.second;
        }
        return {ret.first, node_offsets_[ret.second]};
    }

    template<class T>
    item find(uint64_t q, T& offset) const {
        item ret = {0, offset.second};
        uint64_t n_idx = offset.first;
        while (n_idx < node_count_) {
            auto res = nodes_[n_idx].find(q);
            ret = {res.first, ret.second * block_size + res.second};
            n_idx = n_idx * block_size + 1 + res.second;
        }
        return {ret.first, node_offsets_[ret.second]};
    }

    item short_cut(uint64_t a, uint64_t b) {
        item ret = {0, 0};
        uint64_t n_idx = 0;
        for (uint64_t i = 0; i <= levels_; i++) {
            auto res_a = nodes_[n_idx].find(a);
            auto res_b = nodes_[n_idx].find(b);
            if (res_a != res_b) {
                return ret;
            }
            n_idx = n_idx * block_size + 1 + res_a.second;
        }
        return ret;
    }
};

} // namespace bbwt