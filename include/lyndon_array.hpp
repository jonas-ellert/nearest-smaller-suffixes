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

#include <cstring>
#include <iostream>
#include <limits>


namespace xssinternal {
  constexpr static uint64_t DEFAULT_THRESHOLD = 128;
}

// INTERFACE:


template <typename index_type, typename value_type>
static void lyndon_array(value_type const* const text,
                         index_type* const array,
                         uint64_t const n,
                         uint64_t threshold = xssinternal::DEFAULT_THRESHOLD);
                         
                         
template <typename index_type, typename value_type>
static void nss_array(value_type const* const text,
                      index_type* const array,
                      uint64_t const n,
                      uint64_t threshold = xssinternal::DEFAULT_THRESHOLD);
                      
                      
template <typename index_type, typename value_type>
static auto pss_array(value_type const* const text,
                      index_type* const pss,
                      uint64_t const n,
                      uint64_t threshold = xssinternal::DEFAULT_THRESHOLD);
                      
                      
template <typename index_type, typename value_type>
static auto
pss_and_lyndon_array(value_type const* const text,
                     index_type* const pss,
                     index_type* const lyndon,
                     uint64_t const n,
                     uint64_t threshold = xssinternal::DEFAULT_THRESHOLD);
                     
                     
template <typename index_type, typename value_type>
static auto
pss_and_nss_array(value_type const* const text,
                  index_type* const pss,
                  index_type* const lyndon,
                  uint64_t const n,
                  uint64_t threshold = xssinternal::DEFAULT_THRESHOLD);



// IMPLEMENTATION

#define xss_always_inline __attribute__((always_inline)) inline
#define xss_likely(x) __builtin_expect(!!(x), 1)
#define xss_unlikely(x) __builtin_expect(!!(x), 0)

namespace xssinternal {
  
  // can never be below 8
  constexpr static uint64_t MIN_THRESHOLD = 8;

  inline static void fix_threshold(uint64_t& threshold) {
    threshold = std::max(threshold, MIN_THRESHOLD);
  }

  template <typename index_type>
  static void warn_type_width(const uint64_t n, const std::string name) {
    if (n > std::numeric_limits<index_type>::max()) {
      std::cerr << "WARNING: " << name << " --- n=" << n
                << ": Given index_type of width " << sizeof(index_type)
                << " bytes is insufficient!" << std::endl;
    }
  }
  
  
  template <typename index_type, typename value_type>
  struct lce_type {
    const value_type* text;

    xss_always_inline index_type without_bounds(const index_type l,
                                                const index_type r,
                                                index_type lce = 0) const {
      while (text[l + lce] == text[r + lce])
        ++lce;
      return lce;
    }

    xss_always_inline index_type
    with_both_bounds(const index_type l,
                     const index_type r,
                     index_type lower,
                     const index_type upper) const {
      while (lower < upper && text[l + lower] == text[r + lower])
        ++lower;
      return lower;
    }

    xss_always_inline index_type with_upper_bound(
        const index_type l, const index_type r, const index_type upper) const {
      return with_both_bounds(l, r, 0, upper);
    }

    xss_always_inline index_type with_lower_bound(
        const index_type l, const index_type r, const index_type lower) const {
      return without_bounds(l, r, lower);
    }
  };
  
  
  template <typename index_type, typename value_type>
  struct array_context_type {

    const value_type* text;
    index_type* array;
    const index_type n;

    index_type* aux = nullptr;

    const lce_type<index_type, value_type> get_lce =
        lce_type<index_type, value_type>{text};
  };
  
  // DUVAL
  template <typename value_type>
  xss_always_inline static std::pair<uint64_t, uint64_t>
  is_extended_lyndon_run(const value_type* text, const uint64_t n) {
    std::pair<uint64_t, uint64_t> result = {0, 0};
    uint64_t i = 0;
    while (i < n) {
      uint64_t j = i + 1, k = i;
      while (j < n && text[k] <= text[j]) {
        if (text[k] < text[j])
          k = i;
        else
          k++;
        j++;
      }
      if (xss_unlikely((j - k) > result.first)) {
        result.first = j - k;
        result.second = i;
      }
      while (i <= k) {
        i += j - k;
      }
    }
    const uint64_t period = result.first;
    if (2 * period > n)
      return {0, 0};
    for (i = period; i < n; ++i) {
      if (xss_unlikely(text[i - period] != text[i]))
        return {0, 0};
    }
    return result;
  }
  
