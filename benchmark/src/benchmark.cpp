#include <xss_sdsl.hpp>

#include <divsufsort.h>
#include <divsufsort64.h>
#include <file_util.hpp>
#include <lyndon-isa-nsv.hpp>
#include <run_algorithm.hpp>
#include <sstream>
#include <tlx/cmdline_parser.hpp>

struct {
  std::vector<std::string> file_paths;
  uint64_t bytes_per_char = 1;
  uint64_t number_of_runs = 5;
  uint64_t prefix_size = 0;
  std::string contains = "";
  std::string not_contains = "";
  std::string list_of_cores = "1,2,4,8,16";
  bool list = false;

  bool matches_cores(const uint64_t cores) const {
    std::stringstream c(list_of_cores);
    while (c.good()) {
      std::string c_sub;
      getline(c, c_sub, ',');
      if (c_sub == std::to_string(cores))
        return true;
    }
    return false;
  }

  bool matches(const std::string algo) const {
    std::stringstream c(contains);
    while (c.good()) {
      std::string c_sub;
      getline(c, c_sub, ',');
      if (algo.find(c_sub) == std::string::npos)
        return false;
    }

    if (not_contains.size() == 0)
      return true;

    std::stringstream nc(not_contains);
    while (nc.good()) {
      std::string nc_sub;
      getline(nc, nc_sub, ',');
      if (algo.find(nc_sub) != std::string::npos)
        return false;
    }
    return true;
  }

} s;

