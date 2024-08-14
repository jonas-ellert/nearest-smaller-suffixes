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

#include <divsufsort.h>
#include <divsufsort64.h>

#pragma once

template <typename index_type, typename value_type>
static void lyndon_isa_nsv(value_type const* const text, index_type * const result, const uint64_t n) {
  static_assert(sizeof(index_type) == 4 || sizeof(index_type) == 8);
  static_assert(std::is_unsigned<index_type>::value);

  if (sizeof(index_type) == 4 && n > std::numeric_limits<int>::max()) {
    std::cerr << "WARNING: lyndon_isa_nsv --- n=" << n
              << ": Given index_type of width " << sizeof(index_type)
              << " bytes is insufficient!" << std::endl;
  }

  using sa_type = typename std::conditional<sizeof(index_type) == 4, int32_t,
                                            int64_t>::type;

  sa_type* sa = (sa_type*) result;
  if constexpr (sizeof(index_type) == 4) {
    divsufsort(text, sa, n);
  } else {
    divsufsort64(text, sa, n);
  }

  index_type* isa = (index_type*) malloc(n * sizeof(index_type));
  for (uint64_t i = 0; i < n; ++i) {
    isa[sa[i]] = i;
  }

  result[n - 1] = 1;
  for (index_type i = n - 1; i > 0;) {
    index_type j = i--;
    while (isa[i] < isa[j]) {
      j += result[j];
    }
    result[i] = j - i;
  }
  free(isa);
}
