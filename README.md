# Nearest Smaller Suffixes

This repository provides a lightweight header-only library for data structures that answer nearest smaller suffix queries. If you want to include it in your own project, refer to https://github.com/jonas-ellert/nearest-smaller-suffixes-mwe for a minimal working example. **Note that currently only GCC is supported.**

## Computing the NSS or PSS Array

To compute the NSS or PSS array, simply use the following syntax. **Note that the algorithms currently only work if both the first and the last character are smaller than all other characters in the input text.**

```c++
std::string text = "$northamerica$";
auto text_ptr = text.data();
auto n = text.size();

// use uint64_t if n is larger than std::numeric_limits<uint32_t>::max();
std::vector<uint32_t> nss = xss::nss_array<uint32_t>(text_ptr, n);
std::vector<uint32_t> pss = xss::pss_array<uint32_t>(text_ptr, n);
```

The succinct representation of the PSS array can be obtained as follows:

```c++
xss::bit_vector pss_tree = xss::pss_tree(text_ptr, n);
```

The PSS tree only needs `2n + 2` bits of memory and can simulate access to both the NSS and the PSS array. If you include the Succinct Data Structures Library in your project (see https://github.com/simongog/sdsl-lite), then you can build and use a support data structure for constant time queries as follows:

```c++
sdsl::bit_vector bv(2*n + 2);
xss::pss_tree(text_ptr, n, bv.data());
xss::pss_tree_support_sdsl support(bv)

// run some queries
std::cout << "PSS of 5 is " << support.pss(5) << std::endl;
std::cout << "NSS of 5 is " << support.nss(5) << std::endl;
std::cout << "Longest Lyndon word at index 5 is " << support.lyndon(5) << std::endl;
```

## Running Benchmarks

You can also compile this project as a standalone benchmark tool. To clone the repository and run some tests, simply execute the following commands:

```
git clone https://github.com/jonas-ellert/nearest-smaller-suffixes.git
cd nearest-smaller-suffixes
mkdir build; cd build
cmake ..
make check
```

Running benchmarks is easy. We provide the following algorithms (the 32 bit structures are also available as 64 bit versions):

* `pss-tree-plain`: Builds the PSS tree without the support data structure
* `pss-tree-support`: Builds the PSS tree with the support data structure
* `lyndon-array32`: Builds the Lyndon array
* `nss-array32`: Builds the NSS array
* `pss-array32`: Builds the PSS array
* `lyndon-isa-nsv32`: Builds the Lyndon array by computing the NSV array on the inverse suffix array
* `divsufsort32`: Builds the suffix array

The command below runs all algorithms except for `nss-array32` and `nss-array64`. The input text is the prefix of length `l=1GiB` of the file `f=/data_sets/dna.txt`. Each algorithms is executed `r=5` times, and the median time determines the final result.

```
make benchmark
./benchmark/src/benchmark -f /data_sets/dna.txt -r 5 -l 1GiB --not-contains nss-array
```
