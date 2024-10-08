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

#include <gtest/gtest.h>
#include <xss.hpp>

#include "strings/test_lookahead.hpp"
#include "strings/test_manual.hpp"
#include "strings/test_overlap.hpp"
#include "strings/test_random.hpp"
#include "strings/test_runs.hpp"
#include "util/check_array.hpp"
#include "util/check_tree.hpp"
#include <sdsl/bit_vectors.hpp>

constexpr static uint64_t min_n = 64;
constexpr static uint64_t max_n = 128ULL * 1024;
constexpr static uint64_t run_max_n = 1024;
constexpr static uint64_t run_of_runs_min_rep = 2;
constexpr static uint64_t run_of_runs_max_rep = 10;
constexpr static uint64_t run_of_runs_min_pow_rep = 16;
constexpr static uint64_t run_of_runs_max_pow_rep = 1024;

using check_array_type = check_array<true, true>;
using check_tree_type = check_tree<true, true>;

enum structure_type { array, tree };
constexpr structure_type TEST_ARRAY = structure_type::array;
constexpr structure_type TEST_TREE = structure_type::tree;

template <uint64_t log_count = 16,
          structure_type stype,
          typename instance_collection>
static void instance_tests(instance_collection&& instances) {
  using check_type =
      typename std::conditional<stype == TEST_ARRAY, check_array_type,
                                check_tree_type>::type;
  std::cout << "Number of instances: " << instances.size() << std::endl;
  const uint64_t logging_interval =
      (instances.size() + log_count - 2) / (log_count - 1);

  auto check_instance = [&](const decltype(instances[0])& t, uint64_t verbose) {
    if (verbose) {
      std::vector<bool> used_chars(256, false);
      for (auto c : t)
        used_chars[c] = true;
      uint64_t sigma = 0;
      for (auto b : used_chars)
        if (b)
          ++sigma;
      std::cout << "Testing instance " << verbose << " (of length " << t.size()
                << " and alphabet size " << (sigma - 1) << ")." << std::endl;
    }

    if constexpr (stype == TEST_ARRAY) {
      auto pss_and_nss = xss::get<xss::PSS, xss::NSS>(t.data(), t.size());
      auto pss_and_lyndon = xss::get<xss::PSS, xss::LYNDON>(t.data(), t.size());
      auto nss = xss::get<xss::NSS>(t.data(), t.size());
      auto lyndon = xss::get<xss::LYNDON>(t.data(), t.size());
      check_type::check_pss(t, pss_and_nss.first);
      check_type::check_nss(t, nss);
      check_type::check_nss_vs_lyndon(t, nss, lyndon);
      check_type::check_nss_vs_lyndon(t, pss_and_nss.second, lyndon);
      check_type::check_nss_vs_lyndon(t, nss, pss_and_lyndon.second);

      std::reverse(t.begin(), t.end());

      pss_and_nss = xss::get<xss::PSS, xss::NSS>(t.data(), t.size());
      pss_and_lyndon = xss::get<xss::PSS, xss::LYNDON>(t.data(), t.size());
      nss = xss::get<xss::NSS>(t.data(), t.size());
      lyndon = xss::get<xss::LYNDON>(t.data(), t.size());
      check_type::check_pss(t, pss_and_nss.first);
      check_type::check_nss(t, nss);
      check_type::check_nss_vs_lyndon(t, nss, lyndon);
      check_type::check_nss_vs_lyndon(t, pss_and_nss.second, lyndon);
      check_type::check_nss_vs_lyndon(t, nss, pss_and_lyndon.second);
    } else {
      sdsl::bit_vector bv(2 * t.size() + 2);
      xss::pss_tree(t.data(), bv.data(), t.size());
      check_type::check_bps(t, bv);
      std::reverse(t.begin(), t.end());
      xss::pss_tree(t.data(), bv.data(), t.size());
      check_type::check_bps(t, bv);
    }
  };

  for (uint64_t i = 0; i < instances.size() - 1;) {
    check_instance(instances[i], i + 1);
    for (uint64_t j = 1; j < logging_interval && i + j < instances.size() - 1;
         ++j) {
      check_instance(instances[i + j], 0);
    }
    i += logging_interval;
  }
  check_instance(instances[instances.size() - 1], instances.size());
}

TEST(arrays, hand_selected) {
  std::cout << "Testing XSS with hand selected instances "
            << "(stuff that caused problems in the past)." << std::endl;
  instance_tests<32, TEST_ARRAY>(get_instances_for_manual_test());
}

TEST(arrays, overlap) {
  std::cout
      << "Testing XSS with instances that maximize overlaps (without runs)."
      << std::endl;
  instance_tests<8, TEST_ARRAY>(
      get_instances_for_overlap_test(128, 16, 1048576));
}

TEST(arrays, lookahead) {
  std::cout << "Testing XSS with hand selected instances "
            << "(cover all lookahead cases)." << std::endl;
  instance_tests<8, TEST_ARRAY>(get_instances_for_lookahead_test(512));
}

