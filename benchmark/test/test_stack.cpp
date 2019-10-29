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

#include "util/random.hpp"
#include <ds/tree/stack.hpp>
#include <gtest/gtest.h>

constexpr static uint64_t number_of_elements = 1024ULL * 1024;

template <typename stack_type>
static void test_random(stack_type& stack) {
  auto rng = random_number_generator(1, 256);
  std::vector<uint64_t> elements(number_of_elements);
  elements[0] = 0;
  for (uint64_t i = 1; i < number_of_elements; ++i) {
    elements[i] = elements[i - 1] + rng();
    stack.push(elements[i]);
  }

  for (uint64_t i = number_of_elements; i > 1;) {
    EXPECT_EQ(stack.top(), elements[--i]);
    stack.pop();
  }
  EXPECT_EQ(stack.top(), (uint64_t) 0);
}

TEST(stack, unbuffered_random) {
  std::cout << "Testing telescope stack with " << number_of_elements
            << " random elements." << std::endl;
  telescope_stack stack;
  test_random(stack);
}

TEST(stack, buffered_random) {
  std::cout << "Testing telescope stack with " << number_of_elements
            << " random elements." << std::endl;
  telescope_stack_buffered<uint64_t> stack(number_of_elements);
  test_random(stack);
}

TEST(dummy, dummy) {
  telescope_stack stack;
  stack.push(15);
  stack.push(29);
  stack.push(1024);
}