int main(int argc, char const* argv[]) {
  tlx::CmdlineParser cp;
  cp.set_description("Nearest Smaller Suffix Construction");
  cp.set_author("Jonas Ellert <jonas.ellert@tu-dortmund.de>");

  cp.add_stringlist('f', "file", s.file_paths, "Path(s) to the text file(s).");

  cp.add_bytes('r', "runs", s.number_of_runs,
               "Number of repetitions of the algorithm (default = 5).");
  cp.add_bytes('l', "length", s.prefix_size,
               "Length of the prefix of the text that should be considered.");

  cp.add_string('\0', "contains", s.contains,
                "Only execute algorithms that contain at least one of the "
                "given strings (comma separated).");
  cp.add_string('\0', "not-contains", s.not_contains,
                "Only execute algorithms that contain none of the given "
                "strings (comma separated).");
  cp.add_string('\0', "threads", s.list_of_cores,
                "Execute parallel algorithms using the given number of "
                "OMP threads (multiple options possible; comma separated, e.g. "
                "\"1,2,4,8\").");

  cp.add_flag('\0', "list", s.list, "List the available algorithms.");

  if (!cp.process(argc, argv)) {
    return -1;
  }

  if (s.list) {
    std::cout << "Available algorithms:" << std::endl;
    std::cout << "    "
              << "lyndon-array" << std::endl;
    std::cout << "    "
              << "nss-array" << std::endl;
    std::cout << "    "
              << "pss-array" << std::endl;
    std::cout << "    "
              << "pss-and-lyndon-array" << std::endl;
    std::cout << "    "
              << "pss-and-nss-array" << std::endl;
    std::cout << "    "
              << "pss-tree" << std::endl;
    std::cout << "    "
              << "divsufsort" << std::endl;
    return 0;
  }

  for (auto file : s.file_paths) {
    uint8_t sigma = 0;
    std::vector<uint8_t> text_vec =
        file_to_instance(file, s.prefix_size, sigma);
    const std::string info =
        std::string("file=") + file + " sigma=" + std::to_string(sigma);

    if (s.matches("pss-tree-plain")) {
      xss::bit_vector bv(2 * text_vec.size() + 2);
      auto runner = [&]() {
        xss::pss_tree(text_vec.data(), bv.data(), text_vec.size());
      };
      auto teardown = [&]() { bv = xss::bit_vector(2 * text_vec.size() + 2); };
      run_generic("pss-tree-plain", info, text_vec.size() - 2, s.number_of_runs,
                  runner, teardown);
    }

    if (s.matches("pss-tree-support")) {
      sdsl::bit_vector bv(2 * text_vec.size() + 2);
      auto runner = [&]() {
        xss::pss_tree(text_vec.data(), bv.data(), text_vec.size());
        auto support = xss::pss_tree_support_sdsl(bv);
      };
      auto teardown = [&]() { bv = sdsl::bit_vector(2 * text_vec.size() + 2); };
      run_generic("pss-tree-support", info, text_vec.size() - 2,
                  s.number_of_runs, runner, teardown);
    }

    if (s.matches("lyndon-array32")) {
      std::vector<uint32_t> array(text_vec.size());
      auto runner = [&]() {
        xss::lyndon_array(text_vec.data(), array.data(), text_vec.size());
      };
      run_generic("lyndon-array32", info, text_vec.size() - 2, s.number_of_runs,
                  runner);
    }

    if (s.matches("nss-array32")) {
      std::vector<uint32_t> array(text_vec.size());
      auto runner = [&]() {
        xss::nss_array(text_vec.data(), array.data(), text_vec.size());
      };
      run_generic("nss-array32", info, text_vec.size() - 2, s.number_of_runs,
                  runner);
    }

    if (s.matches("pss-array32")) {
      std::vector<uint32_t> array(text_vec.size());
      auto runner = [&]() {
        xss::pss_array(text_vec.data(), array.data(), text_vec.size());
      };
      run_generic("pss-array32", info, text_vec.size() - 2, s.number_of_runs,
                  runner);
    }

    for (int p = 1; p < 1025; ++p) {
      if (s.matches("pss-array32-par") && s.matches_cores(p)) {
        std::vector<uint32_t> array(text_vec.size());
        auto runner = [&]() {
          xss::pss_array_parallel(text_vec.data(), array.data(),
                                  text_vec.size(), p);
        };
        run_generic("pss-array32-par", info + " threads=" + std::to_string(p),
                    text_vec.size() - 2, s.number_of_runs, runner);
      }
    }

    if (s.matches("pss-and-lyndon-array32")) {
      std::vector<uint32_t> array1(text_vec.size());
      std::vector<uint32_t> array2(text_vec.size());
      auto runner = [&]() {
        xss::pss_and_lyndon_array(text_vec.data(), array1.data(), array2.data(),
                                  text_vec.size());
      };
      run_generic("pss-and-lyndon-array32", info, text_vec.size() - 2,
                  s.number_of_runs, runner);
    }

    for (int p = 1; p < 1025; ++p) {
      if (s.matches("pss-and-lyndon-array32-par") && s.matches_cores(p)) {
        std::vector<uint32_t> array1(text_vec.size());
        std::vector<uint32_t> array2(text_vec.size());
        auto runner = [&]() {
          xss::pss_and_lyndon_array_parallel(text_vec.data(), array1.data(),
                                             array2.data(), text_vec.size(), p);
        };
        run_generic("pss-and-lyndon-array32-par",
                    info + " threads=" + std::to_string(p), text_vec.size() - 2,
                    s.number_of_runs, runner);
      }
    }

    if (s.matches("pss-and-nss-array32")) {
      std::vector<uint32_t> array1(text_vec.size());
      std::vector<uint32_t> array2(text_vec.size());
      auto runner = [&]() {
        xss::pss_and_nss_array(text_vec.data(), array1.data(), array2.data(),
                               text_vec.size());
      };
      run_generic("pss-and-nss-array32", info, text_vec.size() - 2,
                  s.number_of_runs, runner);
    }

    for (int p = 1; p < 1025; ++p) {
      if (s.matches("pss-and-nss-array32-par") && s.matches_cores(p)) {
        std::vector<uint32_t> array1(text_vec.size());
        std::vector<uint32_t> array2(text_vec.size());
        auto runner = [&]() {
          xss::pss_and_nss_array_parallel(text_vec.data(), array1.data(),
                                          array2.data(), text_vec.size(), p);
        };
        run_generic("pss-and-nss-array32-par",
                    info + " threads=" + std::to_string(p), text_vec.size() - 2,
                    s.number_of_runs, runner);
      }
    }

    if (s.matches("lyndon-isa-nsv32")) {
      std::vector<uint32_t> array(text_vec.size() - 1);
      auto runner = [&]() {
        lyndon_isa_nsv(&(text_vec.data()[1]), array.data(),
                       text_vec.size() - 1);
      };
      run_generic("lyndon-isa-nsv32", info, text_vec.size() - 2,
                  s.number_of_runs, runner);
    }

    if (s.matches("divsufsort32")) {
      std::vector<int32_t> sa_vec(text_vec.size() - 1);
      auto runner = [&]() {
        divsufsort(&(text_vec.data()[1]), sa_vec.data(), text_vec.size() - 1);
      };
      auto teardown = [&]() {
        sa_vec.resize(0);
        sa_vec.resize(text_vec.size() - 1);
      };
      run_generic("divsufsort32", info, text_vec.size() - 2, s.number_of_runs,
                  runner, teardown);
    }

    if (s.matches("lyndon-array64")) {
      std::vector<uint64_t> array(text_vec.size());
      auto runner = [&]() {
        xss::lyndon_array(text_vec.data(), array.data(), text_vec.size());
      };
      run_generic("lyndon-array64", info, text_vec.size() - 2, s.number_of_runs,
                  runner);
    }

    if (s.matches("nss-array64")) {
      std::vector<uint64_t> array(text_vec.size());
      auto runner = [&]() {
        xss::nss_array(text_vec.data(), array.data(), text_vec.size());
      };
      run_generic("nss-array64", info, text_vec.size() - 2, s.number_of_runs,
                  runner);
    }

    if (s.matches("pss-array64")) {
      std::vector<uint64_t> array(text_vec.size());
      auto runner = [&]() {
        xss::pss_array(text_vec.data(), array.data(), text_vec.size());
      };
      run_generic("pss-array64", info, text_vec.size() - 2, s.number_of_runs,
                  runner);
    }

    if (s.matches("pss-and-lyndon-array64")) {
      std::vector<uint64_t> array1(text_vec.size());
      std::vector<uint64_t> array2(text_vec.size());
      auto runner = [&]() {
        xss::pss_and_lyndon_array(text_vec.data(), array1.data(), array2.data(),
                                  text_vec.size());
      };
      run_generic("pss-and-lyndon-array64", info, text_vec.size() - 2,
                  s.number_of_runs, runner);
    }

    if (s.matches("pss-and-nss-array64")) {
      std::vector<uint64_t> array1(text_vec.size());
      std::vector<uint64_t> array2(text_vec.size());
      auto runner = [&]() {
        xss::pss_and_nss_array(text_vec.data(), array1.data(), array2.data(),
                               text_vec.size());
      };
      run_generic("pss-and-nss-array64", info, text_vec.size() - 2,
                  s.number_of_runs, runner);
    }

    if (s.matches("lyndon-isa-nsv64")) {
      std::vector<uint64_t> array(text_vec.size() - 1);
      auto runner = [&]() {
        lyndon_isa_nsv(&(text_vec.data()[1]), array.data(),
                       text_vec.size() - 1);
      };
      run_generic("lyndon-isa-nsv64", info, text_vec.size() - 2,
                  s.number_of_runs, runner);
    }

    if (s.matches("divsufsort64")) {
      std::vector<int64_t> sa_vec(text_vec.size() - 1);
      auto runner = [&]() {
        divsufsort64(&(text_vec.data()[1]), sa_vec.data(), text_vec.size() - 1);
      };
      auto teardown = [&]() {
        sa_vec.resize(0);
        sa_vec.resize(text_vec.size() - 1);
      };
      run_generic("divsufsort64", info, text_vec.size() - 2, s.number_of_runs,
                  runner, teardown);
    }
  }
}
