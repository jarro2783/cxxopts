#include "cxxopts.hpp"

namespace yaco
{

//===========================================================================
// applicator

// The prototype of applicator.
// This way of using static functions in a class enables partial specialization.
template <class Action> struct applicator
{
  template <class Opt>
  static void apply(const Action &action, Opt &O) { action.Apply(O); }
};

//===========================================================================
// ApplyAction

template <typename Obj, typename Action>
void ApplyAction(Obj &O, const Action & action)
{
  applicator<Action>::apply(action, O);
}

template <typename Obj, typename Action, typename ... Actions>
void ApplyAction(Obj &O, const Action & action, const Actions & ... actions)
{
  ApplyAction(O, action);
  ApplyAction(O, actions...);
}

//===========================================================================
// Option

struct Opt : public cxxopts::Option
{
  template <class ... Action>
  explicit Opt(const Action & ... actions) : cxxopts::Option("", "")
  {
    ApplyAction(*this, actions...);
    Done();
  }

  ~Opt() override = default;

  void Done()
  {
  }

};

//===========================================================================
// Parser

struct Parser : public cxxopts::Options
{
  template <typename ... Actions>
  explicit Parser(const Actions & ... actions) : cxxopts::Options("")
  {
    ApplyAction(*this, actions...);
  }

  ~Parser() override = default;

  Options &program_name(std::string name)
  {
    this->m_program = std::move(name);
    return *this;
  }

};

//===========================================================================
// Exception

class option_multiset_error : public cxxopts::OptionSpecException
{
public:
  explicit option_multiset_error(const std::string& option)
      : OptionSpecException("Option " +
                            cxxopts::LQUOTE + option + cxxopts::RQUOTE +
                            " is set multiple times.")
  {
  }
};

}

namespace yaco  /// Applicators - the instance properties implicit modifier.
{

template <unsigned N> struct applicator <char[N]>
{
  static void apply(const char *str, Parser &Os)
  {
    Os.program_name(std::string(str));
  }

  static void apply(const char *str, Opt &O)
  {
    if (!O.opts_.empty())
    {
      cxxopts::throw_or_mimic<option_multiset_error>
          (std::string(str) + " (previous: " + O.opts_ + ")");
    }
    O.opts_ = std::string(str);
  }
};
}   /// yaco

namespace yaco  /// The instance properties explicit modifier.
{
  template <class T> struct type
  {
    void Apply(Opt &O) const
    {
      O.value_ = cxxopts::value<T>();
    }
  };

  struct desc
  {
    desc(const char *str) : desc_(std::string(str)) {}
    void Apply(Opt &O) const
    {
      O.desc_ = desc_;
    }
    std::string desc_;
  };

  struct inject
  {
    inject(Parser &Os) : Os_(Os) { }
    void Apply(Opt &O) const
    {
      Os_.add_option("", O);
    }
    Parser &Os_;
  };
}   // namespace yaco

yaco::Parser options("objectification-demo");
yaco::Opt help("h,help", yaco::desc("display help messages."), yaco::inject(options));
yaco::Opt job("j", yaco::desc("jobs"), yaco::type<int>(), yaco::inject(options));

int main(int argc, char **argv) {
  auto result = options.parse(argc, argv);
  if (result.count("help") > 0) {
    std::cout << options.help() << std::endl;
    exit(0);
  }
  return 0;
}