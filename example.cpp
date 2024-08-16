#include <string>
#include <utility>
#include <vector>
#include "include/lyndon_array.hpp"

int main() {
  // need sentinel characters!
  std::string in("!amtrakairbusmississippi!");
  std::cout << "Input:  " << in << std::endl;
  
  std::pair<std::vector<unsigned int>, std::vector<unsigned int>> out 
    { in.size(), in.size() };
    
  std::pair<std::vector<unsigned int>, std::vector<unsigned int>> clean 
    { in.size(), in.size() };
  
  lyndon_array(in.data(), out.first.data(), in.size());
  std::cout << "\nLyndon: "; 
  for(auto v : out.first) std::cout << v << " ";
  std::cout << std::endl;
  
  out = clean;
  nss_array(in.data(), out.first.data(), in.size());
  std::cout << "\nNSS:    "; 
  for(auto v : out.first) std::cout << v << " ";
  std::cout << std::endl;
  
  out = clean;
  pss_array(in.data(), out.first.data(), in.size());
  std::cout << "\nPSS:    "; 
  for(auto v : out.first) std::cout << v << " ";
  std::cout << std::endl << std::endl;  
  
  out = clean;
  pss_and_lyndon_array(in.data(), out.first.data(), out.second.data(), in.size());
  std::cout << "\nLyndon: "; 
  for(auto v : out.second) std::cout << v << " ";
  std::cout << std::endl;
  std::cout << "\nPSS:    "; 
  for(auto v : out.first) std::cout << v << " ";
  std::cout << std::endl << std::endl;  
  
  out = clean;
  pss_and_nss_array(in.data(), out.first.data(), out.second.data(), in.size());
  std::cout << "\nNSS:    "; 
  for(auto v : out.second) std::cout << v << " ";
  std::cout << std::endl;
  std::cout << "\nPSS:    "; 
  for(auto v : out.first) std::cout << v << " ";
  std::cout << std::endl << std::endl;
  
  return 0;  
}
