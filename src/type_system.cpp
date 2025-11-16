#include <cxxoptspp.hpp>
#include <sstream>
#include <stdexcept>
#include <regex>

namespace cxxoptspp {

// ============================
// Type System Implementation
// ============================

AbstractType::AbstractType(std::string name)
  : m_name(std::move(name)) {}

AbstractType::~AbstractType() = default;

std::string AbstractType::name() const {
  return m_name;
}

// ============================
// Built-in Type Implementations
// ============================

// StringType
template <>
std::string StringType::parse(const std::string& value) const {
  return value;
}

template <>
std::string StringType::serialize(const std::string& value) const {
  return value;
}

template <>
std::string StringType::to_string(const std::string& value) const {
  return value;
}

// IntType
template <>
int IntType::parse(const std::string& value) const {
  return std::stoi(value);
}

template <>
std::string IntType::serialize(const int& value) const {
  return std::to_string(value);
}

template <>
std::string IntType::to_string(const int& value) const {
  return std::to_string(value);
}

// BoolType
template <>
bool BoolType::parse(const std::string& value) const {
  std::string lower = value;
  std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
  return lower == "true" || lower == "1" || lower == "on" || lower == "yes";
}

template <>
std::string BoolType::serialize(const bool& value) const {
  return value ? "true" : "false";
}

template <>
std::string BoolType::to_string(const bool& value) const {
  return value ? "true" : "false";
}

// FloatType
template <>
float FloatType::parse(const std::string& value) const {
  return std::stof(value);
}

template <>
std::string FloatType::serialize(const float& value) const {
  return std::to_string(value);
}

template <>
std::string FloatType::to_string(const float& value) const {
  return std::to_string(value);
}

// IPAddressType
IPAddressType::IPAddressType()
  : AbstractType("ip_address") {}

std::any IPAddressType::parse(const std::string& value) const {
  std::regex ip_regex(R"((\d+)\.(\d+)\.(\d+)\.(\d+))");
  std::smatch match;
  
  if (!std::regex_match(value, match, ip_regex)) {
    throw std::invalid_argument("Invalid IP address: " + value);
  }
  
  IPAddress ip;
  for (int i = 1; i <= 4; ++i) {
    int octet = std::stoi(match[i]);
    if (octet < 0 || octet > 255) {
      throw std::invalid_argument("Invalid octet in IP address: " + value);
    }
    ip.octets[i-1] = octet;
  }
  
  return ip;
}

std::string IPAddressType::serialize(const std::any& value) const {
  const IPAddress& ip = std::any_cast<const IPAddress&>(value);
  std::ostringstream oss;
  oss << (int)ip.octets[0] << "." << (int)ip.octets[1] << "." << (int)ip.octets[2] << "." << (int)ip.octets[3];
  return oss.str();
}

std::string IPAddressType::to_string(const std::any& value) const {
  return serialize(value);
}

// DateTimeRangeType
DateTimeRangeType::DateTimeRangeType()
  : AbstractType("datetime_range") {}

std::any DateTimeRangeType::parse(const std::string& value) const {
  // Simple implementation for demo
  size_t dash_pos = value.find("-");
  if (dash_pos == std::string::npos) {
    throw std::invalid_argument("Invalid date range: " + value);
  }
  
  DateTimeRange range;
  range.start = value.substr(0, dash_pos);
  range.end = value.substr(dash_pos + 1);
  
  // Validate dates (simplified)
  // In real implementation, would use std::chrono or similar
  
  return range;
}

std::string DateTimeRangeType::serialize(const std::any& value) const {
  const DateTimeRange& range = std::any_cast<const DateTimeRange&>(value);
  return range.start + "-" + range.end;
}

std::string DateTimeRangeType::to_string(const std::any& value) const {
  return serialize(value);
}

// ============================
// Type Manager Implementation
// ============================

TypeManager& TypeManager::instance() {
  static TypeManager manager;
  return manager;
}

template <typename T>
void TypeManager::register_type(const std::shared_ptr<AbstractType>& type) {
  m_types[typeid(T).hash_code()] = type;
  m_type_names[type->name()] = type;
}

template <typename T>
std::shared_ptr<AbstractType> TypeManager::get_type() const {
  auto it = m_types.find(typeid(T).hash_code());
  if (it != m_types.end()) {
    return it->second;
  }
  return nullptr;
}

std::shared_ptr<AbstractType> TypeManager::get_type_by_name(const std::string& name) const {
  auto it = m_type_names.find(name);
  if (it != m_type_names.end()) {
    return it->second;
  }
  return nullptr;
}

// ============================
// Custom type parsers for cxxopts
// ============================

bool parse_value(const std::string& text, IPAddress& value) {
  IPAddressType type;
  try {
    value = std::any_cast<IPAddress>(type.parse(text));
    return true;
  } catch (const std::exception&) {
    return false;
  }
}

bool parse_value(const std::string& text, std::vector<IPAddress>& value) {
  IPAddressType type;
  
  // Split by comma
  size_t pos = 0;
  std::string token;
  std::string copy = text;
  
  while ((pos = copy.find(',')) != std::string::npos) {
    token = copy.substr(0, pos);
    try {
      value.push_back(std::any_cast<IPAddress>(type.parse(token)));
    } catch (const std::exception&) {
      return false;
    }
    copy.erase(0, pos + 1);
  }
  
  if (!copy.empty()) {
    try {
      value.push_back(std::any_cast<IPAddress>(type.parse(copy)));
    } catch (const std::exception&) {
      return false;
    }
  }
  
  return true;
}

bool parse_value(const std::string& text, DateTimeRange& value) {
  DateTimeRangeType type;
  try {
    value = std::any_cast<DateTimeRange>(type.parse(text));
    return true;
  } catch (const std::exception&) {
    return false;
  }
}

bool parse_value(const std::string& text, std::vector<DateTimeRange>& value) {
  DateTimeRangeType type;
  
  // Split by comma
  size_t pos = 0;
  std::string token;
  std::string copy = text;
  
  while ((pos = copy.find(',')) != std::string::npos) {
    token = copy.substr(0, pos);
    try {
      value.push_back(std::any_cast<DateTimeRange>(type.parse(token)));
    } catch (const std::exception&) {
      return false;
    }
    copy.erase(0, pos + 1);
  }
  
  if (!copy.empty()) {
    try {
      value.push_back(std::any_cast<DateTimeRange>(type.parse(copy)));
    } catch (const std::exception&) {
      return false;
    }
  }
  
  return true;
}

// ============================
// Collection Parser
// ============================

template <typename T>
bool CollectionParser<T>::parse(const std::string& value, char delimiter, std::vector<T>& result) {
  result.clear();
  
  std::stringstream ss(value);
  std::string token;
  
  while (std::getline(ss, token, delimiter)) {
    T parsed_value;
    if (!parse_value(token, parsed_value)) {
      return false;
    }
    result.push_back(std::move(parsed_value));
  }
  
  return true;
}

} // namespace cxxoptspp