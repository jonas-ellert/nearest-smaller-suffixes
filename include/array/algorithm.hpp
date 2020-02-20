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
#include "find_pss.hpp"
#include "run_extension.hpp"

namespace xss {

namespace internal {

  template <bool build_nss,
            bool build_lyndon,
            typename index_type,
            typename value_type>
  static auto
  pss_and_x_array(const value_type* text,
                  const uint64_t n,
                  uint64_t threshold = internal::DEFAULT_THRESHOLD) {
    using namespace internal;

    using res_type = typename std::conditional<
        build_nss || build_lyndon,
        std::pair<std::vector<index_type>, std::vector<index_type>>,
        std::vector<index_type>>::type;

    static_assert(!(build_nss && build_lyndon));

    if constexpr (build_nss)
      warn_type_width<index_type>(n, "xss::pss_and_nss_array");
    else if constexpr (build_lyndon)
      warn_type_width<index_type>(n, "xss::pss_and_lyndon_array");
    else
      warn_type_width<index_type>(n, "xss::pss_array");

    fix_threshold(threshold);

    res_type result;
    index_type* array;
    index_type* aux;

    if constexpr (build_nss || build_lyndon) {
      result.first = std::vector<index_type>(n, 0);
      result.second = std::vector<index_type>(n, 0);
      array = result.first.data();
      aux = result.second.data();
    } else {
      result = std::vector<index_type>(n, 0);
      array = result.data();
    }

    array_context_type<index_type, value_type> ctx{text, array, (index_type) n};

    array[0] = 0; // will be overwritten with n later
    if constexpr (build_nss || build_lyndon) {
      aux[0] = n - 1;
      ctx.aux = aux;
    }

    index_type j, lce;
    for (index_type i = 1; i < n - 1; ++i) {
      j = i - 1;
      lce = ctx.get_lce.without_bounds(j, i);

      if (xss_likely(lce <= threshold)) {
        while (text[j + lce] > text[i + lce]) {
          if constexpr (build_nss)
            aux[j] = i;
          if constexpr (build_lyndon)
            aux[j] = i - j;
          j = array[j];
          lce = ctx.get_lce.without_bounds(j, i);
          if (xss_unlikely(lce > threshold))
            break;
        }

        if (xss_likely(lce <= threshold)) {
          array[i] = j;
          continue;
        }
      }

      index_type max_lce, max_lce_j, pss_of_i;
      xss_array_find_pss(ctx, j, i, lce, max_lce_j, max_lce, pss_of_i);

      if constexpr (build_nss || build_lyndon) {
        while (j > pss_of_i) {
          if constexpr (build_nss)
            aux[j] = i;
          if constexpr (build_lyndon)
            aux[j] = i - j;
          j = array[j];
        }
      }

      array[i] = pss_of_i;

      const index_type distance = i - max_lce_j;
      if (xss_unlikely(max_lce >= 2 * distance))
        pss_array_run_extension<build_nss, build_lyndon>(ctx, max_lce_j, i,
                                                         max_lce, distance);
      else
        pss_array_amortized_lookahead<build_nss, build_lyndon>(
            ctx, max_lce_j, i, max_lce, distance);
    }

    // PSS does not exist <=> pss[i] = n
    array[0] = array[n - 1] = n;

    if constexpr (build_nss || build_lyndon) {
      j = n - 2;
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
static auto pss_array(const value_type* text,
                      const uint64_t n,
                      uint64_t threshold = internal::DEFAULT_THRESHOLD) {
  return internal::pss_and_x_array<false, false, index_type, value_type>(
      text, n, threshold);
}

template <typename index_type = uint32_t, typename value_type>
static auto
pss_and_nss_array(const value_type* text,
                  const uint64_t n,
                  uint64_t threshold = internal::DEFAULT_THRESHOLD) {
  return internal::pss_and_x_array<true, false, index_type, value_type>(
      text, n, threshold);
}

template <typename index_type = uint32_t, typename value_type>
static auto
pss_and_lyndon_array(const value_type* text,
                     const uint64_t n,
                     uint64_t threshold = internal::DEFAULT_THRESHOLD) {
  return internal::pss_and_x_array<false, true, index_type, value_type>(
      text, n, threshold);
}

template <typename index_type = uint32_t, typename value_type>
static auto nss_array(const value_type* text,
                      const uint64_t n,
                      uint64_t threshold = internal::DEFAULT_THRESHOLD) {
  using namespace internal;
  warn_type_width<index_type>(n, "xss::nss_array");
  fix_threshold(threshold);

  std::vector<index_type> result(n, 0);
  index_type* array = result.data();
  array_context_type<index_type, value_type> ctx{text, array, (index_type) n};

  array[0] = 0; // will be overwritten with n - 1 later

  index_type j, lce;
  for (index_type i = 1; i < n - 1; ++i) {
    j = i - 1;
    lce = ctx.get_lce.without_bounds(j, i);

    if (xss_likely(lce < threshold)) {
      while (text[j + lce] > text[i + lce]) {
        index_type next_j = array[j];
        array[j] = i;
        j = next_j;
        lce = ctx.get_lce.without_bounds(j, i);
        if (xss_unlikely(lce > threshold))
          break;
      }

      if (xss_likely(lce <= threshold)) {
        array[i] = j;
        continue;
      }
    }

    index_type max_lce, max_lce_j, pss_of_i;
    xss_array_find_pss(ctx, j, i, lce, max_lce_j, max_lce, pss_of_i);

    while (j > pss_of_i) {
      index_type next_j = array[j];
      array[j] = i;
      j = next_j;
    }
    array[i] = pss_of_i;

    const index_type distance = i - max_lce_j;
    if (xss_unlikely(max_lce >= 2 * distance))
      nss_array_run_extension(ctx, max_lce_j, i, max_lce, distance);
    else
      nss_array_amortized_lookahead(ctx, max_lce_j, i, max_lce, distance);
  }

  // PROCESS ELEMENTS WITHOUT NSS
  j = n - 2;
  while (j > 0) {
    index_type next_j = array[j];
    array[j] = n - 1;
    j = next_j;
  }

  array[0] = n - 1;
  array[n - 1] = n;

  return result;
}

template <typename index_type = uint32_t, typename value_type>
static auto lyndon_array(const value_type* text,
                         const uint64_t n,
                         uint64_t threshold = internal::DEFAULT_THRESHOLD) {
  using namespace internal;
  warn_type_width<index_type>(n, "xss::lyndon_array");
  fix_threshold(threshold);

  std::vector<index_type> result(n, 0);
  index_type* array = result.data();
  array_context_type<index_type, value_type> ctx{text, array, (index_type) n};

  array[0] = 0; // will be overwritten with n - 1 later

  index_type j, lce;
  for (index_type i = 1; i < n - 1; ++i) {
    j = i - 1;
    lce = ctx.get_lce.without_bounds(j, i);

    if (xss_likely(lce < threshold)) {
      while (text[j + lce] > text[i + lce]) {
        index_type next_j = array[j];
        array[j] = i - j;
        j = next_j;
        lce = ctx.get_lce.without_bounds(j, i);
        if (xss_unlikely(lce > threshold))
          break;
      }

      if (xss_likely(lce <= threshold)) {
        array[i] = j;
        continue;
      }
    }

    index_type max_lce, max_lce_j, pss_of_i;
    xss_array_find_pss(ctx, j, i, lce, max_lce_j, max_lce, pss_of_i);

    while (j > pss_of_i) {
      index_type next_j = array[j];
      array[j] = i - j;
      j = next_j;
    }
    array[i] = pss_of_i;

    const index_type distance = i - max_lce_j;
    if (xss_unlikely(max_lce >= 2 * distance))
      lyndon_array_run_extension(ctx, max_lce_j, i, max_lce, distance);
    else
      lyndon_array_amortized_lookahead(ctx, max_lce_j, i, max_lce);
  }

  // PROCESS ELEMENTS WITHOUT NSS
  j = n - 2;
  while (j > 0) {
    index_type next_j = array[j];
    array[j] = n - j - 1;
    j = next_j;
  }

  array[0] = n - 1;
  array[n - 1] = 1;

  return result;
}

// template <typename nss_type, typename index_type>
// static nss_type& nss_to_lyndon(nss_type& nss, const index_type n) {
//  for (index_type i = 0; i < n; ++i) {
//    nss[i] -= i;
//  }
//  return nss;
//}
//
// template <typename lyndon_type, typename index_type>
// static lyndon_type& lyndon_to_nss(lyndon_type& lyndon, const index_type n) {
//  for (index_type i = 0; i < n; ++i) {
//    lyndon[i] += i;
//  }
//  return lyndon;
//}

} // namespace xss
