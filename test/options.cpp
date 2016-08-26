#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <initializer_list>

#include "../src/cxxopts.hpp"

class Argv {
  public:

  Argv(std::initializer_list<const char*> argv)
  : m_argv(new char*[argv.size()])
  , m_argc(argv.size())
  {
    int i = 0;
    auto iter = argv.begin();
    while (iter != argv.end()) {
      auto len = strlen(*iter) + 1;
      auto ptr = std::unique_ptr<char[]>(new char[len]);

      strcpy(ptr.get(), *iter);
      m_args.push_back(std::move(ptr));
      m_argv.get()[i] = m_args.back().get();

      ++iter;
      ++i;
    }
  }

  char** argv() const {
    return m_argv.get();
  }

  int argc() const {
    return m_argc;
  }

  private:

  std::vector<std::unique_ptr<char[]>> m_args;
  std::unique_ptr<char*[]> m_argv;
  int m_argc;
};

TEST_CASE("Basic options", "[options]")
{

  cxxopts::Options options("tester", " - test basic options");

  options.add_options()
    ("long", "a long option")
    ("s,short", "a short option")
    ("value", "an option with a value", cxxopts::value<std::string>())
    ("a,av", "a short option with a value", cxxopts::value<std::string>());

  Argv argv({
    "tester",
    "--long",
    "-s",
    "--value",
    "value",
    "-a",
    "b"
  });

  char** actual_argv = argv.argv();
  auto argc = argv.argc();

  options.parse(argc, actual_argv);

  CHECK(options.count("long") == 1);
  CHECK(options.count("s") == 1);
  CHECK(options.count("value") == 1);
  CHECK(options.count("a") == 1);
  CHECK(options["value"].as<std::string>() == "value");
  CHECK(options["a"].as<std::string>() == "b");
}

TEST_CASE("No positional", "[positional]")
{
  cxxopts::Options options("test_positional", " - test no positional options");

  Argv argv({"tester", "a", "b", "def"});

  char** passed_argv = argv.argv();
  auto argc = argv.argc();
  options.parse(argc, passed_argv);
}
