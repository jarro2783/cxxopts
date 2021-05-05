#include "catch.hpp"

#include <initializer_list>

#include "cxxopts.hpp"

class Argv {
  public:

  Argv(std::initializer_list<const char*> args)
  : m_argv(new const char*[args.size()])
  , m_argc(static_cast<int>(args.size()))
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

  const char** argv() const {
    return m_argv.get();
  }

  int argc() const {
    return m_argc;
  }

  private:

  std::vector<std::unique_ptr<char[]>> m_args{};
  std::unique_ptr<const char*[]> m_argv;
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
    ("nothing", "won't exist", cxxopts::value<std::string>())
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

  auto** actual_argv = argv.argv();
  auto argc = argv.argc();

  auto result = options.parse(argc, actual_argv);

  CHECK(result.count("long") == 1);
  CHECK(result.count("s") == 1);
  CHECK(result.count("value") == 1);
  CHECK(result.count("a") == 1);
  CHECK(result["value"].as<std::string>() == "value");
  CHECK(result["a"].as<std::string>() == "b");
  CHECK(result.count("6") == 1);
  CHECK(result.count("p") == 2);
  CHECK(result.count("space") == 2);

  auto& arguments = result.arguments();
  REQUIRE(arguments.size() == 7);
  CHECK(arguments[0].key() == "long");
  CHECK(arguments[0].value() == "true");
  CHECK(arguments[0].as<bool>() == true);

  CHECK(arguments[1].key() == "short");
  CHECK(arguments[2].key() == "value");
  CHECK(arguments[3].key() == "av");

  CHECK_THROWS_AS(result["nothing"].as<std::string>(), cxxopts::option_has_no_value_exception&);
}

TEST_CASE("Short options", "[options]")
{
  cxxopts::Options options("test_short", " - test short options");

  options.add_options()
    ("a", "a short option", cxxopts::value<std::string>());

  Argv argv({"test_short", "-a", "value"});

  auto actual_argv = argv.argv();
  auto argc = argv.argc();

  auto result = options.parse(argc, actual_argv);

  CHECK(result.count("a") == 1);
  CHECK(result["a"].as<std::string>() == "value");

  REQUIRE_THROWS_AS(options.add_options()("", "nothing option"),
    cxxopts::invalid_option_format_error&);
}

TEST_CASE("No positional", "[positional]")
{
  cxxopts::Options options("test_no_positional",
    " - test no positional options");

  Argv av({"tester", "a", "b", "def"});

  auto** argv = av.argv();
  auto argc = av.argc();
  auto result = options.parse(argc, argv);

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

  std::vector<std::string> pos_names = {"positional"};

  options.parse_positional(pos_names.begin(), pos_names.end());

  auto result = options.parse(argc, argv);

  CHECK(result.unmatched().size() == 0);
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

  auto** argv = av.argv();
  auto argc = av.argc();

  auto result = options.parse(argc, argv);

  CHECK(result.unmatched().size() == 0);
  CHECK(result.count("output"));
  CHECK(result["input"].as<std::string>() == "b");
  CHECK(result["output"].as<std::string>() == "a");

  auto& positional = result["positional"].as<std::vector<std::string>>();

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

  auto** argv = av.argv();
  auto argc = av.argc();

  auto old_argv = argv;
  auto old_argc = argc;

  auto result = options.parse(argc, argv);

  auto& unmatched = result.unmatched();
  CHECK((unmatched == std::vector<std::string>{"a", "b", "c", "d"}));
}

TEST_CASE("Positional not valid", "[positional]") {
  cxxopts::Options options("positional_invalid", "invalid positional argument");
  options.add_options()
      ("long", "a long option", cxxopts::value<std::string>())
      ;

  options.parse_positional("something");

  Argv av({"foobar", "bar", "baz"});

  auto** argv = av.argv();
  auto argc = av.argc();

  CHECK_THROWS_AS(options.parse(argc, argv), cxxopts::option_not_exists_exception&);
}

TEST_CASE("Empty with implicit value", "[implicit]")
{
  cxxopts::Options options("empty_implicit", "doesn't handle empty");
  options.add_options()
    ("implicit", "Has implicit", cxxopts::value<std::string>()
      ->implicit_value("foo"));

  Argv av({"implicit", "--implicit="});

  auto** argv = av.argv();
  auto argc = av.argc();

  auto result = options.parse(argc, argv);

  REQUIRE(result.count("implicit") == 1);
  REQUIRE(result["implicit"].as<std::string>() == "");
}

