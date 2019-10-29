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
#include "stack.hpp"

namespace xss {
namespace internal {

  template <typename ctx_type, typename index_type>
  xss_always_inline static void pss_tree_find_pss(ctx_type& ctx,
                                                  const index_type j,
                                                  const index_type i,
                                                  const index_type lce,
                                                  index_type& max_lce_j,
                                                  index_type& max_lce,
                                                  index_type& pss_of_i) {

    auto& stack = ctx.stack;
    reverse_telescope_stack reverse_stack;

    uint64_t new_j = j;
    uint64_t new_lce = lce;

    max_lce = lce;

    while (ctx.text[new_j + new_lce] > ctx.text[i + new_lce]) {
      reverse_stack.push(new_j);
      stack.pop();
      new_j = stack.top();
      new_lce = ctx.get_lce.without_bounds(new_j, i);
      if (new_lce >= max_lce) {
        max_lce = new_lce;
        max_lce_j = new_j;
      }
    }
    pss_of_i = new_j;

    while(reverse_stack.top() < reverse_telescope_stack::max_val) {
      stack.push(reverse_stack.top());
      reverse_stack.pop();
    }
  }

} // namespace internal
} // namespace xss