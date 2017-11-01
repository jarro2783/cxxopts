#include "catch.hpp"

#include <initializer_list>

#include "cxxopts.hpp"

class Argv {
  public:

  Argv(std::initializer_list<const char*> args)
  : m_argv(new char*[args.size()])
  , m_argc(args.size())
  {
    int i = 0;
    auto iter = args.begin();
    while (iter != args.end()) {
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
    ("p, space", "an option with space between short and long")
    ;

  Argv argv({
    "tester",
    "--long",
    "-s",
    "--value",
    "value",
    "-a",
    "b",
    "-6",
    "-p",
    "--space",
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
  CHECK(options.count("p") == 2);
  CHECK(options.count("space") == 2);
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

TEST_CASE("Required arguments", "[options]")
{
  cxxopts::Options options("required", " - test required options");
  options.add_options()
    ("one", "one option")
    ("two", "second option")
  ;

  Argv argv({
    "required",
    "--one"
  });

  auto aargv = argv.argv();
  auto argc = argv.argc();

  options.parse(argc, aargv);
  REQUIRE_THROWS_AS(cxxopts::check_required(options, {"two"}),
    cxxopts::option_required_exception);
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

TEST_CASE("Integers", "[options]")
{
  cxxopts::Options options("parses_integers", "parses integers correctly");
  options.add_options()
    ("positional", "Integers", cxxopts::value<std::vector<int>>());

  Argv av({"ints", "--", "5", "6", "-6", "0", "0xab", "0xAf", "0x0"});

  char** argv = av.argv();
  auto argc = av.argc();

  options.parse_positional("positional");
  options.parse(argc, argv);

  REQUIRE(options.count("positional") == 7);

  auto& positional = options["positional"].as<std::vector<int>>();
  CHECK(positional[0] == 5);
  CHECK(positional[1] == 6);
  CHECK(positional[2] == -6);
  CHECK(positional[3] == 0);
  CHECK(positional[4] == 0xab);
  CHECK(positional[5] == 0xaf);
  CHECK(positional[6] == 0x0);
}

TEST_CASE("Unsigned integers", "[options]")
{
  cxxopts::Options options("parses_unsigned", "detects unsigned errors");
  options.add_options()
    ("positional", "Integers", cxxopts::value<std::vector<unsigned int>>());

  Argv av({"ints", "--", "-2"});

  char** argv = av.argv();
  auto argc = av.argc();

  options.parse_positional("positional");
  CHECK_THROWS_AS(options.parse(argc, argv), cxxopts::argument_incorrect_type);
}

TEST_CASE("Integer bounds", "[integer]")
{
  cxxopts::Options options("integer_boundaries", "check min/max integer");
  options.add_options()
    ("positional", "Integers", cxxopts::value<std::vector<int8_t>>());

  SECTION("No overflow")
  {
    Argv av({"ints", "--", "127", "-128", "0x7f", "-0x80", "0x7e"});

    auto argv = av.argv();
    auto argc = av.argc();

    options.parse_positional("positional");
    options.parse(argc, argv);

    REQUIRE(options.count("positional") == 5);

    auto& positional = options["positional"].as<std::vector<int8_t>>();
    CHECK(positional[0] == 127);
    CHECK(positional[1] == -128);
    CHECK(positional[2] == 0x7f);
    CHECK(positional[3] == -0x80);
    CHECK(positional[4] == 0x7e);
  }
}

TEST_CASE("Overflow on boundary", "[integer]")
{
  using namespace cxxopts::values;

  int8_t si;
  uint8_t ui;

  CHECK_THROWS_AS((integer_parser("128", si)), cxxopts::argument_incorrect_type);
  CHECK_THROWS_AS((integer_parser("-129", si)), cxxopts::argument_incorrect_type);
  CHECK_THROWS_AS((integer_parser("256", ui)), cxxopts::argument_incorrect_type);
  CHECK_THROWS_AS((integer_parser("-0x81", si)), cxxopts::argument_incorrect_type);
  CHECK_THROWS_AS((integer_parser("0x80", si)), cxxopts::argument_incorrect_type);
  CHECK_THROWS_AS((integer_parser("0x100", ui)), cxxopts::argument_incorrect_type);
}

TEST_CASE("Integer overflow", "[options]")
{
  cxxopts::Options options("reject_overflow", "rejects overflowing integers");
  options.add_options()
    ("positional", "Integers", cxxopts::value<std::vector<int8_t>>());

  Argv av({"ints", "--", "128"});

  auto argv = av.argv();
  auto argc = av.argc();

  options.parse_positional("positional");
  CHECK_THROWS_AS(options.parse(argc, argv), cxxopts::argument_incorrect_type);
}

TEST_CASE("Floats", "[options]")
{
  cxxopts::Options options("parses_floats", "parses floats correctly");
  options.add_options()
    ("double", "Double precision", cxxopts::value<double>())
    ("positional", "Floats", cxxopts::value<std::vector<float>>());

  Argv av({"floats", "--double", "0.5", "--", "4", "-4", "1.5e6", "-1.5e6"});

  char** argv = av.argv();
  auto argc = av.argc();

  options.parse_positional("positional");
  options.parse(argc, argv);

  REQUIRE(options.count("double") == 1);
  REQUIRE(options.count("positional") == 4);

  CHECK(options["double"].as<double>() == 0.5);

  auto& positional = options["positional"].as<std::vector<float>>();
  CHECK(positional[0] == 4);
  CHECK(positional[1] == -4);
  CHECK(positional[2] == 1.5e6);
  CHECK(positional[3] == -1.5e6);
}

TEST_CASE("Invalid integers", "[integer]") {
    cxxopts::Options options("invalid_integers", "rejects invalid integers");
    options.add_options()
    ("positional", "Integers", cxxopts::value<std::vector<int>>());

    Argv av({"ints", "--", "Ae"});

    char **argv = av.argv();
    auto argc = av.argc();

    options.parse_positional("positional");
    CHECK_THROWS_AS(options.parse(argc, argv), cxxopts::argument_incorrect_type);
}