TEST_CASE("Boolean without implicit value", "[implicit]")
{
  cxxopts::Options options("no_implicit", "bool without an implicit value");
  options.add_options()
    ("bool", "Boolean without implicit", cxxopts::value<bool>()
      ->no_implicit_value());

  SECTION("When no value provided") {
    Argv av({"no_implicit", "--bool"});

    auto** argv = av.argv();
    auto argc = av.argc();

    CHECK_THROWS_AS(options.parse(argc, argv), cxxopts::missing_argument_exception&);
  }

  SECTION("With equal-separated true") {
    Argv av({"no_implicit", "--bool=true"});

    auto** argv = av.argv();
    auto argc = av.argc();

    auto result = options.parse(argc, argv);
    CHECK(result.count("bool") == 1);
    CHECK(result["bool"].as<bool>() == true);
  }

  SECTION("With equal-separated false") {
    Argv av({"no_implicit", "--bool=false"});

    auto** argv = av.argv();
    auto argc = av.argc();

    auto result = options.parse(argc, argv);
    CHECK(result.count("bool") == 1);
    CHECK(result["bool"].as<bool>() == false);
  }

  SECTION("With space-separated true") {
    Argv av({"no_implicit", "--bool", "true"});

    auto** argv = av.argv();
    auto argc = av.argc();

    auto result = options.parse(argc, argv);
    CHECK(result.count("bool") == 1);
    CHECK(result["bool"].as<bool>() == true);
  }

  SECTION("With space-separated false") {
    Argv av({"no_implicit", "--bool", "false"});

    auto** argv = av.argv();
    auto argc = av.argc();

    auto result = options.parse(argc, argv);
    CHECK(result.count("bool") == 1);
    CHECK(result["bool"].as<bool>() == false);
  }
}

TEST_CASE("Default values", "[default]")
{
  cxxopts::Options options("defaults", "has defaults");
  options.add_options()
    ("default", "Has implicit", cxxopts::value<int>()->default_value("42"))
    ("v,vector", "Default vector", cxxopts::value<std::vector<int>>()
      ->default_value("1,4"))
    ;

  SECTION("Sets defaults") {
    Argv av({"implicit"});

    auto** argv = av.argv();
    auto argc = av.argc();

    auto result = options.parse(argc, argv);
    CHECK(result.count("default") == 0);
    CHECK(result["default"].as<int>() == 42);

    auto& v = result["vector"].as<std::vector<int>>();
    REQUIRE(v.size() == 2);
    CHECK(v[0] == 1);
    CHECK(v[1] == 4);
  }

  SECTION("When values provided") {
    Argv av({"implicit", "--default", "5"});

    auto** argv = av.argv();
    auto argc = av.argc();

    auto result = options.parse(argc, argv);
    CHECK(result.count("default") == 1);
    CHECK(result["default"].as<int>() == 5);
  }
}

TEST_CASE("Parse into a reference", "[reference]")
{
  int value = 0;

  cxxopts::Options options("into_reference", "parses into a reference");
  options.add_options()
    ("ref", "A reference", cxxopts::value(value));

  Argv av({"into_reference", "--ref", "42"});

  auto argv = av.argv();
  auto argc = av.argc();

  auto result = options.parse(argc, argv);
  CHECK(result.count("ref") == 1);
  CHECK(value == 42);
}

TEST_CASE("Integers", "[options]")
{
  cxxopts::Options options("parses_integers", "parses integers correctly");
  options.add_options()
    ("positional", "Integers", cxxopts::value<std::vector<int>>());

  Argv av({"ints", "--", "5", "6", "-6", "0", "0xab", "0xAf", "0x0"});

  auto** argv = av.argv();
  auto argc = av.argc();

  options.parse_positional("positional");
  auto result = options.parse(argc, argv);

  REQUIRE(result.count("positional") == 7);

  auto& positional = result["positional"].as<std::vector<int>>();
  REQUIRE(positional.size() == 7);
  CHECK(positional[0] == 5);
  CHECK(positional[1] == 6);
  CHECK(positional[2] == -6);
  CHECK(positional[3] == 0);
  CHECK(positional[4] == 0xab);
  CHECK(positional[5] == 0xaf);
  CHECK(positional[6] == 0x0);
}

