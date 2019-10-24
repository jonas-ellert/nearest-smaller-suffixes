#include <xss.hpp>

int main()
{
  std::cout << "Hello World!" << std::endl;
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
