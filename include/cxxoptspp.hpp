#ifndef CXXOPTSPP_HPP_INCLUDED
#define CXXOPTSPP_HPP_INCLUDED

#include <cxxopts.hpp>
#include <memory>
#include <map>
#include <vector>
#include <string>
#include <any>
#include <functional>
#include <regex>
#include <nlohmann/json.hpp>

namespace cxxoptspp {

using json = nlohmann::json;

// Forward declarations
class Command;
class CommandParser;
class Type;
class DependencyRule;
class ParseTree;

// ============================
// 1. Multi-level Subcommands
// ============================

class Command {
public:
  Command(std::string name, std::string description)
    : m_name(std::move(name)), m_description(std::move(description)), m_parent(nullptr), m_options(m_name, m_description) {}

  // Subcommand management
  std::shared_ptr<Command> add_subcommand(std::string name, std::string description = "");
  std::shared_ptr<Command> get_subcommand(const std::string& name) const;
  const CommandMap& subcommands() const;

  // Option management with inheritance support
  template <typename T>
  Command& add_option(std::string long_option, std::string description, T defaultValue, bool required = false) {
    // Add the option to the internal cxxopts::Options object
    m_options.add_option("", std::move(long_option), std::move(description), cxxopts::value<T>(defaultValue)->default_value(std::to_string(defaultValue)), required);
    
    // Save the option definition with type erasure
    auto def = std::make_unique<OptionDefinition<T>>(std::move(long_option), std::move(description), defaultValue, required);
    m_option_definitions.push_back(std::move(def));
    
    return *this;
  }
  
  Command& add_option(std::string long_option, std::string description, std::string defaultValue, bool required = false) {
    // Add the option to the internal cxxopts::Options object
    m_options.add_option("", std::move(long_option), std::move(description), cxxopts::value<std::string>(defaultValue)->default_value('"' + defaultValue + '"'), required);
    
    // Save the option definition with type erasure
    auto def = std::make_unique<OptionDefinition<std::string>>(std::move(long_option), std::move(description), std::move(defaultValue), required);
    m_option_definitions.push_back(std::move(def));
    
    return *this;
  }
  
  void parse_positional(const std::string& option);
  void parse_positional(std::vector<std::string> options);

  // Getter for option definitions
  const std::vector<OptionDefinition>& option_definitions() const;

  // Dependency management
  void add_dependency(std::string rule);
  const DependencyList& dependencies() const;

  // Getters
  const std::string& name() const;
  const std::string& description() const;
  const cxxopts::Options& options() const;
  cxxopts::Options& options();
  const Command* parent() const;
  
  // Getter for option definitions (for CommandParser only)
  const std::vector<std::unique_ptr<OptionDefinitionBase>>& option_definitions() const;

private:
  friend class CommandParser;

  std::string m_name;
  std::string m_description;
  const Command* m_parent;
  CommandMap m_subcommands;
  cxxopts::Options m_options;
  DependencyList m_dependencies;

  // For option inheritance with type erasure
  struct OptionDefinitionBase {
    std::string option;
    std::string description;
    std::string defaultValue;
    bool required;
    
    OptionDefinitionBase(std::string option, std::string description, std::string defaultValue, bool required)
      : option(std::move(option)), description(std::move(description)), defaultValue(std::move(defaultValue)), required(required) {}
    
    virtual ~OptionDefinitionBase() = default;
    virtual void add_to_options(cxxopts::Options& options) const = 0;
  };
  
  template <typename T>
  struct OptionDefinition : public OptionDefinitionBase {
    OptionDefinition(std::string option, std::string description, T defaultValue, bool required)
      : OptionDefinitionBase(std::move(option), std::move(description), std::to_string(defaultValue), required), defaultValue(defaultValue) {}
    
    void add_to_options(cxxopts::Options& options) const override {
      T value = defaultValue;
      options.add_option("", option, description, cxxopts::value<T>(value)->default_value(std::to_string(defaultValue)), required);
    }
    
