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
#include "util/check_array.hpp"

constexpr static uint64_t min_n = 64;
constexpr static uint64_t max_n = 128ULL * 1024;
constexpr static uint64_t run_max_n = 1024;
constexpr static uint64_t run_of_runs_min_rep = 2;
constexpr static uint64_t run_of_runs_max_rep = 10;
constexpr static uint64_t run_of_runs_min_pow_rep = 16;
constexpr static uint64_t run_of_runs_max_pow_rep = 1024;

using check_type = check_array<true, true>;

template <typename instance_collection>
static void instance_tests(instance_collection&& instances) {
  std::cout << "Number of instances: " << instances.size() << std::endl;
  const uint64_t logging_interval =
      (instances.size() > 32) ? (instances.size() / 16) : 1;

  auto check_instance = [&](const decltype(instances[0])& t, uint64_t verbose) {
    if (verbose)
      std::cout << "Testing instance " << verbose << " (of length " << t.size()
                << "): " << std::flush;

    check_type::check_nss(t, xss::nss_array(t.data(), t.size()));
    check_type::check_pss(t, xss::nss_array(t.data(), t.size()));
    if (verbose)
      std::cout << "Forwards done. " << std::flush;

    std::reverse(t.begin(), t.end());
    check_type::check_nss(t, xss::nss_array(t.data(), t.size()));
    check_type::check_pss(t, xss::nss_array(t.data(), t.size()));
    if (verbose)
      std::cout << "Backwards done." << std::endl;
  };

  for (uint64_t i = 0; i < instances.size();) {
    check_instance(instances[i], i);
    for (uint64_t j = 1; j < logging_interval && i + j < instances.size();
         ++j) {
      check_instance(instances[i + j], 0);
    }
    i += logging_interval;
  }
}

TEST(arrays, hand_selected) {
  std::cout << "Testing XSS with hand selected instances "
            << "(stuff that caused problems in the past)." << std::endl;
  instance_tests(get_instances_for_manual_test());
}

TEST(arrays, lookahead) {
  std::cout << "Testing XSS with hand selected instances "
            << "(cover all lookahead cases)." << std::endl;
  instance_tests(get_instances_for_lookahead(512));
}

TEST(dummy, dummy) {
  std::cout << "Hello Test!" << std::endl;
  std::string teststr = "$northamerica$";
  auto strptr = teststr.data();
  std::cout << teststr << " --- Length: " << teststr.size() << std::endl;
  auto p = xss::pss_array(strptr, teststr.size());
  std::cout << "PSS Array: ";
  for (auto val : p) {
    std::cout << val << ", ";
  }
  std::cout << std::endl;
  auto n = xss::nss_array(strptr, teststr.size());
  std::cout << "NSS Array: ";
  for (auto val : n) {
    std::cout << val << ", ";
  }
  std::cout << std::endl;
  auto l = xss::lyndon_array(strptr, teststr.size());
  std::cout << "Lyn Array: ";
  for (auto val : l) {
    std::cout << val << ", ";
  }
  std::cout << std::endl;

  check_array<>::check_nss(teststr, n);
  check_array<>::check_pss(teststr, p);
}