  template <typename index_type, typename value_type>
  xss_always_inline index_type get_anchor(const value_type* lce_str,
                                          const index_type lce_len) {

    const index_type ell = lce_len >> 2;

    // check if gamm_ell is an extended lyndon run
    const auto duval = is_extended_lyndon_run(&(lce_str[ell]), lce_len - ell);

    // try to extend the lyndon run as far as possible to the left
    if (duval.first > 0) {
      const index_type period = duval.first;
      const auto repetition_eq = [&](const index_type l, const index_type r) {
        for (index_type k = 0; k < period; ++k)
          if (lce_str[l + k] != lce_str[r + k])
            return false;
        return true;
      };
      int64_t lhs = ell + duval.second - period;
      while (lhs >= 0 && repetition_eq(lhs, lhs + period)) {
        lhs -= period;
      }
      return std::min(ell, (index_type)(lhs + (period << 1)));
    } else {
      return ell;
    }
  }
  
  
  template <bool build_nss,
            bool build_lyndon,
            typename ctx_type,
            typename index_type>
  xss_always_inline static void
  pss_array_amortized_lookahead(ctx_type& ctx,
                                const index_type j,
                                index_type& i,
                                index_type max_lce,
                                const index_type distance) {
    const index_type anchor = get_anchor(&(ctx.text[i]), max_lce);
    // copy NSS values up to anchor
    for (index_type k = 1; k < anchor; ++k) {
      ctx.array[i + k] = ctx.array[j + k] + distance;
      if constexpr (build_nss)
        ctx.aux[i + k] = ctx.aux[j + k] + distance;
      if constexpr (build_lyndon)
        ctx.aux[i + k] = ctx.aux[j + k];
    }
    i += anchor - 1;
  }

  template <typename ctx_type, typename index_type>
  xss_always_inline static void
  nss_array_amortized_lookahead(ctx_type& ctx,
                                const index_type j,
                                index_type& i,
                                index_type max_lce,
                                const index_type distance) {

    const index_type anchor = get_anchor(&(ctx.text[i]), max_lce);
    index_type next_pss = i;
    // copy NSS values up to anchor
    for (index_type k = 1; k < anchor; ++k) {
      if (ctx.array[j + k] < j + anchor) {
        ctx.array[i + k] = ctx.array[j + k] + distance;
      } else {
        ctx.array[i + k] = next_pss;
        next_pss = i + k;
      }
    }
    i += anchor - 1;
  }