    T defaultValue;
  };
  
  // Specialization for string type
  template <>
  struct OptionDefinition<std::string> : public OptionDefinitionBase {
    OptionDefinition(std::string option, std::string description, std::string defaultValue, bool required)
      : OptionDefinitionBase(std::move(option), std::move(description), defaultValue, required), defaultValue(std::move(defaultValue)) {}
    
    void add_to_options(cxxopts::Options& options) const override {
      std::string value = defaultValue;
      options.add_option("", option, description, cxxopts::value<std::string>(value)->default_value('"' + defaultValue + '"'), required);
    }
    
    std::string defaultValue;
  };
  
  std::vector<std::unique_ptr<OptionDefinitionBase>> m_option_definitions;
};

class CommandParser {
public:
  explicit CommandParser(Command& root_command);

  cxxopts::ParseResult parse(int argc, const char* const argv[]);
  cxxopts::ParseResult parse(const std::vector<std::string>& args);

  const Command& current_command() const;
  const cxxopts::ParseResult& result() const;

private:
  struct ParseContext {
    const Command* current_cmd;
    std::vector<std::string> remaining_args;
    cxxopts::Options merged_options;
    std::vector<std::shared_ptr<DependencyRule>> merged_dependencies;
  };

  void merge_options(const Command& cmd, cxxopts::Options& merged) const;
  void merge_dependencies(const Command& cmd, std::vector<std::shared_ptr<DependencyRule>>& merged) const;
  void validate_dependencies(const cxxopts::ParseResult& result) const;

  Command& m_root_command;
  const Command* m_current_command;
  cxxopts::ParseResult m_result;
};

// ============================
// 2. Type System
// ============================

class Type {
public:
  virtual ~Type() = default;

  virtual std::any from_string(const std::string& input) const = 0;
  virtual std::string to_string(const std::any& value) const = 0;
  virtual bool validate(const std::string& input) const = 0;
};

template <typename T>
class BasicType : public Type {
public:
  std::any from_string(const std::string& input) const override {
    T value;
    std::istringstream iss(input);
    iss >> value;
    if (!iss.eof()) {
      throw cxxopts::exceptions::incorrect_argument_type(input);
    }
    return value;
  }

  std::string to_string(const std::any& value) const override {
    std::ostringstream oss;
    oss << std::any_cast<const T&>(value);
    return oss.str();
  }

  bool validate(const std::string& input) const override {
    try {
      from_string(input);
      return true;
    } catch (...) {
      return false;
    }
  }
};

template <typename T>
class ContainerType : public Type {
public:
  explicit ContainerType(char delimiter = ',') : m_delimiter(delimiter) {}

  std::any from_string(const std::string& input) const override {
    std::vector<T> result;
    std::istringstream iss(input);
    std::string token;

    while (std::getline(iss, token, m_delimiter)) {
      BasicType<T> element_type;
      result.push_back(std::any_cast<T>(element_type.from_string(token)));
    }

    return result;
  }

  std::string to_string(const std::any& value) const override {
    const auto& container = std::any_cast<const std::vector<T>&>(value);
    std::ostringstream oss;
    BasicType<T> element_type;

    for (size_t i = 0; i < container.size(); ++i) {
      if (i > 0) oss << m_delimiter;
      oss << element_type.to_string(container[i]);
    }

    return oss.str();
  }

  bool validate(const std::string& input) const override {
    try {
      from_string(input);
      return true;
    } catch (...) {
      return false;
    }
  }

private:
  char m_delimiter;
};

// Example custom type: IP Address
class IPAddress {
public:
  IPAddress() = default;
  IPAddress(uint8_t a, uint8_t b, uint8_t c, uint8_t d)
    : octets{a, b, c, d} {}

  uint8_t octets[4];

