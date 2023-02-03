#include <cassert>
#include <cxxopts.hpp>
#include <fuzzer/FuzzedDataProvider.h>

constexpr int kMaxOptions = 1024;
constexpr int kMaxArgSize = 1024;

enum class ParseableTypes
{
  kInt,
  kString,
  kVectorString,
  kFloat,
  kDouble,

  // Marker for fuzzer.
  kMaxValue,
};

template <typename T>
void
add_fuzzed_option(cxxopts::Options* options, FuzzedDataProvider* provider)
{
  assert(options);
  assert(provider);

  options->add_options()(provider->ConsumeRandomLengthString(kMaxArgSize),
                         provider->ConsumeRandomLengthString(kMaxArgSize),
                         cxxopts::value<T>());
}

extern "C" int
LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
  try
  {
    FuzzedDataProvider provider(data, size);

    // Randomly generate a usage string.
    cxxopts::Options options(provider.ConsumeRandomLengthString(kMaxArgSize),
                             provider.ConsumeRandomLengthString(kMaxArgSize));

    // Randomly generate a set of flags configurations.
    for (int i = 0; i < provider.ConsumeIntegralInRange<int>(0, kMaxOptions);
         i++)
    {
      switch (provider.ConsumeEnum<ParseableTypes>())
      {
      case ParseableTypes::kInt:
        add_fuzzed_option<int>(&options, &provider);
        break;
      case ParseableTypes::kString:
        add_fuzzed_option<std::string>(&options, &provider);
        break;
      case ParseableTypes::kVectorString:
        add_fuzzed_option<std::vector<std::string>>(&options, &provider);
        break;
      case ParseableTypes::kFloat:
        add_fuzzed_option<float>(&options, &provider);
        break;
      case ParseableTypes::kDouble:
        add_fuzzed_option<double>(&options, &provider);
        break;
      default:
        break;
      }
    }
    // Sometimes allow unrecognised options.
    if (provider.ConsumeBool())
    {
      options.allow_unrecognised_options();
    }
    // Sometimes allow trailing positional arguments.
    if (provider.ConsumeBool())
    {
      std::string positional_option_name =
        provider.ConsumeRandomLengthString(kMaxArgSize);
      options.add_options()(positional_option_name,
                            provider.ConsumeRandomLengthString(kMaxArgSize),
                            cxxopts::value<std::vector<std::string>>());
      options.parse_positional({positional_option_name});
    }

    // Build command line input.
    const int argc = provider.ConsumeIntegralInRange<int>(1, kMaxOptions);

    std::vector<std::string> command_line_container;
    command_line_container.reserve(argc);

    std::vector<const char*> argv;
    argv.reserve(argc);

    for (int i = 0; i < argc; i++)
    {
      command_line_container.push_back(
        provider.ConsumeRandomLengthString(kMaxArgSize));
      argv.push_back(command_line_container[i].c_str());
    }

    // Parse command line;
    auto result = options.parse(argc, argv.data());
  } catch (...)
  {
  }

  return 0;
}
