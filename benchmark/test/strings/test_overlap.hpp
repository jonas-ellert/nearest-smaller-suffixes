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

#include "base.hpp"

static void generate_test_high_overlap_internal(const uint64_t n,
                                                const uint8_t min_char,
                                                vec_type& result) {
  if (n < 2)
    return;
  const auto pre_size = result.size();
  result.push_back(min_char);
  generate_test_high_overlap_internal((n - 2) / 2, min_char + 1, result);
  const auto len = result.size() - pre_size;
  for (uint64_t i = 0; i < len; ++i) {
    result.push_back(result[result.size() - len]);
  }
}

static vec_type generate_test_high_overlap(const uint64_t n) {
  vec_type result;
  result.reserve(n);
  result.push_back(test_gen_sentinel);
  generate_test_high_overlap_internal(n - 2, 'A', result);
  const auto len = result.size() - 1;
  while (result.size() < n - 1)
    result.push_back(result[result.size() - len]);
  result.push_back(test_gen_sentinel);
  return result;
}

template <typename... n_types>
static std::vector<vec_type> get_instances_for_overlap_test(
    const uint64_t instances, const uint64_t min_n, const uint64_t max_n) {
  std::vector<vec_type> result(instances);
  for (uint64_t i = 0; i < instances; ++i) {
    result[i] = generate_test_high_overlap(
        min_n + ((i * (max_n - min_n)) / (instances - 1)));
  }
  return result;
}