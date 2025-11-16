#include <iostream>
#include <cxxoptspp.hpp>

using namespace cxxopts;
using namespace cxxoptspp;

int main(int argc, char** argv) {
  try {
    // Create root command
    Command root_cmd("mytool", "A tool with enhanced features");
    
    // Add root options
    int verbose_level = 0;
    root_cmd.add_options()
      ("h,help", "Print help message")
      ("v,verbose", "Increase verbosity", value(verbose_level))
      ("d,debug", "Enable debug mode", value<bool>());

    // Add subcommands
    auto module_cmd = root_cmd.add_subcommand("module", "Module subcommand");
    auto subcmd = module_cmd->add_subcommand("subcmd", "Nested subcommand");

    // Add module options
    std::string module_name;
    module_cmd->add_options()
      ("n,name", "Module name", value(module_name))
      ("m,mode", "Operation mode", value<std::string>());

    // Add nested subcommand options with dependencies
    std::string output_file;
    bool overwrite = false;
    
    subcmd->add_options()
      ("o,output", "Output file", value(output_file))
      ("w,overwrite", "Overwrite existing file", value(overwrite))
      ("s,size", "File size", value<int>())
      ("t,type", "File type", value<std::string>());

    // Add dependency rules
    // --output is only valid when --mode=file (inherited from module)
    subcmd->add_dependency("(mode == file) ==> output");
    // --verbose needs to be used with --debug
    root_cmd.add_dependency("verbose ==> debug");
    // Either --size or --type must be provided
    subcmd->add_dependency("size || type");

    // Example with custom type (IPAddress)
    Command network_cmd = root_cmd.add_subcommand("network", "Network operations");
    IPAddress server_ip;
    std::vector<IPAddress> client_ips;
    
    network_cmd->add_options()
      ("s,server", "Server IP address", value<IPAddress>())
      ("c,clients", "Client IP addresses (comma-separated)", value<std::vector<IPAddress>>());

    // Parse command line
    CommandParser parser(root_cmd);
    ParseResult result = parser.parse(argc, argv);

    // Handle help
    if (result.count("help")) {
      std::cout << root_cmd.options().help() << std::endl;
      return 0;
    }

    // Display results
    std::cout << "Root options:" << std::endl;
    if (result.count("verbose")) {
      std::cout << "  Verbose level: " << result["verbose"].as<int>() << std::endl;
    }
    if (result.count("debug")) {
      std::cout << "  Debug mode: enabled" << std::endl;
    }

    const Command& current_cmd = parser.current_command();
    std::cout << "\nCurrent command path: " << current_cmd.name() << std::endl;
    
    // Show module options if applicable
    if (result.count("name")) {
      std::cout << "Module name: " << result["name"].as<std::string>() << std::endl;
    }
    if (result.count("mode")) {
      std::cout << "Mode: " << result["mode"].as<std::string>() << std::endl;
    }
    if (result.count("output")) {
      std::cout << "Output file: " << result["output"].as<std::string>() << std::endl;
    }
    if (result.count("overwrite")) {
      std::cout << "Overwrite: " << (result["overwrite"].as<bool>() ? "yes" : "no") << std::endl;
    }

    // Example with IP address custom type
    if (current_cmd.name() == "network") {
      if (result.count("server")) {
        IPAddress ip = result["server"].as<IPAddress>();
        std::cout << "Server IP: " << ip.to_string() << std::endl;
      }
      if (result.count("clients")) {
        auto ips = result["clients"].as<std::vector<IPAddress>>();
        std::cout << "Client IPs: ";
        for (size_t i = 0; i < ips.size(); ++i) {
          if (i > 0) std::cout << ", ";
          std::cout << ips[i].to_string();
        }
        std::cout << std::endl;
      }
    }

    // Example of reversible parsing
    std::cout << "\nReversible parsing example:" << std::endl;
    std::cout << "Original command: ";
    for (int i = 0; i < argc; ++i) {
      std::cout << argv[i] << " ";
    }
    std::cout << std::endl;

    // TODO: Demonstrate serialization/deserialization

    return 0;

  } catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }
}
