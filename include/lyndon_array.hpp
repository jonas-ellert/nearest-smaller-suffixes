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


// INTERFACE:
template <typename index_type, typename value_type>
static void lyndon_array_nosentinels(value_type const* const text,
                                     index_type* const array,
                                     uint64_t const n,
                                     uint64_t threshold = 128);
                         
                         
// INTERFACE:
template <typename index_type, typename value_type>
static void lyndon_array_sentinels(value_type const* const text,
                                   index_type* const array,
                                   uint64_t const n,
                                   uint64_t threshold = 128);


// IMPLEMENTATION

#define xss_always_inline __attribute__((always_inline)) inline
#define xss_likely(x) __builtin_expect(!!(x), 1)
#define xss_unlikely(x) __builtin_expect(!!(x), 0)

namespace xssinternal {
  
template <typename text_type, typename array_type>
static void lyndon_array_internal(text_type const &text,
                                  array_type &array,
                                  uint64_t const n,
                                  uint64_t threshold);

template <typename value_type>
struct text_sentinel_wrapper {
  value_type const * const text;
  size_t const n;
  size_t const offset;
  
  text_sentinel_wrapper(value_type const * const text, size_t const n, size_t offset = 0) : text(text), n(n), offset(offset) {}
  
  xss_always_inline int64_t operator[](size_t const i) const {
    if (xss_likely(0 < i && i <= n)) {
      return textshift[i];
    }
    else {
      return std::numeric_limits<int64_t>::min();
    }
  }
  
  text_sentinel_wrapper operator+(size_t const add) const {
    return text_sentinel_wrapper {text, n, offset + add};
  }
  
 private:
  value_type const * const textshift = text - 1 + offset;
};

template <typename index_type>
struct array_sentinel_wrapper {
  index_type * const array;
  index_type left_sentinel;
  index_type right_sentinel;
  size_t const n;
  
  array_sentinel_wrapper(index_type * const array, size_t const n) : array(array), n(n) {}
  
  xss_always_inline index_type& operator[](size_t const i) {
    if (xss_likely(0 < i && i <= n)) {
      return arrayshift[i];
    }
    else {
      return (i == 0) ? left_sentinel : right_sentinel;
    }
  }
  
 private:
  index_type * const arrayshift = array - 1 ;
};



}

template <typename index_type, typename value_type>
static void lyndon_array_nosentinels(value_type const* const text,
                                     index_type* const array,
                                     uint64_t const n,
                                     uint64_t threshold) {
                           
  xssinternal::text_sentinel_wrapper<value_type> wrap_text(text, n);
  xssinternal::array_sentinel_wrapper<index_type> wrap_array(array, n);
  xssinternal::lyndon_array_internal(wrap_text, wrap_array, n + 2, threshold);
}


template <typename index_type, typename value_type>
static void lyndon_array_sentinels(value_type const* const text,
                                   index_type* const array,
                                   uint64_t const n,
                                   uint64_t threshold) {
  xssinternal::lyndon_array_internal(text, array, n, threshold);
}


namespace xssinternal {
  
  // can never be below 8
  constexpr static uint64_t MIN_THRESHOLD = 8;

  inline static void fix_threshold(uint64_t& threshold) {
    threshold = std::max(threshold, MIN_THRESHOLD);
  }  
  
  template <typename text_type>
  struct lce_type {
    text_type const &text;

    xss_always_inline size_t without_bounds(const size_t l,
                                            const size_t r,
                                            size_t lce = 0) const {
      while (text[l + lce] == text[r + lce])
        ++lce;
      return lce;
    }

    xss_always_inline size_t
    with_both_bounds(const size_t l,
                     const size_t r,
                     size_t lower,
                     const size_t upper) const {
      while (lower < upper && text[l + lower] == text[r + lower])
        ++lower;
      return lower;
    }

    xss_always_inline size_t with_upper_bound(
        const size_t l, const size_t r, const size_t upper) const {
      return with_both_bounds(l, r, 0, upper);
    }

