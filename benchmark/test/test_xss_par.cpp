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

#include "util/check_array.hpp"
#include <xss.hpp>

#include "strings/test_lookahead.hpp"
#include "strings/test_manual.hpp"
#include "strings/test_overlap.hpp"
#include "strings/test_random.hpp"
#include "strings/test_runs.hpp"

using check_array_type = check_array<true, true>;

enum structure_type { array, tree };
constexpr structure_type TEST_ARRAY = structure_type::array;
constexpr structure_type TEST_TREE = structure_type::tree;

template <uint64_t log_count = 16,
          structure_type stype,
          typename instance_collection>
static void instance_tests(instance_collection&& instances) {
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
      for (int i = 1; i < 17; i <<= 2) { // 1, 4, 16
        auto pss_nss =
            xss::pss_and_nss_array_parallel<uint32_t>(t.data(), t.size(), i, 0);
        check_array_type::check_pss(t, pss_nss.first);
        check_array_type::check_nss(t, pss_nss.second);
        //        auto pss_lyndon =
        //        xss::pss_and_lyndon_array_parallel<uint32_t>(t.data(),
        //        t.size(), i); check_array_type::check_pss(t,
        //        pss_lyndon.first); check_array_type::check_nss_vs_lyndon(t,
        //        pss_nss.second, pss_lyndon.second);
      }
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
