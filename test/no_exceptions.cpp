#include "cxxopts.hpp"

#include <string>
#include <vector>

int main(int argc, char** argv)
{
  cxxopts::Options options("no-exceptions");
  options.add_options()
    ("number", "Integer", cxxopts::value<int>())
    ("flag", "Flag", cxxopts::value<bool>()->default_value("false"))
    ("items", "Items", cxxopts::value<std::vector<std::string>>());

  if (argc > 1)
  {
    options.parse(argc, argv);
    // Invalid input must terminate in throw_or_mimic before reaching here.
    return 2;
  }

  const char* args[] = {"test", "--number=-42", "--flag", "--items=a,b"};
  const auto result = options.parse(4, args);
  if (result["number"].as<int>() != -42 || !result["flag"].as<bool>() ||
      result["items"].as<std::vector<std::string>>() !=
        std::vector<std::string>({"a", "b"}))
  {
    return 2;
  }
  return 0;
}
