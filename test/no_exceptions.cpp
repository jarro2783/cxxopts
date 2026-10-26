#include "cxxopts.hpp"

#include <cstdint>
#include <string>
#include <vector>

int main(int argc, char** argv)
{
  cxxopts::Options options("no-exceptions");
  options.add_options()
    ("number", "Integer", cxxopts::value<int>())
    ("int64", "Int64", cxxopts::value<int64_t>())
    ("float", "Float", cxxopts::value<float>())
    ("double", "Double", cxxopts::value<double>())
    ("flag", "Flag", cxxopts::value<bool>()->default_value("false"))
    ("items", "Items", cxxopts::value<std::vector<std::string>>());

  if (argc > 1)
  {
    options.parse(argc, argv);
    // Invalid input must terminate in throw_or_mimic before reaching here.
    return 2;
  }

  const char* args[] = {
    "test",
    "--number=-42",
    "--int64=9223372036854775807",
    "--float=3.14",
    "--double=-2.71828",
    "--flag",
    "--items=a,b"
  };
  const auto result = options.parse(7, args);
  if (result["number"].as<int>() != -42 ||
      result["int64"].as<int64_t>() != 9223372036854775807LL ||
      result["float"].as<float>() < 3.13f || result["float"].as<float>() > 3.15f ||
      result["double"].as<double>() > -2.71 || result["double"].as<double>() < -2.72 ||
      !result["flag"].as<bool>() ||
      result["items"].as<std::vector<std::string>>() !=
        std::vector<std::string>({"a", "b"}))
  {
    return 2;
  }
  return 0;
}
