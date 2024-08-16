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

#include <utility>
#include <vector>

constexpr static uint8_t test_gen_sentinel = '\0';
using vec_type = std::vector<uint8_t>;


namespace xss {

constexpr uint64_t PSS = 0;
constexpr uint64_t NSS = 1;
constexpr uint64_t LYNDON = 2;

template<uint64_t first_array, uint64_t second_array = first_array>
auto get(uint8_t const * const text, uint64_t const len) {
  
  static_assert(0 <= first_array && first_array <= 2);
  static_assert(0 <= second_array && second_array <= 2);
  static_assert(second_array == first_array || (first_array == PSS && second_array != PSS));
  
  if constexpr (first_array != second_array) {
    std::pair<std::vector<uint32_t>, std::vector<uint32_t>> result 
      { std::vector<uint32_t>(len), std::vector<uint32_t>(len) };
      
    if constexpr (second_array == NSS) 
      pss_and_nss_array(text, result.first.data(), result.second.data(), len);
    else 
      pss_and_lyndon_array(text, result.first.data(), result.second.data(), len);
      
    return result;
  }
  else {
    std::vector<uint32_t> result(len);
    
    if constexpr(second_array == PSS)
      pss_array(text, result.data(), len);
    else if constexpr(second_array == NSS)
      nss_array(text, result.data(), len);
    else 
      lyndon_array(text, result.data(), len);
    
    return result;
  }
}



}