  std::string to_string() const {
    std::ostringstream oss;
    oss << static_cast<int>(octets[0]) << "." 
        << static_cast<int>(octets[1]) << "." 
        << static_cast<int>(octets[2]) << "." 
        << static_cast<int>(octets[3]);
    return oss.str();
  }

  bool operator==(const IPAddress& other) const {
    return std::memcmp(octets, other.octets, 4) == 0;
  }
};

// Specialize parse_value for IPAddress
template<>
inline void parse_value(const std::string& text, IPAddress& value) {
  std::regex ip_regex(R"((\d+)\.(\d+)\.(\d+)\.(\d+))");
  std::smatch match;

  if (!std::regex_match(text, match, ip_regex)) {
    throw cxxopts::exceptions::incorrect_argument_type(text);
  }

  for (int i = 0; i < 4; ++i) {
    int octet = std::stoi(match[i+1]);
    if (octet < 0 || octet > 255) {
      throw cxxopts::exceptions::incorrect_argument_type(text);
    }
    value.octets[i] = static_cast<uint8_t>(octet);
  }
}

// Specialize parse_value for vector<IPAddress>
template<>
inline void parse_value(const std::string& text, std::vector<IPAddress>& value) {
  if (text.empty()) {
    return;
  }
  std::stringstream in(text);
  std::string token;
  while(!in.eof() && std::getline(in, token, CXXOPTS_VECTOR_DELIMITER)) {
    IPAddress ip;
    parse_value(token, ip);
    value.emplace_back(std::move(ip));
  }
}

class IPAddressType : public Type {
public:
  std::any from_string(const std::string& input) const override {
    return IPAddress::from_string(input);
  }

  std::string to_string(const std::any& value) const override {
    return std::any_cast<const IPAddress&>(value).to_string();
  }

  bool validate(const std::string& input) const override {
    try {
      IPAddress::from_string(input);
      return true;
    } catch (...) {
      return false;
    }
  }
};

// ============================
// 3. Dependency Rules Engine
// ============================

class DependencyRule {
public:
  virtual ~DependencyRule() = default;
  virtual bool evaluate(const cxxopts::ParseResult& result) const = 0;
  virtual std::string error_message() const = 0;
};

class SimpleDependencyRule : public DependencyRule {
public:
  explicit SimpleDependencyRule(std::string rule_str);
  bool evaluate(const cxxopts::ParseResult& result) const override;
  std::string error_message() const override;

private:
  enum class TokenType { AND, OR, NOT, LPAREN, RPAREN, OPTION, END };

  struct Token {
    TokenType type;
    std::string value;
  };

  std::vector<Token> tokenize(const std::string& rule) const;
  std::vector<Token> infix_to_postfix(const std::vector<Token>& infix) const;
  bool evaluate_postfix(const std::vector<Token>& postfix, const cxxopts::ParseResult& result) const;

  std::string m_rule_str;
  std::vector<Token> m_postfix;
  std::string m_error_msg;
};

// ============================
// 4. Reversible Parsing & Serialization
// ============================

class ParseNode {
public:
  enum class NodeType { ROOT, COMMAND, OPTION, VALUE, POSITIONAL };

  ParseNode(NodeType type, std::string value, const Command* cmd = nullptr)
    : m_type(type), m_value(std::move(value)), m_command(cmd) {}

  // Add child node
  void add_child(std::shared_ptr<ParseNode> child) {
    m_children.push_back(std::move(child));
  }

  // Serialization
  json to_json() const;

  // Deserialization
  static std::shared_ptr<ParseNode> from_json(const json& j, const Command* root_cmd);

  // Reconstruct command line
  std::vector<std::string> to_command_line() const;

private:
  NodeType m_type;
  std::string m_value;
  const Command* m_command;
  std::vector<std::shared_ptr<ParseNode>> m_children;
};

class ParseTree {
public:
  ParseTree() : m_root(std::make_shared<ParseNode>(ParseNode::NodeType::ROOT, "")) {}

