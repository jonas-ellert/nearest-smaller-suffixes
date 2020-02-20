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

#include "array/algorithm.hpp"
#include "tree/algorithm.hpp"
#include "tree/support/pss_tree_support_naive.hpp"

namespace xss {
enum array_type { pss, nss, lyndon, none };

constexpr static array_type PSS = array_type::pss;
constexpr static array_type NSS = array_type::nss;
constexpr static array_type LYNDON = array_type::lyndon;

template <array_type T1,
          array_type T2,
          typename index_type = uint32_t,
          typename value_type>
static auto get(const value_type* text,
                const uint64_t n,
                uint64_t threshold = internal::DEFAULT_THRESHOLD) {

  static_assert(T1 != array_type::none);
  if constexpr (T2 == array_type::none) {
    if constexpr (T1 == PSS)
      return pss_array(text, n, threshold);
    if constexpr (T1 == NSS)
      return nss_array(text, n, threshold);
    if constexpr (T1 == LYNDON)
      return lyndon_array(text, n, threshold);
    else {
      std::abort();
    }
  } else {
    static_assert(T1 == PSS);
    static_assert(T2 != PSS);
    if constexpr (T2 == NSS)
      return pss_and_nss_array(text, n, threshold);
    if constexpr (T2 == LYNDON)
      return pss_and_lyndon_array(text, n, threshold);
    else {
      std::abort();
    }
  }
}

template <array_type T1, typename index_type = uint32_t, typename value_type>
static auto get(const value_type* text,
                const uint64_t n,
                uint64_t threshold = internal::DEFAULT_THRESHOLD) {
  return get<T1, array_type::none, index_type, value_type>(text, n, threshold);
}
} // namespace xss
