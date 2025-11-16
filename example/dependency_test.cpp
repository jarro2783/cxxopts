#include <cxxoptspp.hpp>
#include <iostream>

int main(int argc, char* argv[]) {
  try {
    // Create root command
    cxxoptspp::Command root_cmd("app", "Test application for dependency rules");
    root_cmd.add_option("mode", "Operating mode", "default");
    root_cmd.add_option("output", "Output file path", "out.txt");
    root_cmd.add_option("verbose", "Enable verbose mode", false);
    
    // Add a dependency rule: mode == file
    root_cmd.add_dependency("--mode == file");
    
    // Add another rule: if verbose is enabled, output must end with .log
    root_cmd.add_dependency("--verbose && --output == *.log");
    
    // Create parser and parse
    cxxoptspp::CommandParser parser(root_cmd);
    auto result = parser.parse(argc, argv);
    
    // Print results
    std::cout << "Command executed successfully!" << std::endl;
    std::cout << "Mode: " << result["mode"].as<std::string>() << std::endl;
    std::cout << "Output: " << result["output"].as<std::string>() << std::endl;
    std::cout << "Verbose: " << result["verbose"].as<bool>() << std::endl;
    
  } catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }
  
  return 0;
}