TEST(arrays, runs) {
  std::cout << "Testing XSS with runs of runs." << std::endl;
  instance_tests<8, TEST_ARRAY>(get_instances_for_run_of_runs_test(1048576));
}

TEST(arrays, random) {
  std::cout << "Testing XSS with random instances..." << std::endl;
  std::cout << "sigma: [2, 15], n: [1, 1023]" << std::endl;
  instance_tests<4, TEST_ARRAY>(
      get_instances_for_random_test(8192, 2, 15, 16, 1023));
  std::cout << "sigma: [16, 255], n: [1, 1023]" << std::endl;
  instance_tests<4, TEST_ARRAY>(
      get_instances_for_random_test(8192, 16, 255, 16, 1023));
  std::cout << "sigma: [2, 15], n: [1024, 16383]" << std::endl;
  instance_tests<4, TEST_ARRAY>(
      get_instances_for_random_test(2048, 2, 15, 1024, 16383));
  std::cout << "sigma: [16, 255], n: [1024, 16383]" << std::endl;
  instance_tests<4, TEST_ARRAY>(
      get_instances_for_random_test(2048, 16, 255, 1024, 16383));
  std::cout << "sigma: [2, 15], n: [16384, 1048576]" << std::endl;
  instance_tests<4, TEST_ARRAY>(
      get_instances_for_random_test(128, 2, 15, 16384, 1048576));
  std::cout << "sigma: [16, 255], n: [16384, 1048576]" << std::endl;
  instance_tests<4, TEST_ARRAY>(
      get_instances_for_random_test(128, 16, 255, 16384, 1048576));
}

TEST(tree, hand_selected) {
  std::cout << "Testing XSS with hand selected instances "
            << "(stuff that caused problems in the past)." << std::endl;
  instance_tests<32, TEST_TREE>(get_instances_for_manual_test());
}

TEST(tree, overlap) {
  std::cout
      << "Testing XSS with instances that maximize overlaps (without runs)."
      << std::endl;
  instance_tests<8, TEST_TREE>(
      get_instances_for_overlap_test(128, 16, 1048576));
}

TEST(tree, lookahead) {
  std::cout << "Testing XSS with hand selected instances "
            << "(cover all lookahead cases)." << std::endl;
  instance_tests<8, TEST_TREE>(get_instances_for_lookahead_test(512));
}

TEST(tree, runs) {
  std::cout << "Testing XSS with runs of runs." << std::endl;
  instance_tests<8, TEST_TREE>(
      get_instances_for_run_of_runs_test(64ULL * 1024));
}

TEST(tree, random) {
  std::cout << "Testing XSS with random instances..." << std::endl;
  std::cout << "sigma: [2, 15], n: [1, 1023]" << std::endl;
  instance_tests<4, TEST_TREE>(
      get_instances_for_random_test(8192, 2, 15, 16, 1023));
  std::cout << "sigma: [16, 255], n: [1, 1023]" << std::endl;
  instance_tests<4, TEST_TREE>(
      get_instances_for_random_test(8192, 16, 255, 16, 1023));
  std::cout << "sigma: [2, 15], n: [1024, 16383]" << std::endl;
  instance_tests<4, TEST_TREE>(
      get_instances_for_random_test(2048, 2, 15, 1024, 16383));
  std::cout << "sigma: [16, 255], n: [1024, 16383]" << std::endl;
  instance_tests<4, TEST_TREE>(
      get_instances_for_random_test(2048, 16, 255, 1024, 16383));
  std::cout << "sigma: [2, 15], n: [16384, 1048576]" << std::endl;
  instance_tests<4, TEST_TREE>(
      get_instances_for_random_test(128, 2, 15, 16384, 1048576));
  std::cout << "sigma: [16, 255], n: [16384, 1048576]" << std::endl;
  instance_tests<4, TEST_TREE>(
      get_instances_for_random_test(128, 16, 255, 16384, 1048576));
}

TEST(dummy, dummy) {
  //  std::cout << "Hello Test!" << std::endl;
  //  std::string teststr = "$northamerica$";
  //  auto strptr = teststr.data();
  //  std::cout << teststr << " --- Length: " << teststr.size() << std::endl;
  //  auto p = xss::pss_array(strptr, teststr.size());
  //  std::cout << "PSS Array: ";
  //  for (auto val : p) {
  //    std::cout << val << ", ";
  //  }
  //  std::cout << std::endl;
  //  auto n = xss::nss_array(strptr, teststr.size());
  //  std::cout << "NSS Array: ";
  //  for (auto val : n) {
  //    std::cout << val << ", ";
  //  }
  //  std::cout << std::endl;
  //  auto l = xss::lyndon_array(strptr, teststr.size());
  //  std::cout << "Lyn Array: ";
  //  for (auto val : l) {
  //    std::cout << val << ", ";
  //  }
  //  std::cout << std::endl;
  //
  //  check_array<>::check_nss(teststr, n);
  //  check_array<>::check_pss(teststr, p);
  //
  //  auto bps = xss::pss_tree(strptr, teststr.size());
  //  std::cout << "PSS Tree: " << bps << std::endl;
  //  check_tree<>::check_bps(teststr, bps);
}
