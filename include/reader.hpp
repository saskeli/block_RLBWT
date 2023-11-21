#pragma once

#include <fstream>
#include <iostream>
#include <cstdint>

namespace bbwt {
template<class alphabet_type>
class file_reader {
   private:
    std::istream* in;

    struct if_ref {
        uint32_t length;
        uint8_t head;
    };
    class if_iterator {
       private:
        std::istream* stream_;

       public:
        uint32_t length_;
        uint8_t head_;

       private:
        uint8_t next_head_;

       public:
        if_iterator(uint8_t head, std::istream* stream)
            : stream_(stream), length_(head ? 1 : 0), head_(head) {
            if (head) {
                char c;
                while (stream_->read(&c, 1)) {
                    if (stream_->eof()) {
                        next_head_ = '\0';
                        break;
                    }
                    if (alphabet_type::convert(c) == alphabet_type::convert(head_)) {
                        length_++;
                    } else {
                        next_head_ = uint8_t(c);
                        break;
                    }
                }
            } else {
                next_head_ = '\0';
            }
        }

        bool operator==(const if_iterator& rhs) const {
            return head_ == rhs.head_ && length_ == rhs.length_;
        }

        bool operator!=(const if_iterator& rhs) const {
            return !operator==(rhs);
        }

        if_iterator& operator++() {
            head_ = next_head_;
            if (!stream_->eof()) {
                length_ = 1;
                char c;
                while (stream_->get(c)) {
                    if (alphabet_type::convert(c) == alphabet_type::convert(head_)) {
                        length_++;
                    } else {
                        next_head_ = uint8_t(c);
                        break;
                    }
                }
                if (stream_->eof()) {
                    next_head_ = '\0';
                }
            } else {
                length_ = 0;
            }
            return *this;
        }

        if_ref operator*() { return {length_, head_}; }
    };

   public:
    file_reader(std::istream* in_file) : in(in_file) {}

    if_iterator begin() {
        uint8_t c = in->get();
        return if_iterator(c, in);
    }

    if_iterator end() { return if_iterator('\0', in); }
};

template<class alphabet_type>
class multi_reader {
   private:
    std::ifstream* heads_;
    std::ifstream* runs_;

    struct mf_ref {
        uint32_t length;
        uint8_t head;
    };
    class mf_iterator {
       private:
        std::ifstream* heads_;
        std::ifstream* runs_;

       public:
        uint32_t length_;
        uint8_t head_;
        mf_iterator(std::ifstream* heads, std::ifstream* runs)
            : heads_(heads), runs_(runs) {
            heads_->read(reinterpret_cast<char*>(&head_), 1);
            runs_->read(reinterpret_cast<char*>(&length_), sizeof(uint32_t));
            if (heads_->gcount() == 0 || runs_->gcount() == 0) {
                head_ = '\0';
                length_ = 0;
                return;
            }
            int c = heads_->peek();
            while (c != EOF && alphabet_type::convert(c) == alphabet_type::convert(head_)) {
                heads_->read(reinterpret_cast<char*>(&head_), 1);
                uint32_t len;
                runs_->read(reinterpret_cast<char*>(&len), sizeof(uint32_t));
                length_ += len;
                c = heads_->peek();
            }
        }

        mf_iterator(std::ifstream* heads, std::ifstream* runs, uint32_t length,
                    uint8_t head)
            : heads_(heads), runs_(runs), length_(length), head_(head) {}

        bool operator==(const mf_iterator& rhs) const {
            return head_ == rhs.head_ && length_ == rhs.length_;
        }

        bool operator!=(const mf_iterator& rhs) const {
            return !operator==(rhs);
        }

        mf_iterator& operator++() {
            heads_->read(reinterpret_cast<char*>(&head_), 1);
            runs_->read(reinterpret_cast<char*>(&length_), sizeof(uint32_t));
            if (heads_->gcount() == 0 || runs_->gcount() == 0) {
                head_ = '\0';
                length_ = 0;
                return *this;
            }
            int c = heads_->peek();
            while (c != EOF && alphabet_type::convert(c) == alphabet_type::convert(head_)) {
                heads_->read(reinterpret_cast<char*>(&head_), 1);
                uint32_t len;
                runs_->read(reinterpret_cast<char*>(&len), sizeof(uint32_t));
                length_ += len;
                c = heads_->peek();
            }
            return *this;
        }

        mf_ref operator*() { return {length_, head_}; }
    };

   public:
    multi_reader(std::ifstream* heads, std::ifstream* runs)
        : heads_(heads), runs_(runs) {}

    mf_iterator begin() { return mf_iterator(heads_, runs_); }

    mf_iterator end() { return mf_iterator(heads_, runs_, 0, '\0'); }
};
}  // namespace bbwt
