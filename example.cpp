#include <string>
#include <utility>
#include <vector>
#include "include/lyndon_array.hpp"

int main() {
  std::string in("amtrakairbusmississippi");
  std::cout << "Input:  " << in << std::endl;
  std::cout << "Length: " << in.size() << std::endl;
  
  std::vector<unsigned int> out(in.size());

  lyndon_array_nosentinels(in.data(), out.data(), in.size());
  std::cout << "\nLyndon (simulate sentinels): "; 
  for(auto v : out) std::cout << v << " ";
  std::cout << std::endl;
  
  
  std::cout << std::endl;
  std::cout << std::endl;
  
  
  std::string in2("!amtrakairbusmississippi!");
  std::cout << "Input:  " << in2 << std::endl;
  std::cout << "Length: " << in2.size() << std::endl;
  
  std::vector<unsigned int> out2(in2.size());
  
  lyndon_array_sentinels(in2.data(), out2.data(), in2.size());
  std::cout << "\nLyndon (physical sentinels): "; 
  for(auto v : out2) std::cout << v << " ";
  std::cout << std::endl;
  
  return 0;  
}