TEST_CASE("Leading zero integers", "[options]")
{
  cxxopts::Options options("parses_integers", "parses integers correctly");
  options.add_options()
    ("positional", "Integers", cxxopts::value<std::vector<int>>());

  Argv av({"ints", "--", "05", "06", "0x0ab", "0x0001"});

  auto** argv = av.argv();
  auto argc = av.argc();

  options.parse_positional("positional");
  auto result = options.parse(argc, argv);

  REQUIRE(result.count("positional") == 4);

  auto& positional = result["positional"].as<std::vector<int>>();
  REQUIRE(positional.size() == 4);
  CHECK(positional[0] == 5);
  CHECK(positional[1] == 6);
  CHECK(positional[2] == 0xab);
  CHECK(positional[3] == 0x1);
}

TEST_CASE("Unsigned integers", "[options]")
{
  cxxopts::Options options("parses_unsigned", "detects unsigned errors");
  options.add_options()
    ("positional", "Integers", cxxopts::value<std::vector<unsigned int>>());

  Argv av({"ints", "--", "-2"});

  auto** argv = av.argv();
  auto argc = av.argc();

  options.parse_positional("positional");
  CHECK_THROWS_AS(options.parse(argc, argv), cxxopts::argument_incorrect_type&);
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
    auto result = options.parse(argc, argv);

    REQUIRE(result.count("positional") == 5);

    auto& positional = result["positional"].as<std::vector<int8_t>>();
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

  CHECK_THROWS_AS((integer_parser("128", si)), cxxopts::argument_incorrect_type&);
  CHECK_THROWS_AS((integer_parser("-129", si)), cxxopts::argument_incorrect_type&);
  CHECK_THROWS_AS((integer_parser("256", ui)), cxxopts::argument_incorrect_type&);
  CHECK_THROWS_AS((integer_parser("-0x81", si)), cxxopts::argument_incorrect_type&);
  CHECK_THROWS_AS((integer_parser("0x80", si)), cxxopts::argument_incorrect_type&);
  CHECK_THROWS_AS((integer_parser("0x100", ui)), cxxopts::argument_incorrect_type&);
}

TEST_CASE("Integer overflow", "[options]")
{
  using namespace cxxopts::values;

  cxxopts::Options options("reject_overflow", "rejects overflowing integers");
  options.add_options()
    ("positional", "Integers", cxxopts::value<std::vector<int8_t>>());

  Argv av({"ints", "--", "128"});

  auto argv = av.argv();
  auto argc = av.argc();

  options.parse_positional("positional");
  CHECK_THROWS_AS(options.parse(argc, argv), cxxopts::argument_incorrect_type&);

  int integer = 0;
  CHECK_THROWS_AS((integer_parser("23423423423", integer)), cxxopts::argument_incorrect_type&);
  CHECK_THROWS_AS((integer_parser("234234234234", integer)), cxxopts::argument_incorrect_type&);
}

TEST_CASE("Floats", "[options]")
{
  cxxopts::Options options("parses_floats", "parses floats correctly");
  options.add_options()
    ("double", "Double precision", cxxopts::value<double>())
    ("positional", "Floats", cxxopts::value<std::vector<float>>());

  Argv av({"floats", "--double", "0.5", "--", "4", "-4", "1.5e6", "-1.5e6"});

  auto** argv = av.argv();
  auto argc = av.argc();

  options.parse_positional("positional");
  auto result = options.parse(argc, argv);

  REQUIRE(result.count("double") == 1);
  REQUIRE(result.count("positional") == 4);

  CHECK(result["double"].as<double>() == 0.5);

  auto& positional = result["positional"].as<std::vector<float>>();
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

    auto** argv = av.argv();
    auto argc = av.argc();

    options.parse_positional("positional");
    CHECK_THROWS_AS(options.parse(argc, argv), cxxopts::argument_incorrect_type&);
}

