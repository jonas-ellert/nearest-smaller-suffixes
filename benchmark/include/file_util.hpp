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

#include <algorithm>
#include <fstream>
#include <vector>

#include <si_units.hpp>

static uint8_t standardize(std::vector<uint8_t>& vector) {

  std::vector<bool> char_occurs(256, false);
  for (uint64_t i = 1; i < vector.size() - 1; ++i) {
    char_occurs[vector[i]] = true;
  }

  uint64_t sigma = 0;
  for (const auto b : char_occurs) {
    sigma += b ? 1 : 0;
  }
  std::cout << "[STANDARDIZE]         Alphabet size: sigma=" << sigma << "."
            << std::endl;

  if (char_occurs[0]) {
    uint64_t increase = 1;
    if (sigma == 256) {
      std::cerr << "[STANDARDIZE ERROR]   Cannot add sentinels (no unused "
                   "characters, sigma=256)."
                << std::endl;
      std::cerr
          << "[STANDARDIZE WARNING] Replacing all null-characters with "
             "1-characters.\n"
          << "                      This may influence the resulting data "
             "structures."
          << std::endl;
      sigma = 255;
    } else {
      for (uint64_t i = 0; i < 256; ++i) {
        if (!char_occurs[i]) {
          increase = i;
          break;
        }
      }
      std::cout << "[STANDARDIZE]         Text contains null-characters, but "
                   "does not contain the character "
                << increase << "." << std::endl;
      std::cout << "[STANDARDIZE]         Incrementing all characters that are "
                   "smaller than "
                << increase
                << ". This does not influence the resulting data structures."
                << std::endl;
    }

    for (auto& character : vector)
      if (xss_unlikely(character < increase))
        ++character;
  }

  std::cout
      << "[STANDARDIZE]         Adding sentinels at beginning and end of text."
      << std::endl;
  vector[0] = '\0';
  vector[vector.size() - 1] = '\0';
  return sigma;
}

// adds sentinels
static std::vector<uint8_t> file_to_instance(const std::string& file_name,
                                             const uint64_t prefix_size,
                                             uint8_t& sigma) {
  std::ifstream stream(file_name.c_str(), std::ios::in | std::ios::binary);

  if (!stream) {
    std::cerr << "File " << file_name << " not found.\n";
    exit(EXIT_FAILURE);
  }

  stream.seekg(0, std::ios::end);
  uint64_t size_in_characters = stream.tellg();
  stream.seekg(0);

  if (prefix_size > 0) {
    size_in_characters = std::min(prefix_size, size_in_characters);
  }
  uint64_t size_in_bytes = size_in_characters;

  // +2 sentinels
  std::vector<uint8_t> result(size_in_characters + 2);
  stream.read(reinterpret_cast<char*>(&(result.data()[1])), size_in_bytes);
  stream.close();

  std::cout << "Finished reading file \"" << file_name << "\"." << std::endl;
  std::cout << "Size (w/o sentinels): "
            << "[" << size_in_characters << " characters] = "
            << ((size_in_bytes > 1023)
                    ? ("[" + std::to_string(size_in_bytes) + " bytes] = ")
                    : "")
            << "[" << to_SI_string(size_in_bytes) << "]" << std::endl;
  sigma = standardize(result);
  return result;
}

[[maybe_unused]] static std::vector<uint8_t>
file_to_instance(const std::string& file_name, const uint64_t prefix_size) {
  uint8_t dummy;
  return file_to_instance(file_name, prefix_size, dummy);
}