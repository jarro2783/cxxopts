#include "cxxoptspp.hpp"
#include <iostream>

using namespace std;

int main(int argc, const char* argv[]) {
  // Create root command
  Command root("app", "Multilevel subcommands example");
  root.add_option("--verbose", "Verbose mode", false, false);
  root.add_option("--config", "Configuration file", string("config.json"), false);

  // Create level 1 subcommand
  auto sub1 = root.add_subcommand("sub1", "Level 1 subcommand");
  sub1->add_option("--config", "Configuration file", string("sub1_config.json"), false);
  sub1->add_option("--sub1-option", "Sub1 specific option", 123, false);

  // Create level 2 subcommand
  auto sub1_sub1 = sub1->add_subcommand("sub1", "Level 2 subcommand");
  sub1_sub1->add_option("--verbose", "Verbose mode", true, false);
  sub1_sub1->add_option("--sub1-sub1-option", "Sub1-sub1 specific option", string("test"), false);

  // Create another level 1 subcommand
  auto sub2 = root.add_subcommand("sub2", "Another level 1 subcommand");
  sub2->add_option("--config", "Configuration file", string("sub2_config.json"), false);

  // Parse arguments
  CommandParser parser(root);
  auto result = parser.parse(argc, argv);

  // Print results
  cout << "Current command: " << parser.current_command().name() << endl;
  cout << "Verbose: " << result["verbose"].as<bool>() << endl;
  cout << "Config: " << result["config"].as<string>() << endl;
  
  if (result.count("sub1-option")) {
    cout << "Sub1 option: " << result["sub1-option"].as<int>() << endl;
  }
  
  if (result.count("sub1-sub1-option")) {
    cout << "Sub1-sub1 option: " << result["sub1-sub1-option"].as<string>() << endl;
  }

  return 0;
}