TEST_CASE("Booleans", "[boolean]") {
  cxxopts::Options options("parses_floats", "parses floats correctly");
  options.add_options()
    ("bool", "A Boolean", cxxopts::value<bool>())
    ("debug", "Debugging", cxxopts::value<bool>())
    ("timing", "Timing", cxxopts::value<bool>())
    ("verbose", "Verbose", cxxopts::value<bool>())
    ("dry-run", "Dry Run", cxxopts::value<bool>())
    ("noExplicitDefault", "No Explicit Default", cxxopts::value<bool>())
    ("defaultTrue", "Timing", cxxopts::value<bool>()->default_value("true"))
    ("defaultFalse", "Timing", cxxopts::value<bool>()->default_value("false"))
    ("others", "Other arguments", cxxopts::value<std::vector<std::string>>())
    ;

  options.parse_positional("others");

  Argv av({"booleans", "--bool=false", "--debug=true", "--timing", "--verbose=1", "--dry-run=0", "extra"});

  auto** argv = av.argv();
  auto argc = av.argc();

  auto result = options.parse(argc, argv);

  REQUIRE(result.count("bool") == 1);
  REQUIRE(result.count("debug") == 1);
  REQUIRE(result.count("timing") == 1);
  REQUIRE(result.count("verbose") == 1);
  REQUIRE(result.count("dry-run") == 1);
  REQUIRE(result.count("noExplicitDefault") == 0);
  REQUIRE(result.count("defaultTrue") == 0);
  REQUIRE(result.count("defaultFalse") == 0);

  CHECK(result["bool"].as<bool>() == false);
  CHECK(result["debug"].as<bool>() == true);
  CHECK(result["timing"].as<bool>() == true);
  CHECK(result["verbose"].as<bool>() == true);
  CHECK(result["dry-run"].as<bool>() == false);
  CHECK(result["noExplicitDefault"].as<bool>() == false);
  CHECK(result["defaultTrue"].as<bool>() == true);
  CHECK(result["defaultFalse"].as<bool>() == false);

  REQUIRE(result.count("others") == 1);
}

TEST_CASE("std::vector", "[vector]") {
  std::vector<double> vector;
  cxxopts::Options options("vector", " - tests vector");
  options.add_options()
      ("vector", "an vector option", cxxopts::value<std::vector<double>>(vector));

  Argv av({"vector", "--vector", "1,-2.1,3,4.5"});

  auto** argv = av.argv();
  auto argc = av.argc();

  options.parse(argc, argv);

  REQUIRE(vector.size() == 4);
  CHECK(vector[0] == 1);
  CHECK(vector[1] == -2.1);
  CHECK(vector[2] == 3);
  CHECK(vector[3] == 4.5);
}

#ifdef CXXOPTS_HAS_OPTIONAL
TEST_CASE("std::optional", "[optional]") {
  std::optional<std::string> optional;
  cxxopts::Options options("optional", " - tests optional");
  options.add_options()
    ("optional", "an optional option", cxxopts::value<std::optional<std::string>>(optional));

  Argv av({"optional", "--optional", "foo"});

  auto** argv = av.argv();
  auto argc = av.argc();

  options.parse(argc, argv);

  REQUIRE(optional.has_value());
  CHECK(*optional == "foo");
}
#endif

TEST_CASE("Unrecognised options", "[options]") {
  cxxopts::Options options("unknown_options", " - test unknown options");

  options.add_options()
    ("long", "a long option")
    ("s,short", "a short option");

  Argv av({
    "unknown_options",
    "--unknown",
    "--long",
    "-su",
    "--another_unknown",
  });

  auto** argv = av.argv();
  auto argc = av.argc();

  SECTION("Default behaviour") {
    CHECK_THROWS_AS(options.parse(argc, argv), cxxopts::option_not_exists_exception&);
  }

  SECTION("After allowing unrecognised options") {
    options.allow_unrecognised_options();
    auto result = options.parse(argc, argv);
    auto& unmatched = result.unmatched();
    CHECK((unmatched == std::vector<std::string>{"--unknown", "--another_unknown"}));
  }
}

TEST_CASE("Allow bad short syntax", "[options]") {
  cxxopts::Options options("unknown_options", " - test unknown options");

  options.add_options()
    ("long", "a long option")
    ("s,short", "a short option");

  Argv av({
    "unknown_options",
    "-some_bad_short",
  });

  auto** argv = av.argv();
  auto argc = av.argc();

  SECTION("Default behaviour") {
    CHECK_THROWS_AS(options.parse(argc, argv), cxxopts::option_syntax_exception&);
  }

  SECTION("After allowing unrecognised options") {
    options.allow_unrecognised_options();
    CHECK_NOTHROW(options.parse(argc, argv));
    REQUIRE(argc == 2);
    CHECK_THAT(argv[1], Catch::Equals("-some_bad_short"));
  }
}

