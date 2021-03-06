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

#include "xss/common/util.hpp"

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
                          index_type max_lce,
                          const index_type period) {
    bool j_smaller_i = ctx.text[j + max_lce] < ctx.text[i + max_lce];
    const index_type repetitions = max_lce / period - 1;
    const index_type new_i = i + (repetitions * period);

    for (index_type k = i + 1; k < new_i; ++k) {
      ctx.array[k] = ctx.array[k - period] + period;
      if constexpr (build_nss)
        ctx.aux[k] = ctx.aux[k - period] + period;
      if constexpr (build_lyndon)
        ctx.aux[k] = ctx.aux[k - period];
    }

    // INCREASING RUN
    if (j_smaller_i) {
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

  template <typename ctx_type, typename index_type>
  xss_always_inline static void
  nss_array_run_extension(ctx_type& ctx,
                          const index_type j,
                          index_type& i,
                          index_type max_lce,
                          const index_type period) {
    bool j_smaller_i = ctx.text[j + max_lce] < ctx.text[i + max_lce];
    const index_type repetitions = max_lce / period - 1;
    const index_type new_i = i + (repetitions * period);

    for (index_type k = i + 1; k < new_i; ++k) {
      ctx.array[k] = ctx.array[k - period] + period;
    }

    // INCREASING RUN
    if (j_smaller_i) {
      for (index_type r = 0; r < repetitions; ++r) {
        i += period;
        ctx.array[i] = i - period;
      }
    }
    // DECREASING RUN
    else {
      const index_type pss_of_new_i = ctx.array[i];
      for (index_type r = 0; r < repetitions; ++r) {
        ctx.array[i] = i + period;
        i += period;
      }
      ctx.array[i] = pss_of_new_i;
    }
  }

  template <typename ctx_type, typename index_type>
  xss_always_inline static void
  lyndon_array_run_extension(ctx_type& ctx,
                             const index_type j,
                             index_type& i,
                             index_type max_lce,
                             const index_type period) {
    bool j_smaller_i = ctx.text[j + max_lce] < ctx.text[i + max_lce];
    const index_type repetitions = max_lce / period - 1;
    const index_type new_i = i + (repetitions * period);

    for (index_type k = i + 1; k < new_i; ++k) {
      ctx.array[k] = ctx.array[k - period];
    }

    // INCREASING RUN
    if (j_smaller_i) {
      for (index_type r = 0; r < repetitions; ++r) {
        i += period;
        ctx.array[i] = i - period;
      }
    }
    // DECREASING RUN
    else {
      const index_type pss_of_new_i = ctx.array[i];
      for (index_type r = 0; r < repetitions; ++r) {
        ctx.array[i] = period;
        i += period;
      }
      ctx.array[i] = pss_of_new_i;
    }
  }

} // namespace internal
} // namespace xss