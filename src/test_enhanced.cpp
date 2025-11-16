#include <iostream>
#include <vector>
#include <cxxoptspp.hpp>

int main() {
  // Test 1: Command hierarchy
  std::cout << "Test 1: Command hierarchy" << std::endl;
  try {
    cxxoptspp::Command root_cmd("tool", "A tool with subcommands");
    auto& module_cmd = root_cmd.add_subcommand("module", "A module command");
    auto& subcmd_cmd = module_cmd.add_subcommand("subcmd", "A subcommand");
    
    subcmd_cmd.add_option<int>("--opt", "An option");
    
    cxxoptspp::CommandParser parser(root_cmd);
    
    int argc = 4;
    const char* argv[] = {"tool", "module", "subcmd", "--opt=42"};
    
    auto result = parser.parse(argc, argv);
    if (result.count("opt")) {
      std::cout << "✓ Parsed --opt=" << result["opt"].as<int>() << std::endl;
    } else {
      std::cout << "✗ Failed to parse --opt" << std::endl;
    }
    
  } catch (const std::exception& e) {
    std::cout << "✗ Exception: " << e.what() << std::endl;
  }
  
  // Test 2: Dependency rules
  std::cout << std::endl << "Test 2: Dependency rules" << std::endl;
  try {
    cxxoptspp::Command cmd("test", "Dependency test");
    cmd.add_option<bool>("--verbose", "Verbose output");
    cmd.add_option<bool>("--debug", "Debug output");
    
    // Add rule: --verbose requires --debug
    cmd.add_dependency("--verbose ==> --debug");
    
    cxxoptspp::CommandParser parser(cmd);
    
    // This should fail because --verbose is used without --debug
    int argc = 2;
    const char* argv[] = {"test", "--verbose"};
    
    try {
      auto result = parser.parse(argc, argv);
      std::cout << "✗ Dependency rule should have failed but passed" << std::endl;
    } catch (const cxxopts::exceptions::parsing& e) {
      std::cout << "✓ Dependency rule correctly caught violation: " << e.what() << std::endl;
    }
    
  } catch (const std::exception& e) {
    std::cout << "✗ Exception: " << e.what() << std::endl;
  }
  
  // Test 3: Custom type IPAddress
  std::cout << std::endl << "Test 3: Custom type IPAddress" << std::endl;
  try {
    cxxoptspp::Command cmd("test", "IP Address test");
    cmd.add_option<cxxoptspp::IPAddress>("--ip", "An IP address");
    cmd.add_option<std::vector<cxxoptspp::IPAddress>>("--ips", "Multiple IP addresses");
    
    cxxoptspp::CommandParser parser(cmd);
    
    int argc = 3;
    const char* argv[] = {"test", "--ip=192.168.1.1", "--ips=10.0.0.1,10.0.0.2"};
    
    auto result = parser.parse(argc, argv);
    
    if (result.count("ip")) {
      auto ip = result["ip"].as<cxxoptspp::IPAddress>();
      std::cout << "✓ Parsed single IP: " << static_cast<int>(ip.octets[0]) << "." 
                << static_cast<int>(ip.octets[1]) << "." 
                << static_cast<int>(ip.octets[2]) << "." 
                << static_cast<int>(ip.octets[3]) << std::endl;
    } else {
      std::cout << "✗ Failed to parse single IP" << std::endl;
    }
    
    if (result.count("ips")) {
      auto ips = result["ips"].as<std::vector<cxxoptspp::IPAddress>>();
      std::cout << "✓ Parsed multiple IPs (" << ips.size() << "):" << std::endl;
      for (const auto& ip : ips) {
        std::cout << "  - " << static_cast<int>(ip.octets[0]) << "." 
                  << static_cast<int>(ip.octets[1]) << "." 
                  << static_cast<int>(ip.octets[2]) << "." 
                  << static_cast<int>(ip.octets[3]) << std::endl;
      }
    } else {
      std::cout << "✗ Failed to parse multiple IPs" << std::endl;
    }
    
  } catch (const std::exception& e) {
    std::cout << "✗ Exception: " << e.what() << std::endl;
  }
  
  // Test 4: Reversible parsing
  std::cout << std::endl << "Test 4: Reversible parsing" << std::endl;
  try {
    cxxoptspp::Command cmd("test", "Reversible parsing test");
    cmd.add_option<bool>("--verbose", "Verbose output");
    cmd.add_option<int>("--count", "A count");
    cmd.add_option<std::string>("--name", "A name");
    
    cxxoptspp::CommandParser parser(cmd);
    
    int argc = 5;
    const char* argv[] = {"test", "--verbose", "--count=42", "--name=test_name"};
    
    auto result = parser.parse(argc, argv);
    
    // Get original inputs
    auto original = parser.original_inputs();
    std::cout << "✓ Original inputs:" << std::endl;
    for (const auto& [opt, val] : original) {
      std::cout << "  - " << opt << "=" << val << std::endl;
    }
    
  } catch (const std::exception& e) {
    std::cout << "✗ Exception: " << e.what() << std::endl;
  }
  
  std::cout << std::endl << "All tests completed" << std::endl;
  return 0;
}