  template <typename ctx_type, typename index_type>
  xss_always_inline static void lyndon_array_amortized_lookahead(
      ctx_type& ctx, const index_type j, index_type& i, index_type max_lce) {

    const index_type anchor = get_anchor(&(ctx.text[i]), max_lce);
    index_type next_pss = i;
    // copy NSS values up to anchor
    for (index_type k = 1; k < anchor; ++k) {
      if (ctx.array[j + k] + j + k < j + anchor) {
        ctx.array[i + k] = ctx.array[j + k];
      } else {
        ctx.array[i + k] = next_pss;
        next_pss = i + k;
      }
    }
    i += anchor - 1;
  }
  
  
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
  
  
  template <typename ctx_type, typename index_type>
  xss_always_inline static void xss_array_find_pss(const ctx_type& ctx,
                                                   const index_type j,
                                                   const index_type i,
                                                   const index_type lce,
                                                   index_type& max_lce_j,
                                                   index_type& max_lce,
                                                   index_type& pss_of_i) {
    index_type upper = j;
    index_type upper_lce = lce;
    index_type lower = upper;
    index_type lower_lce = 0;

    while (ctx.text[upper + upper_lce] > ctx.text[i + upper_lce]) {
      if (xss_unlikely(lower == upper)) {
        for (index_type k = 0; k < upper_lce; ++k)
          lower = ctx.array[lower];
        lower_lce = ctx.get_lce.with_upper_bound(lower, i, upper_lce);
      } else {
        lower_lce =
            ctx.get_lce.with_both_bounds(lower, i, lower_lce, upper_lce);
      }
      if (xss_unlikely(lower_lce == upper_lce)) {
        upper = ctx.array[upper];
        upper_lce = ctx.get_lce.with_lower_bound(upper, i, upper_lce);
      } else
        break;
    }

    // if at this point we have (upper == lower), then we also have
    // text[upper + upper_lce] < text[i + upper_lce]
    if (ctx.text[upper + upper_lce] < ctx.text[i + upper_lce]) {
      // PSS of i is upper
      max_lce_j = pss_of_i = upper;
      max_lce = upper_lce;
    } else {
      // PSS of i lies between upper and lower (could be lower, but not upper)
      // we definitely have upper > lower
      index_type upper_idx = ctx.n - 1;
      index_type lower_idx = upper_idx;
      ctx.array[upper_idx] = upper;
      while (upper > lower) {
        ctx.array[--lower_idx] = ctx.array[upper];
        upper = ctx.array[upper];
      }
      upper = ctx.array[upper_idx];

      while (true) {
        // move lower until same LCE as upper
        lower_lce = ctx.get_lce.with_both_bounds(ctx.array[lower_idx], i,
                                                 lower_lce, upper_lce);
        while (lower_lce < upper_lce) {
          ++lower_idx;
          lower_lce = ctx.get_lce.with_both_bounds(ctx.array[lower_idx], i,
                                                   lower_lce, upper_lce);
        }

        if (lower_idx == upper_idx) {
          pss_of_i = ctx.array[lower_idx - 1];
          break;
        }

        --upper_idx;
        upper_lce =
            ctx.get_lce.with_lower_bound(ctx.array[upper_idx], i, upper_lce);

        if (ctx.text[ctx.array[upper_idx] + upper_lce] <
            ctx.text[i + upper_lce]) {
          pss_of_i = ctx.array[upper_idx];
          break;
        }
      }

      max_lce_j = ctx.array[upper_idx];
      max_lce = upper_lce;
    }
  }
  
  
  
  
  template <bool build_nss,
            bool build_lyndon,
            typename index_type,
            typename value_type>
  static auto
  pss_and_x_array(value_type const* const text,
                  index_type* const array,
                  index_type* const aux,
                  uint64_t const n,
                  uint64_t threshold) {

    static_assert(!(build_nss && build_lyndon));

    if constexpr (build_nss)
      warn_type_width<index_type>(n, "xss::pss_and_nss_array");
    else if constexpr (build_lyndon)
      warn_type_width<index_type>(n, "xss::pss_and_lyndon_array");
    else
      warn_type_width<index_type>(n, "xss::pss_array");

    fix_threshold(threshold);

    static_assert(std::is_unsigned<index_type>::value);
    memset(array, 0, n * sizeof(index_type));
    if constexpr (build_nss || build_lyndon) {
      memset(array, 0, n * sizeof(index_type));
    }

    array_context_type<index_type, value_type> ctx{text, array, (index_type) n, aux};

    array[0] = 0; // will be overwritten with n later
    if constexpr (build_nss || build_lyndon) {
      aux[0] = n - 1;
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
  }

} // namespace xssinternal

template <typename index_type, typename value_type>
static auto pss_array(value_type const* const text,
                      index_type* const pss,
                      uint64_t const n,
                      uint64_t threshold) {
  return xssinternal::pss_and_x_array<false, false>(
      text, pss, (index_type*) nullptr, n, threshold);
}

template <typename index_type, typename value_type>
static auto
pss_and_nss_array(value_type const* const text,
                  index_type* const pss,
                  index_type* const lyndon,
                  uint64_t const n,
                  uint64_t threshold) {
  return xssinternal::pss_and_x_array<true, false>(text, pss, lyndon, n,
                                                threshold);
}

template <typename index_type, typename value_type>
static auto
pss_and_lyndon_array(value_type const* const text,
                     index_type* const pss,
                     index_type* const lyndon,
                     uint64_t const n,
                     uint64_t threshold) {
  return xssinternal::pss_and_x_array<false, true>(text, pss, lyndon, n,
                                                threshold);
}

template <typename index_type, typename value_type>
static void nss_array(value_type const* const text,
                      index_type* const array,
                      uint64_t const n,
                      uint64_t threshold) {
  using namespace xssinternal;
  warn_type_width<index_type>(n, "xss::nss_array");
  fix_threshold(threshold);

  static_assert(std::is_unsigned<index_type>::value);
  memset(array, 0, n * sizeof(index_type));

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
}

template <typename index_type, typename value_type>
static void lyndon_array(value_type const* const text,
                         index_type* const array,
                         uint64_t const n,
                         uint64_t threshold) {
  using namespace xssinternal;
  warn_type_width<index_type>(n, "xss::lyndon_array");
  fix_threshold(threshold);

  static_assert(std::is_unsigned<index_type>::value);
  memset(array, 0, n * sizeof(index_type));

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
}
