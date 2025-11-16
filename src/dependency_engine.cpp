#include <cxxoptspp.hpp>
#include <sstream>
#include <stdexcept>
#include <stack>
#include <algorithm>
#include <regex>

namespace cxxoptspp {

// Enhanced DependencyRule with value comparison support

class EnhancedDependencyRule : public DependencyRule {
public:
  explicit EnhancedDependencyRule(std::string rule_str)
    : m_rule_str(std::move(rule_str)) {
    auto tokens = tokenize(m_rule_str);
    m_postfix = infix_to_postfix(tokens);
    m_error_msg = "Dependency violation: " + m_rule_str;
  }

  bool evaluate(const cxxopts::ParseResult& result) const override {
    return evaluate_postfix(m_postfix, result);
  }

  std::string error_message() const override {
    return m_error_msg;
  }

private:
  enum class TokenType {
    AND, OR, NOT, 
    LPAREN, RPAREN, 
    OPTION, 
    EQ, NE, GT, LT, GE, LE, 
    VALUE, 
    END
  };

  struct Token {
    TokenType type;
    std::string value;
  };

  std::vector<Token> tokenize(const std::string& rule) const {
    std::vector<Token> tokens;
    std::string input = rule;

    // Handle operators with two characters first
    std::vector<std::pair<std::string, TokenType>> operators = {
      {"&&", TokenType::AND},
      {"||", TokenType::OR},
      {"==", TokenType::EQ},
      {"!=", TokenType::NE},
      {">=", TokenType::GE},
      {"<=", TokenType::LE},
      {">", TokenType::GT},
      {"<", TokenType::LT},
      {"!", TokenType::NOT},
      {"(", TokenType::LPAREN},
      {")", TokenType::RPAREN}
    };

    size_t pos = 0;
    while (pos < input.size()) {
      // Skip whitespace
      while (pos < input.size() && std::isspace(input[pos])) {
        ++pos;
      }
      if (pos >= input.size()) break;

      bool found = false;

      // Check for operators
      for (const auto& [op_str, op_type] : operators) {
        if (input.substr(pos, op_str.size()) == op_str) {
          tokens.push_back({op_type, op_str});
          pos += op_str.size();
          found = true;
          break;
        }
      }
      if (found) continue;

      // Check for quoted values
      if (input[pos] == '"' || input[pos] == "'") {
        char quote = input[pos++];
        size_t end_pos = input.find(quote, pos);
        if (end_pos == std::string::npos) {
          throw std::invalid_argument("Unclosed quote in rule: " + rule);
        }
        std::string val = input.substr(pos, end_pos - pos);
        tokens.push_back({TokenType::VALUE, val});
        pos = end_pos + 1;
        continue;
      }

      // Check for options or values
      size_t end_pos = pos;
      while (end_pos < input.size() && 
             !std::isspace(input[end_pos]) &&
             input[end_pos] != '"' && input[end_pos] != "'" &&
             input.substr(end_pos, 2) != "&&" && input.substr(end_pos, 2) != "||" &&
             input.substr(end_pos, 2) != "==" && input.substr(end_pos, 2) != "!=" &&
             input.substr(end_pos, 2) != ">=" && input.substr(end_pos, 2) != "<=" &&
             input[end_pos] != '>' && input[end_pos] != '<' &&
             input[end_pos] != '!' && input[end_pos] != '(' && input[end_pos] != ')') {
        ++end_pos;
      }

      std::string token_str = input.substr(pos, end_pos - pos);
      if (!token_str.empty()) {
        // Check if it's an option (starts with -- or -)
        if (token_str.starts_with("--")) {
          tokens.push_back({TokenType::OPTION, token_str.substr(2)});
        } else if (token_str.starts_with("-")) {
          tokens.push_back({TokenType::OPTION, token_str.substr(1)});
        } else {
          // Assume it's a value
          tokens.push_back({TokenType::VALUE, token_str});
        }
      }

      pos = end_pos;
    }

    tokens.push_back({TokenType::END, ""});
    return tokens;
  }

