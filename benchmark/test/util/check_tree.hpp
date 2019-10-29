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

#include "check_array.hpp"
#include <gtest/gtest.h>

template <bool abort_on_error = true,
          bool report_input_text_on_error = true,
          uint64_t max_text_output_characters = 1ULL * 1024,
          uint64_t max_index_reports_per_check = 5>
struct check_tree {

  using check_array_type = check_array<abort_on_error,
                                       report_input_text_on_error,
                                       max_text_output_characters,
                                       max_index_reports_per_check>;

  template <typename vec_type, typename tree_type>
  inline static void check_bps(const vec_type& text, const tree_type& bps) {
    EXPECT_EQ(text.size() * 2 + 2, bps.size());
    EXPECT_EQ(bps.get(0), true);
    EXPECT_EQ(bps.get(1), true);
    EXPECT_EQ(bps.get(bps.size() - 2), false);
    EXPECT_EQ(bps.get(bps.size() - 1), false);
    std::vector<uint64_t> pss(text.size());
    pss[0] = pss[text.size() - 1] = text.size();
    uint64_t text_i = 1;
    uint64_t text_j = 0;
    for (uint64_t bps_i = 2; bps_i < bps.size() - 2; ++bps_i) {
      if (!bps.get(bps_i)) {
        text_j = pss[text_j];
      } else {
        pss[text_i] = text_j;
        text_j = text_i++;
      }
    }
    check_array_type::check_pss(text, pss);
  }
};