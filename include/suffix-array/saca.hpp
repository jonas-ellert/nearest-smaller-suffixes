#pragma once

#include "../array/sequential/algorithm.hpp"
#include "robin_hood.h"
#include "sequential/bingmann_radix_sort.hpp"
#include <algorithm>
#include <chrono>
#include <iomanip>
#include <sstream>

namespace xss {

using uint128_t = unsigned __int128;

struct timer {
  static std::string dblstr(double const& dbl) {
    std::stringstream str;
    str << std::setprecision(2) << std::fixed << dbl;
    return str.str();
  }

  decltype(std::chrono::high_resolution_clock::now()) begin_;
  decltype(std::chrono::high_resolution_clock::now()) end_;

  void begin() {
    begin_ = std::chrono::high_resolution_clock::now();
  }

  void end() {
    end_ = std::chrono::high_resolution_clock::now();
  }

  uint64_t millis() const {
    return std::chrono::duration_cast<std::chrono::milliseconds>(end_ - begin_)
        .count();
  }

  double seconds() const {
    return millis() / 1000.0;
  }

  double mibs(uint64_t bytes) const {
    return bytes / seconds() / 1024.0 / 1024.0;
  }

  std::string throughput_string(uint64_t bytes) const {
    return "[throughput=" + dblstr(mibs(bytes)) + "mibs in " +
           dblstr(seconds()) + "s]";
  }
};

template <typename index_type = uint32_t, typename value_type>
static auto new_saca(const value_type* text, const uint64_t n) {
  static_assert(std::is_unsigned<value_type>::value);
  static_assert(std::is_unsigned<index_type>::value);
  static_assert(sizeof(value_type) == 1);
  static_assert(sizeof(index_type) == 4);
  //  using unordered_map64 = std::unordered_map<uint64_t, index_type>;
  using unordered_map64 = robin_hood::unordered_flat_map<uint64_t, index_type>;

  constexpr index_type DELETED = std::numeric_limits<index_type>::max();
  constexpr index_type MAX_LYN = 8;

  std::cout << "\n\nStart SACA..." << std::endl;

  timer quick_time;
  timer total_time;

  total_time.begin();

  quick_time.begin();
  auto pss_and_lyndon = pss_and_lyndon_array(text, n);
  auto& pss = pss_and_lyndon.first;
  auto& lyndon = pss_and_lyndon.second;
  quick_time.end();
  std::cout << "Computed PSS + lyndon " << quick_time.throughput_string(n)
            << std::endl;

  quick_time.begin();

  std::vector<index_type> to_sort_idx;
  std::vector<index_type> to_sort_lyn;
  std::vector<int> first_occ(n);

  {
    constexpr index_type buckets = std::pow(2, 16);
    std::vector<index_type> first_occ_short(buckets);
    unordered_map64 first_occ_med;
    unordered_map64 first_occ_long;
    to_sort_idx.emplace_back(0);
    to_sort_lyn.emplace_back(1);
    first_occ[n - 1] = 0;
    first_occ[0] = -2;
    lyndon[0] = lyndon[n - 1] = 0;
    for (index_type i = 1; i < n - 1; ++i) {
      index_type const lyn = lyndon[i];
      lyndon[i] = 0;
      index_type* f;
      if (lyn == 1) {
        f = &(first_occ_short[index_type(text[i]) << 8]);
      } else if (lyn == 2) {
        f = &(first_occ_short[index_type(text[i]) << 8 | text[i + 1]]);
      } else if (lyn <= MAX_LYN) {
        uint64_t lyndon_word = 0ULL;
        for (index_type j = 0; j < lyn; ++j) {
          lyndon_word <<= 8;
          lyndon_word |= uint64_t(text[i + j]);
        }
        f = &(first_occ_med[lyndon_word]);
      } else {
        to_sort_idx.emplace_back(i);
        to_sort_lyn.emplace_back(lyn);
        first_occ[i] = -1;
        continue;
      }

      index_type& first = *f;
      if (first == 0) {
        first = i;
        to_sort_idx.emplace_back(i);
        to_sort_lyn.emplace_back(lyn);
        first_occ[i] = -1;
      } else {
        first_occ[i] = first;
        --first_occ[first];
        for (index_type j = 1; j < lyn; ++j) {
          if (first_occ[first + j] < 0) {
            first_occ[i + j] = first + j;
            --first_occ[first + j];
          } else {
            first_occ[i + j] = first_occ[first + j];
            --first_occ[first_occ[first + j]];
          }
        }
        i += lyn - 1;
      }
    }
  }
  quick_time.end();

  std::cout << "Computed to_sort " << quick_time.throughput_string(n) << ": "
            << to_sort_idx.size() << std::endl;

  quick_time.begin();
  modified_bingmann::bingmann_msd_lyndon(text, to_sort_idx.data(),
                                         to_sort_lyn.data(),
                                         (index_type) to_sort_idx.size());
  quick_time.end();
  std::cout << "Sorted " << quick_time.throughput_string(n) << std::endl;

  auto substreq = [&text](index_type const& a, index_type const& b,
                          index_type const& len) {
    for (index_type i = 0; i < len; ++i)
      if (text[a + i] != text[b + i])
        return false;
    return true;
  };

  quick_time.begin();
  index_type current_group_lyn = 1;

  to_sort_idx[0] = 0;
  to_sort_lyn[0] = 2;

  index_type write_b = 1;
  for (index_type b = 1; b < to_sort_idx.size(); ++b) {
    index_type current_i = to_sort_idx[b];
    index_type current_lyn = to_sort_lyn[b];
    if (current_lyn <= MAX_LYN) {
      // small group
      current_group_lyn = current_lyn;
      to_sort_idx[write_b] = current_i;
      to_sort_lyn[write_b] = -first_occ[current_i];
      ++write_b;
    } else if (current_lyn == current_group_lyn &&
               substreq(to_sort_idx[b], to_sort_idx[write_b - 1],
                        current_lyn)) {
      // extend large group
      first_occ[current_i] = to_sort_idx[write_b - 1];
      ++to_sort_lyn[write_b - 1];
    } else {
      // new large group
      current_group_lyn = current_lyn;
      to_sort_idx[write_b] = current_i;
      to_sort_lyn[write_b] = 1;
      ++write_b;
    }
  }
  index_type const number_of_groups = write_b;
  quick_time.end();

  std::cout << "Computed group sizes " << quick_time.throughput_string(n)
            << ". Groups: " << number_of_groups << std::endl;

  quick_time.begin();
  index_type in_smaller_groups = 0;
  for (index_type b = 0; b < number_of_groups; ++b) {
    index_type current_i = to_sort_idx[b];
    index_type in_current_group = to_sort_lyn[b];
    index_type counter_idx = in_smaller_groups + in_current_group - 1;

    // write negative pointer to end of group
    first_occ[current_i] = -counter_idx;
    // write group counter
    lyndon[counter_idx] = in_smaller_groups;
    in_smaller_groups += in_current_group;
  }
  quick_time.end();
  std::cout << "Computed end-links for first occs "
            << quick_time.throughput_string(n)
            << ". Sanity: " << -first_occ[to_sort_idx[number_of_groups - 1]] + 1
            << " = " << n << std::endl;

  quick_time.begin();
  for (index_type i = 0; i < n; ++i) {
    first_occ[i] =
        (first_occ[i] < 0) ? (-first_occ[i]) : (first_occ[first_occ[i]]);
  }
  quick_time.end();
  std::cout << "Computed end-links for all other positions "
            << quick_time.throughput_string(n) << std::endl;

  total_time.end();
  std::cout << "Ready for phase 2 " << total_time.throughput_string(n)
            << std::endl;


  // and now finally phase 2
  auto &isa = first_occ;
  auto &sa = lyndon;
  quick_time.begin();
  sa[0] = n-1;
  for (index_type i = 0; i < n; ++i) {
    // in case we leave the text
    if (sa[i] == 0) { continue; }

    // put j into the correct position
    index_type j = sa[i]-1;
    while (true) {
      index_type cnt = sa[isa[j]];
      if (isa[j] == 0) { break; }

      // increment counter and mark j as placed
      ++sa[isa[j]];
      isa[j] = 0;

      // place j and get new value for j
      sa[cnt] = j;
      if (j == 0) { break; }
      j = pss[j];
    }
  }
  quick_time.end();

  std::cout << "Phase 2 done " << quick_time.throughput_string(n)
            << std::endl;

  return std::move(sa);
} // namespace xss

} // namespace xss