TEST_CASE("Invalid option syntax", "[options]") {
  cxxopts::Options options("invalid_syntax", " - test invalid syntax");

  Argv av({
    "invalid_syntax",
    "--a",
  });

  auto** argv = av.argv();
  auto argc = av.argc();

  SECTION("Default behaviour") {
    CHECK_THROWS_AS(options.parse(argc, argv), cxxopts::option_syntax_exception&);
  }
}

TEST_CASE("Options empty", "[options]") {
  cxxopts::Options options("Options list empty", " - test empty option list");
  options.add_options();
  options.add_options("");
  options.add_options("", {});
  options.add_options("test");

  Argv argv_({
       "test",
       "--unknown"
     });
  auto argc = argv_.argc();
  auto** argv = argv_.argv();

  CHECK(options.groups().empty());
  CHECK_THROWS_AS(options.parse(argc, argv), cxxopts::option_not_exists_exception&);
}

TEST_CASE("Initializer list with group", "[options]") {
  cxxopts::Options options("Initializer list group", " - test initializer list with group");

  options.add_options("", {
    {"a, address", "server address", cxxopts::value<std::string>()->default_value("127.0.0.1")},
    {"p, port",  "server port",  cxxopts::value<std::string>()->default_value("7110"), "PORT"},
  });

  cxxopts::Option help{"h,help", "Help"};

  options.add_options("TEST_GROUP", {
    {"t, test", "test option"},
    help
  });

  Argv argv({
      "test",
      "--address",
      "10.0.0.1",
      "-p",
      "8000",
      "-t",
    });
  auto** actual_argv = argv.argv();
  auto argc = argv.argc();
  auto result = options.parse(argc, actual_argv);

  CHECK(options.groups().size() == 2);
  CHECK(result.count("address") == 1);
  CHECK(result.count("port") == 1);
  CHECK(result.count("test") == 1);
  CHECK(result.count("help") == 0);
  CHECK(result["address"].as<std::string>() == "10.0.0.1");
  CHECK(result["port"].as<std::string>() == "8000");
  CHECK(result["test"].as<bool>() == true);
}

TEST_CASE("Option add with add_option(string, Option)", "[options]") {
  cxxopts::Options options("Option add with add_option", " - test Option add with add_option(string, Option)");

  cxxopts::Option option_1("t,test", "test option", cxxopts::value<int>()->default_value("7"), "TEST");

  options.add_option("", option_1);
  options.add_option("TEST", {"a,aggregate", "test option 2", cxxopts::value<int>(), "AGGREGATE"});

  Argv argv_({
       "test",
       "--test",
       "5",
       "-a",
       "4"
     });
  auto argc = argv_.argc();
  auto** argv = argv_.argv();
  auto result = options.parse(argc, argv);

  CHECK(result.arguments().size()==2);
  CHECK(options.groups().size() == 2);
  CHECK(result.count("address") == 0);
  CHECK(result.count("aggregate") == 1);
  CHECK(result.count("test") == 1);
  CHECK(result["aggregate"].as<int>() == 4);
  CHECK(result["test"].as<int>() == 5);
}

TEST_CASE("Const array", "[const]") {
  const char* const option_list[] = {"empty", "options"};
  cxxopts::Options options("Empty options", " - test constness");
  auto result = options.parse(2, option_list);
}

TEST_CASE("Parameter follow option", "[parameter]") {
  cxxopts::Options options("param_follow_opt", " - test parameter follow option without space.");
  options.add_options()
    ("j,job", "Job", cxxopts::value<std::vector<unsigned>>());
  Argv av({"implicit",
      "-j", "9",
      "--job", "7",
      "--job=10",
      "-j5",
  });

  auto ** argv = av.argv();
  auto argc = av.argc();

  auto result = options.parse(argc, argv);

  REQUIRE(result.count("job") == 4);

  auto job_values = result["job"].as<std::vector<unsigned>>();
  CHECK(job_values[0] == 9);
  CHECK(job_values[1] == 7);
  CHECK(job_values[2] == 10);
  CHECK(job_values[3] == 5);
}
