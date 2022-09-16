#pragma once

#include <iostream>
#include <fstream>

namespace bbwt {
class file_reader {
  private:
    std::istream* in;
    struct if_ref {
        uint32_t length;
        char head;
    };
    class if_iterator {
      private:
        std::istream* stream_;
      public:
        uint32_t length_;
        char head_;
      private:  
        char next_head_;
      public:
        if_iterator(char head, std::istream* stream) : stream_(stream), length_(head ? 1 : 0), head_(head) {
            if (head) {
                char c;
                while (stream_->get(c)) {
                    if (c == head_) {
                        length_++;
                    } else {
                        next_head_ = c;
                        break;
                    }
                }
                if (stream_->eof()) {
                    next_head_ = '\0';
                }
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
            if (head_) {
                length_ = 1;
                char c;
                while (stream_->get(c)) {
                    if (c == head_) {
                        length_++;
                    } else {
                        next_head_ = c;
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

        if_ref operator*() {
            return {length_, head_};
        }
    };

  public:
    file_reader(std::istream* in_file): in(in_file) {}

    if_iterator begin() {
        char c = in->get();
        return if_iterator(c, in);
    }

    if_iterator end() {
        return if_iterator('\0', in);
    }
};

class multi_reader {
  private:
    std::FILE* heads_;
    std::FILE* runs_;

    struct mf_ref {
        uint32_t length;
        char head;
    };
    class mf_iterator {
      private:
        std::FILE* heads_;
        std::FILE* runs_;
      public:
        uint32_t length_;
        char head_;
        mf_iterator(std::FILE* heads, std::FILE* runs) : heads_(heads), runs_(runs) {
            size_t read_a = std::fread(&head_, sizeof(char), 1, heads_);
            size_t read_b = std::fread(&length_, sizeof(uint32_t), 1, runs_);
            if (read_a == 0 || read_b == 0) {
                head_ = '\0';
                length_ = 0;
            }
        }

        mf_iterator() : heads_(nullptr), runs_(nullptr), length_(0), head_('\0') {}

        bool operator==(const mf_iterator& rhs) const {
            return head_ == rhs.head_ && length_ == rhs.length_;
        }

        bool operator!=(const mf_iterator& rhs) const {
            return !operator==(rhs);
        }

        mf_iterator& operator++() {
            size_t read_a = std::fread(&head_, sizeof(char), 1, heads_);
            size_t read_b = std::fread(&length_, sizeof(uint32_t), 1, runs_);
            if (read_a == 0 || read_b == 0) {
                head_ = '\0';
                length_ = 0;
            }
            return *this;
        }

        mf_ref operator*() {
            return {length_, head_};
        }
    };
  public:
    multi_reader(std::FILE* heads, std::FILE* runs) : heads_(heads), runs_(runs) {}

    mf_iterator begin() {
        return mf_iterator(heads_, runs_);
    }

    mf_iterator end() {
        return mf_iterator();
    }
};
} // namespace bbwt
