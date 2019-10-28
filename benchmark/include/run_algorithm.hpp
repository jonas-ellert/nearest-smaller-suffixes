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

#include <time_measure.hpp>

template <typename runner_type, typename teardown_type>
void run_generic(const std::string algo,
                 const std::string info,
                 const uint64_t n,
                 const uint64_t runs,
                 const double out_bpn,
                 runner_type& runner,
                 teardown_type& teardown) {

  std::cout << "RESULT algo=" << algo << " " << info << " runs=" << runs
            << " n=" << n << " " << std::flush;

  std::pair<uint64_t, uint64_t> time_mem = get_time_mem(runner, teardown, runs);

  const uint64_t total_memory = time_mem.second + n;
  const int64_t additional_memory = time_mem.second - ((n * out_bpn) / 8);

  std::cout << "median_time=" << time_mem.first
            << " total_memory=" << total_memory
            << " additional_memory=" << additional_memory << " ";

  auto mibs = (n / 1024.0 / 1024.0) / (time_mem.first / 1000.0);
  auto bpn = (8.0 * total_memory) / n;
  auto additional_bpn = (8.0 * additional_memory) / n;

  std::cout << "mibs=" << mibs << " bpn=" << bpn
            << " additional_bpn=" << additional_bpn << std::endl;
}

template <typename runner_type>
void run_generic(const std::string algo,
                 const std::string info,
                 const uint64_t n,
                 const uint64_t runs,
                 const double out_bpn,
                 runner_type& runner) {
  auto teardown = []() {};
  run_generic(algo, info, n, runs, out_bpn, runner, teardown);
}
