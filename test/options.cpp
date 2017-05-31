#include "catch.hpp"

#include <initializer_list>

#include "cxxopts.hpp"

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
    ("a,av", "a short option with a value", cxxopts::value<std::string>())
    ("6,six", "a short number option")
    ;

  Argv argv({
    "tester",
    "--long",
    "-s",
    "--value",
    "value",
    "-a",
    "b",
    "-6"
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
  CHECK(options.count("6") == 1);
}

TEST_CASE("Short options", "[options]")
{
  cxxopts::Options options("test_short", " - test short options");

  options.add_options()
    ("a", "a short option", cxxopts::value<std::string>());

  Argv argv({"test_short", "-a", "value"});

  auto actual_argv = argv.argv();
  auto argc = argv.argc();

  options.parse(argc, actual_argv);

  CHECK(options.count("a") == 1);
  CHECK(options["a"].as<std::string>() == "value");

  REQUIRE_THROWS_AS(options.add_options()("", "nothing option"), 
    cxxopts::invalid_option_format_error);
}

TEST_CASE("No positional", "[positional]")
{
  cxxopts::Options options("test_no_positional",
    " - test no positional options");

  Argv av({"tester", "a", "b", "def"});

  char** argv = av.argv();
  auto argc = av.argc();
  options.parse(argc, argv);

  REQUIRE(argc == 4);
  CHECK(strcmp(argv[1], "a") == 0);
}

TEST_CASE("All positional", "[positional]")
{
  std::vector<std::string> positional;

  cxxopts::Options options("test_all_positional", " - test all positional");
  options.add_options()
    ("positional", "Positional parameters",
      cxxopts::value<std::vector<std::string>>(positional))
  ;

  Argv av({"tester", "a", "b", "c"});

  auto argc = av.argc();
  auto argv = av.argv();

  options.parse_positional("positional");

  options.parse(argc, argv);

  REQUIRE(argc == 1);
  REQUIRE(positional.size() == 3);

  CHECK(positional[0] == "a");
  CHECK(positional[1] == "b");
  CHECK(positional[2] == "c");
}

TEST_CASE("Some positional explicit", "[positional]")
{
  cxxopts::Options options("positional_explicit", " - test positional");

  options.add_options()
    ("input", "Input file", cxxopts::value<std::string>())
    ("output", "Output file", cxxopts::value<std::string>())
    ("positional", "Positional parameters",
      cxxopts::value<std::vector<std::string>>())
  ;

  options.parse_positional({"input", "output", "positional"});

  Argv av({"tester", "--output", "a", "b", "c", "d"});

  char** argv = av.argv();
  auto argc = av.argc();

  options.parse(argc, argv);

  CHECK(argc == 1);
  CHECK(options.count("output"));
  CHECK(options["input"].as<std::string>() == "b");
  CHECK(options["output"].as<std::string>() == "a");

  auto& positional = options["positional"].as<std::vector<std::string>>();

  REQUIRE(positional.size() == 2);
  CHECK(positional[0] == "c");
  CHECK(positional[1] == "d");
}

TEST_CASE("No positional with extras", "[positional]")
{
  cxxopts::Options options("posargmaster", "shows incorrect handling");
  options.add_options()
      ("dummy", "oh no", cxxopts::value<std::string>())
      ;

  Argv av({"extras", "--", "a", "b", "c", "d"});

  char** argv = av.argv();
  auto argc = av.argc();

  auto old_argv = argv;
  auto old_argc = argc;

  options.parse(argc, argv);

  REQUIRE(argc == old_argc - 1);
  CHECK(argv[0] == std::string("extras"));
  CHECK(argv[1] == std::string("a"));
}

TEST_CASE("Empty with implicit value", "[implicit]")
{
  cxxopts::Options options("empty_implicit", "doesn't handle empty");
  options.add_options()
    ("implicit", "Has implicit", cxxopts::value<std::string>()
      ->implicit_value("foo"));

  Argv av({"implicit", "--implicit", ""});

  char** argv = av.argv();
  auto argc = av.argc();

  options.parse(argc, argv);

  REQUIRE(options.count("implicit") == 1);
  REQUIRE(options["implicit"].as<std::string>() == "");
}