    xss_always_inline size_t with_lower_bound(
        const size_t l, const size_t r, const size_t lower) const {
      return without_bounds(l, r, lower);
    }
  };
  
  
  template <typename text_type, typename array_type>
  struct array_context_type {

    text_type const &text;
    array_type &array;
    size_t const n;

    const lce_type<text_type> get_lce = lce_type<text_type>{text};
  };
  
  // DUVAL
  template <typename text_type>
  xss_always_inline static std::pair<uint64_t, uint64_t>
  is_extended_lyndon_run(text_type const &text, const uint64_t n) {
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
  
  template <typename text_type>
  xss_always_inline size_t get_anchor(text_type const &lce_str,
                                      const size_t lce_len) {

    const size_t ell = lce_len >> 2;

    // check if gamm_ell is an extended lyndon run
    const auto duval = is_extended_lyndon_run(lce_str + ell, lce_len - ell);

    // try to extend the lyndon run as far as possible to the left
    if (duval.first > 0) {
      const size_t period = duval.first;
      const auto repetition_eq = [&](const size_t l, const size_t r) {
        for (size_t k = 0; k < period; ++k)
          if (lce_str[l + k] != lce_str[r + k])
            return false;
        return true;
      };
      int64_t lhs = ell + duval.second - period;
      while (lhs >= 0 && repetition_eq(lhs, lhs + period)) {
        lhs -= period;
      }
      return std::min(ell, (size_t)(lhs + (period << 1)));
    } else {
      return ell;
    }
  }

  template <typename ctx_type>
  xss_always_inline static void lyndon_array_amortized_lookahead(
      ctx_type& ctx, const size_t j, size_t& i, size_t max_lce) {

    const size_t anchor = get_anchor(ctx.text + i, max_lce);
    size_t next_pss = i;
    // copy NSS values up to anchor
    for (size_t k = 1; k < anchor; ++k) {
      if (ctx.array[j + k] + j + k < j + anchor) {
        ctx.array[i + k] = ctx.array[j + k];
      } else {
        ctx.array[i + k] = next_pss;
        next_pss = i + k;
      }
    }
    i += anchor - 1;
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
  
  template <typename text_type, typename array_type>
  static void lyndon_array_internal(text_type const &text,
                                    array_type &array,
                                    uint64_t const n,
                                    uint64_t threshold) {
    using namespace xssinternal;
    fix_threshold(threshold);
    
    for (size_t i = 0; i < n; ++i) {
      array[i] = 0;
    }

    array_context_type<text_type, array_type> ctx{text, array, n};

    array[0] = 0; // will be overwritten with n - 1 later

    size_t j, lce;
    for (size_t i = 1; i < n - 1; ++i) {
      j = i - 1;
      lce = ctx.get_lce.without_bounds(j, i);

      if (xss_likely(lce < threshold)) {
        while (text[j + lce] > text[i + lce]) {
          size_t next_j = array[j];
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

      size_t max_lce, max_lce_j, pss_of_i;
      xss_array_find_pss(ctx, j, i, lce, max_lce_j, max_lce, pss_of_i);

      while (j > pss_of_i) {
        size_t next_j = array[j];
        array[j] = i - j;
        j = next_j;
      }
      array[i] = pss_of_i;

      const size_t distance = i - max_lce_j;
      if (xss_unlikely(max_lce >= 2 * distance))
        lyndon_array_run_extension(ctx, max_lce_j, i, max_lce, distance);
      else
        lyndon_array_amortized_lookahead(ctx, max_lce_j, i, max_lce);
    }

    // PROCESS ELEMENTS WITHOUT NSS
    j = n - 2;
    while (j > 0) {
      size_t next_j = array[j];
      array[j] = n - j - 1;
      j = next_j;
    }

    array[0] = n - 1;
    array[n - 1] = 1;
  }

} // namespace xssinternal


