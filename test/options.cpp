#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "../src/cxxopts.hpp"

class Argv {
  public:

  Argv(int n, const char** argv) 
  : m_argv(new char*[n])
  {
    for (int i = 0; i != n; ++i) {
      auto len = strlen(argv[i]) + 1;
      auto ptr = std::unique_ptr<char[]>(new char[len]);

      strcpy(ptr.get(), argv[i]);
      m_args.push_back(std::move(ptr));
      m_argv.get()[i] = m_args.back().get();
    }
  }

  char** argv() const {
    return m_argv.get();
  }

  private:

  std::vector<std::unique_ptr<char[]>> m_args;
  std::unique_ptr<char*[]> m_argv;
};

TEST_CASE("Basic options", "[options]")
{

  cxxopts::Options options("tester", " - test basic options");

  options.add_options()
    ("long", "a long option")
    ("s,short", "a short option")
    ("value", "an option with a value", cxxopts::value<std::string>())
    ("a,av", "a short option with a value", cxxopts::value<std::string>());

  const char* args[] = {
    "tester",
    "--long",
    "-s",
    "--value",
    "value",
    "-a",
    "b"
  };

  int argc = sizeof(args) / sizeof(args[0]);

  Argv argv(argc, args);

  char** actual_argv = argv.argv();

  options.parse(argc, actual_argv);

  CHECK(options.count("long") == 1);
  CHECK(options.count("s") == 1);
  CHECK(options.count("value") == 1);
  CHECK(options.count("a") == 1);
  CHECK(options["value"].as<std::string>() == "value");
  CHECK(options["a"].as<std::string>() == "b");
}
