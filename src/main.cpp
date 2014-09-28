#include <codecvt>
#include <iostream>
#include <locale>
#include <sstream>

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
      std::cout << "arg " << j << ": " << result[j] << std::endl;
    }
  }

  cxxopts::Options options;

  options.add_options()
    ("a,apple", "an apple")
    ("b,bob", "Bob")
    ("b,bob", "Bob")
  ;

  options.parse(argc, argv);

  } catch (const std::regex_error& e)
  {
    std::cout << "regex_error: " << e.what() << std::endl;
    exit(1);
  }

  return 0;
}
