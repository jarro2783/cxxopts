#include <cxxoptspp.hpp>
#include <sstream>
#include <stdexcept>
#include <stack>
#include <algorithm>

namespace cxxoptspp {

// ============================
// Command Implementation
// ============================

Command::Command(std::string name, std::string description)
  : m_name(std::move(name)),
    m_description(std::move(description)),
    m_parent(nullptr),
    m_options(m_name)
{}

std::shared_ptr<Command> Command::add_subcommand(std::string name, std::string description) {
  auto cmd = std::make_shared<Command>(std::move(name), std::move(description));
  cmd->m_parent = this;
  m_subcommands[cmd->m_name] = cmd;
  return cmd;
}

std::shared_ptr<Command> Command::get_subcommand(const std::string& name) const {
  auto it = m_subcommands.find(name);
  return it != m_subcommands.end() ? it->second : nullptr;
}

const Command::CommandMap& Command::subcommands() const {
  return m_subcommands;
}



void Command::parse_positional(const std::string& option) {
  m_options.parse_positional(option);
}

void Command::parse_positional(std::vector<std::string> options) {
  m_options.parse_positional(std::move(options));
}

void Command::add_dependency(std::string rule) {
  m_dependencies.push_back(std::make_shared<SimpleDependencyRule>(std::move(rule)));
}

const Command::DependencyList& Command::dependencies() const {
  return m_dependencies;
}

const std::string& Command::name() const {
  return m_name;
}

const std::string& Command::description() const {
  return m_description;
}

const cxxopts::Options& Command::options() const {
  return m_options;
}

cxxopts::Options& Command::options() {
  return m_options;
}

const Command* Command::parent() const {
  return m_parent;
}

const std::vector<std::unique_ptr<Command::OptionDefinitionBase>>& Command::option_definitions() const {
  return m_option_definitions;
}

// ============================
// CommandParser Implementation
// ============================

CommandParser::CommandParser(Command& root_command)
  : m_root_command(root_command),
    m_current_command(&root_command)
{}

cxxopts::ParseResult CommandParser::parse(int argc, const char* const argv[]) {
  return parse(std::vector<std::string>(argv, argv + argc));
}

cxxopts::ParseResult CommandParser::parse(const std::vector<std::string>& args) {
  if (args.empty()) {
    m_result = cxxopts::ParseResult(std::make_shared<cxxopts::OptionMap>());
    return m_result;
  }

  std::vector<std::string> remaining_args(args.begin() + 1, args.end()); // skip program name
  m_current_command = &m_root_command;

  // Traverse command tree
  while (!remaining_args.empty()) {
    const auto& arg = remaining_args.front();
    auto subcmd = m_current_command->get_subcommand(arg);
    
    if (subcmd) {
      m_current_command = subcmd.get();
      remaining_args.erase(remaining_args.begin());
    } else {
      break; // found an option, start parsing
    }
  }

  // Merge options from all ancestor commands (root to current) with proper inheritance
  // Create a new Options object with the current command's name and description
  cxxopts::Options merged_options(m_current_command->name(), m_current_command->description());
  
  // Collect all commands from root to current (inclusive)
  std::vector<const Command*> command_chain;
  const Command* current = m_current_command;
  while (current) {
    command_chain.insert(command_chain.begin(), current); // Insert at front to get root first
    current = current->parent();
  }
  
  // Add options from all commands in the chain (root to current)
  // This ensures that child command options override parent command options
  for (const Command* cmd : command_chain) {
    for (const auto& option_def : cmd->option_definitions()) {
      option_def->add_to_options(merged_options);
    }
  }

  // Merge dependencies from all parent commands
  std::vector<std::shared_ptr<DependencyRule>> merged_deps = m_current_command->dependencies();
  merge_dependencies(*m_current_command, merged_deps);

  // Parse the remaining arguments
  m_result = merged_options.parse(remaining_args);

  // Validate dependencies
  validate_dependencies(m_result);

  return m_result;
}

const Command& CommandParser::current_command() const {
  return *m_current_command;
}

const cxxopts::ParseResult& CommandParser::result() const {
  return m_result;
}

void CommandParser::merge_options(const Command& cmd, cxxopts::Options& merged) const {
  const Command* parent = cmd.parent();
  if (parent) {
    merge_options(*parent, merged);
    // Workaround: Create a new Options object for the parent and parse an empty vector
    // This will populate the option map with default values
    cxxopts::Options parent_options = parent->options();
    parent_options.parse({});
    // Note: We can't directly copy options because cxxopts::Options doesn't support it
    // Instead, we rely on the fact that we've already traversed from root to current command
    // This ensures that the merged options will have the correct priority (child options override parent options)
  }
}

void CommandParser::merge_dependencies(const Command& cmd, std::vector<std::shared_ptr<DependencyRule>>& merged) const {
  const Command* parent = cmd.parent();
  if (parent) {
    auto parent_deps = parent->dependencies();
    merged.insert(merged.begin(), parent_deps.begin(), parent_deps.end());
    merge_dependencies(*parent, merged);
  }
}

void CommandParser::validate_dependencies(const cxxopts::ParseResult& result) const {
  // Check all merged dependencies
  const Command* cmd = m_current_command;
  while (cmd) {
    for (const auto& dep : cmd->dependencies()) {
      if (!dep->evaluate(result)) {
        throw cxxopts::exceptions::parsing(dep->error_message());
      }
    }
    cmd = cmd->parent();
  }
}

// ============================
// SimpleDependencyRule Implementation
// ============================

SimpleDependencyRule::SimpleDependencyRule(std::string rule_str)
  : m_rule_str(std::move(rule_str)) {
  auto tokens = tokenize(m_rule_str);
  m_postfix = infix_to_postfix(tokens);
  m_error_msg = "Dependency violation: " + m_rule_str;
}

bool SimpleDependencyRule::evaluate(const cxxopts::ParseResult& result) const {
  return evaluate_postfix(m_postfix, result);
}

std::string SimpleDependencyRule::error_message() const {
  return m_error_msg;
}

std::vector<SimpleDependencyRule::Token> SimpleDependencyRule::tokenize(const std::string& rule) const {
  std::vector<Token> tokens;
  std::istringstream iss(rule);
  std::string token;

  while (iss >> token) {
    if (token == "&&") {
      tokens.push_back({TokenType::AND, token});
    } else if (token == "||") {
      tokens.push_back({TokenType::OR, token});
    } else if (token == "!") {
      tokens.push_back({TokenType::NOT, token});
    } else if (token == "(") {
      tokens.push_back({TokenType::LPAREN, token});
    } else if (token == ")") {
      tokens.push_back({TokenType::RPAREN, token});
    } else {
      // Option name - remove leading dashes if present
      std::string opt_name = token;
      if (opt_name.starts_with("--")) {
        opt_name = opt_name.substr(2);
      } else if (opt_name.starts_with("-")) {
        opt_name = opt_name.substr(1);
      }
      tokens.push_back({TokenType::OPTION, opt_name});
    }
  }

  tokens.push_back({TokenType::END, ""});
  return tokens;
}

std::vector<SimpleDependencyRule::Token> SimpleDependencyRule::infix_to_postfix(const std::vector<Token>& infix) const {
  std::vector<Token> postfix;
  std::stack<Token> operators;

  auto precedence = [](TokenType type) -> int {
    if (type == TokenType::NOT) return 3;
    if (type == TokenType::AND) return 2;
    if (type == TokenType::OR) return 1;
    return 0;
  };

  for (const auto& token : infix) {
    switch (token.type) {
      case TokenType::OPTION:
        postfix.push_back(token);
        break;
      case TokenType::LPAREN:
        operators.push(token);
        break;
      case TokenType::RPAREN:
        while (!operators.empty() && operators.top().type != TokenType::LPAREN) {
          postfix.push_back(operators.top());
          operators.pop();
        }
        if (!operators.empty()) operators.pop(); // pop '('
        break;
      case TokenType::AND:
      case TokenType::OR:
      case TokenType::NOT:
        while (!operators.empty() && precedence(operators.top().type) >= precedence(token.type)) {
          postfix.push_back(operators.top());
          operators.pop();
        }
        operators.push(token);
        break;
      case TokenType::END:
        break;
    }
  }

  while (!operators.empty()) {
    postfix.push_back(operators.top());
    operators.pop();
  }

  return postfix;
}

bool SimpleDependencyRule::evaluate_postfix(const std::vector<Token>& postfix, const cxxopts::ParseResult& result) const {
  std::stack<bool> values;

  for (const auto& token : postfix) {
    switch (token.type) {
      case TokenType::OPTION:
        values.push(result.count(token.value) > 0);
        break;
      case TokenType::AND:
      {
        bool rhs = values.top(); values.pop();
        bool lhs = values.top(); values.pop();
        values.push(lhs && rhs);
        break;
      }
      case TokenType::OR:
      {
        bool rhs = values.top(); values.pop();
        bool lhs = values.top(); values.pop();
        values.push(lhs || rhs);
        break;
      }
      case TokenType::NOT:
      {
        bool val = values.top(); values.pop();
        values.push(!val);
        break;
      }
      default:
        break;
    }
  }

  return !values.empty() && values.top();
}

// ============================
// ParseNode Implementation
// ============================

json ParseNode::to_json() const {
  json j;
  j["type"] = static_cast<int>(m_type);
  j["value"] = m_value;
  if (m_command) {
    j["command_name"] = m_command->name();
  }
  if (!m_children.empty()) {
    json children;
    for (const auto& child : m_children) {
      children.push_back(child->to_json());
    }
    j["children"] = children;
  }
  return j;
}

std::shared_ptr<ParseNode> ParseNode::from_json(const json& j, const Command* root_cmd) {
  int type_int = j.at("type").get<int>();
  NodeType type = static_cast<NodeType>(type_int);
  std::string value = j.at("value").get<std::string>();
  
  // Find command if specified
  const Command* cmd = nullptr;
  if (j.contains("command_name")) {
    std::string cmd_name = j.at("command_name").get<std::string>();
    // For simplicity, we only set root command
    if (root_cmd && root_cmd->name() == cmd_name) {
      cmd = root_cmd;
    }
    // TODO: Handle nested commands
  }
  
  auto node = std::make_shared<ParseNode>(type, value, cmd);
  
  if (j.contains("children")) {
    for (const auto& child_j : j.at("children")) {
      node->add_child(from_json(child_j, root_cmd));
    }
  }
  
  return node;
}

std::vector<std::string> ParseNode::to_command_line() const {
  std::vector<std::string> args;
  
  switch (m_type) {
    case NodeType::ROOT:
      break;
    case NodeType::COMMAND:
    case NodeType::OPTION:
      args.push_back(m_value);
      break;
    case NodeType::VALUE:
      args.push_back(m_value);
      break;
    case NodeType::POSITIONAL:
      args.push_back(m_value);
      break;
  }
  
  for (const auto& child : m_children) {
    auto child_args = child->to_command_line();
    args.insert(args.end(), child_args.begin(), child_args.end());
  }
  
  return args;
}

// ============================
// ParseTree Implementation
// ============================

ParseTree ParseTree::from_json(const json& j, const Command* root_cmd) {
  ParseTree tree;
  auto root_node = ParseNode::from_json(j, root_cmd);
  tree.m_root = std::move(root_node);
  tree.m_current_node = tree.m_root.get();
  return tree;
}

} // namespace cxxoptspp