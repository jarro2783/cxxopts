#include <cxxoptspp.hpp>
#include <nlohmann/json.hpp>
#include <sstream>
#include <iomanip>
#include <algorithm>

namespace cxxoptspp {

using json = nlohmann::json;

// ============================
// ParseNode Implementation
// ============================

ParseNode::ParseNode()
  : type(Type::ROOT) {}

ParseNode::ParseNode(std::string name)
  : type(Type::COMMAND), name(std::move(name)) {}

ParseNode::ParseNode(std::string name, std::string value)
  : type(Type::OPTION), name(std::move(name)), value(std::move(value)) {}

json ParseNode::to_json() const {
  json j;
  
  j["type"] = static_cast<int>(type);
  
  if (!name.empty()) {
    j["name"] = name;
  }
  
  if (!value.empty()) {
    j["value"] = value;
  }
  
  if (!children.empty()) {
    json j_children = json::array();
    for (const auto& child : children) {
      j_children.push_back(child.to_json());
    }
    j["children"] = j_children;
  }
  
  if (!metadata.empty()) {
    j["metadata"] = metadata;
  }
  
  return j;
}

ParseNode ParseNode::from_json(const json& j) {
  ParseNode node;
  
  int type_int = j.at("type").get<int>();
  node.type = static_cast<ParseNode::Type>(type_int);
  
  if (j.contains("name")) {
    node.name = j.at("name").get<std::string>();
  }
  
  if (j.contains("value")) {
    node.value = j.at("value").get<std::string>();
  }
  
  if (j.contains("children")) {
    for (const auto& j_child : j.at("children")) {
      node.children.push_back(from_json(j_child));
    }
  }
  
  if (j.contains("metadata")) {
    node.metadata = j.at("metadata").get<json>();
  }
  
  return node;
}

std::vector<std::string> ParseNode::to_command_line() const {
  std::vector<std::string> args;
  
  switch (type) {
    case Type::ROOT:
      for (const auto& child : children) {
        auto child_args = child.to_command_line();
        args.insert(args.end(), child_args.begin(), child_args.end());
      }
      break;
      
    case Type::COMMAND:
      args.push_back(name);
      for (const auto& child : children) {
        auto child_args = child.to_command_line();
        args.insert(args.end(), child_args.begin(), child_args.end());
      }
      break;
      
    case Type::OPTION:
      if (value.empty()) {
        // Boolean option, just use --name
        args.push_back("--" + name);
      } else {
        // Value option, use --name=value or --name value
        // Use the shorter form when possible
        if (value.find(' ') == std::string::npos) {
          args.push_back("--" + name + "=" + value);
        } else {
          args.push_back("--" + name);
          args.push_back(value);
        }
      }
      break;
  }
  
  return args;
}

// ============================
// ParseTree Implementation
// ============================

ParseTree::ParseTree()
  : m_root(std::make_shared<ParseNode>()) {}

json ParseTree::to_json() const {
  return m_root->to_json();
}

void ParseTree::from_json(const json& j) {
  m_root = std::make_shared<ParseNode>(ParseNode::from_json(j));
}

std::string ParseTree::to_string() const {
  return to_json().dump(2);
}

std::vector<std::string> ParseTree::to_command_line() const {
  return m_root->to_command_line();
}

// ============================
// StateSerializer Implementation
// ============================

std::string StateSerializer::serialize(const CommandParser& parser, const cxxopts::ParseResult& result) {
  json j;
  
  // Serialize options with original input strings
  for (const auto& [opt_name, opt_value] : parser.original_inputs()) {
    j["original_inputs"][opt_name] = opt_value;
  }
  
  // Serialize command hierarchy
  if (parser.current_command()) {
    j["command_hierarchy"] = parser.command_hierarchy();
  }
  
  // Serialize option definitions (simplified)
  // In real implementation, we'd need to track all option metadata
  
  return j.dump(2);
}

std::string StateSerializer::serialize(const ParseTree& tree) {
  return tree.to_string();
}

std::vector<std::string> StateSerializer::deserialize_to_command_line(const std::string& serialized) {
  json j = json::parse(serialized);
  ParseTree tree;
  tree.from_json(j);
  return tree.to_command_line();
}

// ============================
// CommandParser Implementation for reversible parsing
// ============================

void CommandParser::trace_option(const std::string& opt_name, const std::string& opt_value) {
  m_original_inputs[opt_name] = opt_value;
}

std::unordered_map<std::string, std::string> CommandParser::original_inputs() const {
  return m_original_inputs;
}

std::vector<std::string> CommandParser::command_hierarchy() const {
  std::vector<std::string> hierarchy;
  
  auto cmd = m_current_command;
  while (cmd) {
    hierarchy.push_back(cmd->name());
    cmd = cmd->parent();
  }
  
  std::reverse(hierarchy.begin(), hierarchy.end());
  return hierarchy;
}

// =============================================================
// Additional helper functions for JSON serialization/deserialization
// =============================================================

json Command::to_json() const {
  json j;
  
  j["name"] = m_name;
  j["description"] = m_description;
  
  if (m_parent) {
    j["parent"] = m_parent->name();
  }
  
  // Serialize subcommands
  json subcommands = json::array();
  for (const auto& [name, cmd] : m_subcommands) {
    subcommands.push_back(cmd->to_json());
  }
  j["subcommands"] = subcommands;
  
  return j;
}

json Options::to_json() const {
  json j;
  
  // This would need to access the internal option details from cxxopts
  // For demonstration purposes, we'll keep it simple
  j["options"] = "... internal options ...";
  
  return j;
}

} // namespace cxxoptspp