  std::vector<Token> infix_to_postfix(const std::vector<Token>& infix) const {
    std::vector<Token> postfix;
    std::stack<Token> operators;

    auto precedence = [](TokenType type) -> int {
      if (type == TokenType::NOT) return 5;
      if (type == TokenType::GT || type == TokenType::LT || type == TokenType::GE || type == TokenType::LE) return 4;
      if (type == TokenType::EQ || type == TokenType::NE) return 3;
      if (type == TokenType::AND) return 2;
      if (type == TokenType::OR) return 1;
      return 0;
    };

    for (const auto& token : infix) {
      switch (token.type) {
        case TokenType::OPTION:
        case TokenType::VALUE:
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
        case TokenType::EQ:
        case TokenType::NE:
        case TokenType::GT:
        case TokenType::LT:
        case TokenType::GE:
        case TokenType::LE:
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

  bool evaluate_postfix(const std::vector<Token>& postfix, const cxxopts::ParseResult& result) const {
    // Stack holds either booleans (for existence checks) or strings (for option names/values)
    std::stack<std::variant<bool, std::string>> values;

    for (const auto& token : postfix) {
      switch (token.type) {
        case TokenType::OPTION:
        {
          // For options, we need to store the name to retrieve the value later if needed
          values.push(token.value);
          break;
        }
        case TokenType::VALUE:
        {
          values.push(token.value);
          break;
        }
        case TokenType::AND:
        case TokenType::OR:
        case TokenType::NOT:
        {
          // For logical operations, we need boolean values
          // Convert any string option names to existence booleans
          auto convert_to_bool = [&result](std::variant<bool, std::string>& var) -> bool {
            if (std::holds_alternative<bool>(var)) {
              return std::get<bool>(var);
            }
            const std::string& opt_name = std::get<std::string>(var);
            return result.count(opt_name) > 0;
          };

          if (token.type == TokenType::NOT) {
            auto val_var = values.top(); values.pop();
            bool val = convert_to_bool(val_var);
            values.push(!val);
          } else {
            auto rhs_var = values.top(); values.pop();
            auto lhs_var = values.top(); values.pop();
            
            bool lhs = convert_to_bool(lhs_var);
            bool rhs = convert_to_bool(rhs_var);
            
            if (token.type == TokenType::AND) {
              values.push(lhs && rhs);
            } else { // OR
              values.push(lhs || rhs);
            }
          }
          break;
        }
        case TokenType::EQ:
        case TokenType::NE:
        case TokenType::GT:
        case TokenType::LT:
        case TokenType::GE:
        case TokenType::LE:
        {
          auto rhs_var = values.top(); values.pop();
          auto lhs_var = values.top(); values.pop();
          
          std::string lhs_val, rhs_val;
          
          // Resolve lhs - could be an option name or a value
          if (std::holds_alternative<std::string>(lhs_var)) {
            const std::string& lhs_str = std::get<std::string>(lhs_var);
            // Check if it's an option (exists in result)
            if (result.count(lhs_str) > 0) {
              // It's an option - get its value from the result
              lhs_val = result[lhs_str].as<std::string>();
            } else {
              // It's a literal value
              lhs_val = lhs_str;
            }
          } else {
            // This should not happen for comparison operations
            values.push(false);
            break;
          }
          
          // Resolve rhs - could be an option name or a value
          if (std::holds_alternative<std::string>(rhs_var)) {
            const std::string& rhs_str = std::get<std::string>(rhs_var);
            // Check if it's an option (exists in result)
            if (result.count(rhs_str) > 0) {
              // It's an option - get its value from the result
              rhs_val = result[rhs_str].as<std::string>();
            } else {
              // It's a literal value
              rhs_val = rhs_str;
            }
          } else {
            // This should not happen for comparison operations
            values.push(false);
            break;
          }
          
          // Perform the appropriate comparison
          bool result_val = false;
          switch (token.type) {
            case TokenType::EQ:
              result_val = (lhs_val == rhs_val);
              break;
            case TokenType::NE:
              result_val = (lhs_val != rhs_val);
              break;
            case TokenType::GT:
              result_val = (lhs_val > rhs_val);
              break;
            case TokenType::LT:
              result_val = (lhs_val < rhs_val);
              break;
            case TokenType::GE:
              result_val = (lhs_val >= rhs_val);
              break;
            case TokenType::LE:
              result_val = (lhs_val <= rhs_val);
              break;
          }
          
          values.push(result_val);
          break;
        }
        default:
          // Not implemented yet
          values.push(false);
          break;
      }
    }

    // Ensure the final result is a boolean
    if (values.empty()) return false;
    if (std::holds_alternative<bool>(values.top())) {
      return std::get<bool>(values.top());
    }
    // Last token was an option name, convert to existence check
    return result.count(std::get<std::string>(values.top())) > 0;
  }

  std::string m_rule_str;
  std::vector<Token> m_postfix;
  std::string m_error_msg;
};

// Update Command to use EnhancedDependencyRule
void Command::add_dependency(std::string rule) {
  m_dependencies.push_back(std::make_shared<EnhancedDependencyRule>(std::move(rule)));
}

} // namespace cxxoptspp