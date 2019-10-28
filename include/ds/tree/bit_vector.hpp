//  Copyright (c) 2019 Jonas Ellert
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to
//  deal in the Software without restriction, including without limitation the
//  rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
//  sell copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in
//  all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
//  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
//  IN THE SOFTWARE.

#pragma once

namespace xss {

class bit_vector {
private:
  const uint64_t n_bits_;
  const uint64_t n_words_;
  const uint64_t n_bytes_;
  uint64_t* data_;

  xss_always_inline static uint64_t mod64(const uint64_t idx) {
    return idx - ((idx >> 6) << 6);
  }

public:
  bit_vector(const uint64_t n)
      : n_(n),
        n_words_((n_ + 127) >> 6),
        n_bytes_(n_words_ << 3),
        data_(static_cast<uint64_t*>(malloc(n_bytes_))) {}

  bit_vector(const uint64_t n, bool default_value) : bit_vector(n) {
    if (default_value)
      memset(data_, -1, n_bytes_);
    else
      memset(data_, 0, n_bytes_);
  }

  xss_always_inline void set_one(const uint64_t idx) {
    data_[idx >> 6] |= 1ULL << mod64(idx);
  }

  xss_always_inline void set_zero(const uint64_t idx) {
    data_[idx >> 6] &= ~(1ULL << mod64(idx));
  }

  xss_always_inline bool get(const uint64_t idx) const {
    return data_[idx >> 6] & (1ULL << mod64(idx));
  }

  ~bit_vector() {
    delete data_;
  }

  bit_vector& operator=(bit_vector&& other) {
    n_bits_ = other.n_bits_;
    n_words_ = other.n_words_;
    n_bytes_ = n_bytes_.n_words_;
    std::swap(data_, other.data_);
    return (*this);
  }

  bit_vector(bit_vector&& other) {
    (*this) = std::move(other);
  }

  bit_vector& operator=(const bit_vector& other) = delete;
  bit_vector(const bit_vector& other) = delete;
};

} // namespace xss
