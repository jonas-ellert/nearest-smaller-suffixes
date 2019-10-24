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
}
