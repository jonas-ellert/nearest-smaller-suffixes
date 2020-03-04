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

#include "amortized_lookahead.hpp"
#include "common/context.hpp"
#include "common/util.hpp"
#include "run_extension.hpp"
#include <omp.h>
#include <stack>

namespace xss {

namespace internal {

  static int omp_num_threads() {
    int result = -1;
#pragma omp parallel
    result = omp_get_num_threads();
    return result;
  }

  template <bool build_nss,
            bool build_lyndon,
            typename index_type,
            typename value_type>
  static auto pss_and_x_array_parallel(const value_type* text,
                                       const uint64_t n,
                                       int threads,
                                       uint64_t threshold) {

    using namespace internal;
    static_assert(!(build_nss && build_lyndon));

    using res_type = typename std::conditional<
        build_nss || build_lyndon,
        std::pair<std::vector<index_type>, std::vector<index_type>>,
        std::vector<index_type>>::type;

    if constexpr (build_nss)
      warn_type_width<index_type>(n, "xss::pss_and_nss_array");
    else if constexpr (build_lyndon)
      warn_type_width<index_type>(n, "xss::pss_and_lyndon_array");
    else
      warn_type_width<index_type>(n, "xss::pss_array");

    fix_threshold(threshold);

    res_type result;
    index_type* array;
    index_type* aux = nullptr;

    if constexpr (build_nss || build_lyndon) {
      result.first = std::vector<index_type>(n, n + 1);
      result.second = std::vector<index_type>(n, 0); // this must be initialized with 0!
      array = result.first.data();
      aux = result.second.data();
    } else {
      result = std::vector<index_type>(n, n + 1);
      array = result.data();
    }

    array_context_type<index_type, value_type> ctx{text, array, (index_type) n,
                                                   aux};
    array[0] = 0; // will be overwritten with n later

    const int p = std::min(
        (uint64_t)((threads > 0) ? threads : omp_num_threads()), n >> 1);
    const index_type slice_size = (n + p - 1) / p;

#pragma omp parallel num_threads(p)
    {
      const index_type tn = omp_get_thread_num();
      const index_type lower = std::max((index_type) 1, tn * slice_size);
      const index_type upper =
          std::min((index_type) n - 1, (tn + 1) * slice_size);

      index_type i, j, lce;

      auto auto_lce = [&]() {
        lce = ctx.get_lce.without_bounds(j, i);
        if (xss_unlikely(lce >= threshold)) {
          // finish the entire iteration here!
          index_type max_lce_j = 0, max_lce = 0;

          max_lce = lce;
          max_lce_j = j;

          while (text[j + lce] > text[i + lce]) {
            if (xss_unlikely(array[j] > j))
              return;

            if constexpr (build_nss)
              aux[j] = i;
            if constexpr (build_lyndon)
              aux[j] = i - j;

            j = array[j];
            lce = ctx.get_lce.without_bounds(j, i);
            if (lce >= max_lce) {
              max_lce = lce;
              max_lce_j = j;
            }
          }
          array[i] = j;

          if (max_lce_j > lower) {
            const index_type distance = i - max_lce_j;
            if (max_lce >= 2 * distance) {
              pss_array_run_extension<build_nss, build_lyndon>(
                  ctx, max_lce_j, i, max_lce, distance, upper);
            } else {
              pss_array_amortized_lookahead<build_nss, build_lyndon>(
                  ctx, max_lce_j, i, max_lce, distance, upper);
            }
            j = array[i];
            lce = ctx.get_lce.without_bounds(j, i);
          }
        }
      };

      auto next_j = [&]() {
        if (xss_unlikely(array[j] > j)) {
          index_type k = j - 1;
          lce = ctx.get_lce.without_bounds(k, j);
          while (text[k + lce] > text[j + lce]) {
            // super naive solution
            lce = ctx.get_lce.without_bounds(--k, j);
          }
          j = k;
          lce = ctx.get_lce.without_bounds(j, i);
        } else {
          j = ctx.array[j];
          auto_lce();
        }
      };

      for (i = lower; i < upper; ++i) {
        j = i - 1;
        auto_lce();
        while (text[j + lce] > text[i + lce]) {
          // set secondary array entry
          if constexpr (build_nss)
            aux[j] = i;
          if constexpr (build_lyndon)
            aux[j] = i - j;
          next_j();
        }
        array[i] = j;
      }
    }

    // PSS does not exist <=> pss[i] = n
    array[0] = array[n - 1] = n;
    if constexpr (build_nss)
      aux[n - 1] = n;
    if constexpr (build_lyndon)
      aux[n - 1] = 1;

    if constexpr (build_nss || build_lyndon) {
      aux[0] = n - 1;
      index_type j = n - 2;
      while (j > 0) {
        if constexpr (build_nss)
          aux[j] = n - 1;
        if constexpr (build_lyndon)
          aux[j] = n - j - 1;
        j = array[j];
      }
    }

    return result;
  }
} // namespace internal

template <typename index_type = uint32_t, typename value_type>
static auto
pss_array_parallel(const value_type* text,
                   const uint64_t n,
                   int threads = 0,
                   uint64_t threshold = internal::DEFAULT_THRESHOLD) {
  return internal::pss_and_x_array_parallel<false, false, index_type,
                                            value_type>(text, n, threads,
                                                        threshold);
}

template <typename index_type = uint32_t, typename value_type>
static auto
pss_and_nss_array_parallel(const value_type* text,
                           const uint64_t n,
                           int threads = 0,
                           uint64_t threshold = internal::DEFAULT_THRESHOLD) {
  return internal::pss_and_x_array_parallel<true, false, index_type,
                                            value_type>(text, n, threads,
                                                        threshold);
}

template <typename index_type = uint32_t, typename value_type>
static auto pss_and_lyndon_array_parallel(
    const value_type* text,
    const uint64_t n,
    int threads = 0,
    uint64_t threshold = internal::DEFAULT_THRESHOLD) {
  return internal::pss_and_x_array_parallel<false, true, index_type,
                                            value_type>(text, n, threads,
                                                        threshold);
}

} // namespace xss
