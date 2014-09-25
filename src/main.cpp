#include <iostream>
#include <locale>

#include "cxxopts.hpp"

int main(int argc, char* argv[])
{
  std::locale::global(std::locale(""));
  try
  {

  cxxopts::option_matcher.imbue(std::locale(""));
  cxxopts::option_matcher.assign
    ("--([[:alpha:]][-_[:alpha:]]+)(=(.*))?|-([a-zA-Z]+)", 
     std::regex_constants::extended | std::regex_constants::ECMAScript | 
       std::regex_constants::collate);
  std::cout << cxxopts::option_matcher.getloc().name() << std::endl;

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

  } catch (const std::regex_error& e)
  {
    std::cout << "regex_error: " << e.what() << std::endl;
    exit(1);
  }

  return 0;
}
