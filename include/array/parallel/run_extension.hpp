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

#include "common/util.hpp"

namespace xss {
namespace internal {

  template <bool build_nss,
            bool build_lyndon,
            typename ctx_type,
            typename index_type>
  xss_always_inline static void
  pss_array_run_extension(ctx_type& ctx,
                          const index_type j,
                          index_type& i,
                          const index_type max_lce,
                          const index_type period,
                          const index_type upper) {
    static_assert(!(build_nss && build_lyndon));
    const index_type repetitions =
        std::min(max_lce / period - 1, (upper - i) / period);
    const index_type new_i = i + (repetitions * period);

    if constexpr (build_nss || build_lyndon) {
      for (index_type r = i; r < new_i;) {
        index_type k = r + 1;
        r += period;
//        std::stringstream ss;
//        ss << i << " " << j << " " << max_lce << " " << period << "\n";
        for (; k < r; ++k) {
//          ss << k << "\n";
          if constexpr (build_nss)
            ctx.aux[k] = ctx.aux[k - period] + period;
          if constexpr (build_lyndon)
            ctx.aux[k] = ctx.aux[k - period];
        }
//        std::cout << ss.str() << std::flush;
      }
    }

    for (index_type k = i + 1; k < new_i; ++k) {
      ctx.array[k] = ctx.array[k - period] + period;
    }

    // INCREASING RUN
    if (ctx.text[j + max_lce] < ctx.text[i + max_lce]) {
      for (index_type r = 0; r < repetitions; ++r) {
        i += period;
        ctx.array[i] = i - period;
      }
    }
    // DECREASING RUN
    else {
      const index_type pss_of_new_i = ctx.array[i];
      for (index_type r = 0; r < repetitions; ++r) {
        if constexpr (build_nss)
          ctx.aux[i] = i + period;
        if constexpr (build_lyndon)
          ctx.aux[i] = period;
        i += period;
        ctx.array[i] = pss_of_new_i;
      }
    }
  }

} // namespace internal
} // namespace xss