  // Add root command
  void set_root_command(std::shared_ptr<Command> cmd) {
    m_root->add_child(std::make_shared<ParseNode>(ParseNode::NodeType::COMMAND, cmd->name(), cmd.get()));
    m_current_node = m_root->m_children.back().get();
  }

  // Add subcommand
  void add_subcommand(std::shared_ptr<Command> cmd) {
    auto child = std::make_shared<ParseNode>(ParseNode::NodeType::COMMAND, cmd->name(), cmd.get());
    m_current_node->add_child(std::move(child));
    m_current_node = m_current_node->m_children.back().get();
  }

  // Add option
  void add_option(const std::string& option_str, const Command* cmd = nullptr) {
    m_current_node->add_child(std::make_shared<ParseNode>(ParseNode::NodeType::OPTION, option_str, cmd));
  }

  // Add value
  void add_value(const std::string& value_str, const Command* cmd = nullptr) {
    m_current_node->add_child(std::make_shared<ParseNode>(ParseNode::NodeType::VALUE, value_str, cmd));
  }

  // Serialization
  json to_json() const { return m_root->to_json(); }

  // Deserialization
  static ParseTree from_json(const json& j, const Command* root_cmd);

  // Reconstruct command line
  std::vector<std::string> to_command_line() const { return m_root->to_command_line(); }

private:
  std::shared_ptr<ParseNode> m_root;
  ParseNode* m_current_node;
};

// Helper functions for option creation with custom types
template <typename T>
std::shared_ptr<cxxopts::Value> value() {
  return std::make_shared<cxxopts::values::standard_value<T>>();
}

template <typename T>
std::shared_ptr<cxxopts::Value> value(T& t) {
  return std::make_shared<cxxopts::values::standard_value<T>>(&t);
}

// Custom value with type validation
template <typename T>
class TypedValue : public cxxopts::Value {
public:
  TypedValue() : m_result(std::make_shared<T>()), m_store(m_result.get()) {}
  explicit TypedValue(T* t) : m_store(t) {}

  void add(const std::string& text) const override {
    // Not implemented for custom types
    throw cxxopts::exceptions::option_has_no_value("TypedValue");
  }

  void parse(const std::string& text) const override {
    BasicType<T> type;
    *m_store = std::any_cast<T>(type.from_string(text));
  }

  bool is_container() const override { return false; }
  void parse() const override { /* no default */ }
  bool has_default() const override { return false; }
  bool has_implicit() const override { return false; }

  std::shared_ptr<cxxopts::Value> default_value(const std::string&) override { return shared_from_this(); }
  std::shared_ptr<cxxopts::Value> implicit_value(const std::string&) override { return shared_from_this(); }
  std::shared_ptr<cxxopts::Value> no_implicit_value() override { return shared_from_this(); }
  std::string get_default_value() const override { return ""; }
  std::string get_implicit_value() const override { return ""; }
  bool is_boolean() const override { return false; }
  std::shared_ptr<cxxopts::Value> clone() const override {
    return std::make_shared<TypedValue<T>>(*this);
  }

  const T& get() const { return *m_store; }

private:
  std::shared_ptr<T> m_result;
  T* m_store;
};

// Export to namespace cxxopts for backward compatibility
namespace cxxopts {
  using cxxoptspp::Command;
  using cxxoptspp::CommandParser;
  using cxxoptspp::Type;
  using cxxoptspp::BasicType;
  using cxxoptspp::ContainerType;
  using cxxoptspp::IPAddress;
  using cxxoptspp::IPAddressType;
  using cxxoptspp::DependencyRule;
  using cxxoptspp::SimpleDependencyRule;
  using cxxoptspp::ParseTree;
  using cxxoptspp::TypedValue;
}

} // namespace cxxoptspp

#endif // CXXOPTSPP_HPP_INCLUDED