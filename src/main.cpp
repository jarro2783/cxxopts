#include <iostream>

#include "cxxopts.hpp"

int main(int argc, char* argv[])
{
  try
  {

  std::match_results<const char*> result;

  for (int i = 1; i < argc; ++i)
  {
    std::cout << "Argument " << i << std::endl;
    std::regex_match(argv[i], result, cxxopts::option_matcher);
    std::cout << "empty = " << result.empty() << std::endl;
    std::cout << "size = " << result.size() << std::endl;

    std::cout << "matches:" << std::endl;
    for (int j = 0; j != result.size(); ++j)
    {
      std::cout << result[j] << std::endl;
    }
  }

  } catch (const std::regex_error& e)
  {
    std::cout << "regex_error: " << e.what() << std::endl;
    exit(1);
  }

  return 0;
}
