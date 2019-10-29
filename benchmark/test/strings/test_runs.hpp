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

static vec_type generate_test_run_of_runs(const uint64_t n,
                                          const uint64_t repetitions) {
  uint8_t alphabet_size = 1;
  {
    uint64_t build_n = repetitions;
    while (build_n < n - 2) {
      build_n = (build_n + 1) * repetitions;
      ++alphabet_size;
    }
  }

  vec_type result(n);
  result[0] = result[n - 1] = test_gen_sentinel;
  result[alphabet_size] = 'A' + alphabet_size - 1;

  uint64_t period_len = 1;
  for (uint8_t cc = alphabet_size - 1; cc > 0; --cc) {
    result[cc] = result[cc + 1] - 1;
    const uint64_t remaining = period_len * (repetitions - 1);
    for (uint64_t i = 0; i < remaining; ++i) {
      result[cc + period_len + i + 1] = result[cc + i + 1];
    }
    period_len = (period_len * repetitions) + 1;
  }

  const uint64_t remaining = n - period_len - 2;
  for (uint64_t i = 0; i < remaining; ++i) {
    result[period_len + i + 1] = result[i + 1];
  }
  return result;
}

template <typename... n_types>
static std::vector<vec_type>
get_instances_for_run_of_runs_test(const uint64_t n) {
  std::vector<vec_type> result;
  for (uint64_t r = 2; r <= std::min((n - 2) >> 2, (uint64_t) 512); r *= 2) {
    result.push_back(generate_test_run_of_runs(n, r));
  }
  return result;
}