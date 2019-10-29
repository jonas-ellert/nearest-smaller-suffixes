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

#include "../../common/util.hpp"
#include "bit_vector.hpp"
#include "stack.hpp"

namespace xss {

template <typename index_type = uint64_t, typename value_type>
static auto pss_tree_naive(const value_type* text, const uint64_t n) {
  using namespace internal;
  warn_type_width<index_type>(n, "xss::pss_tree_naive");

  bit_vector result((n << 1) + 2);
  parentheses_stream stream(result);
  buffered_stack<telescope_stack, index_type> stack(n >> 3, telescope_stack());
  tree_context_type<index_type, value_type> ctx{text, result, stream,
                                                (index_type) n};

  // open node 0;
  stream.append_opening_parenthesis();
  stream.append_opening_parenthesis();

  index_type j, lce;
  for (index_type i = 1; i < n - 1; ++i) {
    j = i - 1; // = stack.top();
    lce = ctx.get_lce.without_bounds(j, i);

    while (text[j + lce] > text[i + lce]) {
      stack.pop();
      j = stack.top();
      lce = ctx.get_lce.without_bounds(j, i);
      stream.append_closing_parenthesis();
    }

    stack.push(i);
    stream.append_opening_parenthesis();
  }

  while (stack.top() > 0) {
    stack.pop();
    stream.append_closing_parenthesis();
  }
  stream.append_closing_parenthesis();
  stream.append_opening_parenthesis();
  stream.append_closing_parenthesis();
  stream.append_closing_parenthesis();

  return result;
}

} // namespace xss
