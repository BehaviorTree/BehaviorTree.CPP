/**
 * @brief Command line tool to generate TreeNodesModel XML.
 *
 * This tool outputs the TreeNodesModel XML for nodes registered in the factory,
 * optionally including builtin nodes and loading additional plugins.
 *
 * Usage:
 *   bt_nodes_model [--include-builtin] [--plugin <path>]...
 *
 * Options:
 *   --include-builtin   Include builtin nodes in the output
 *   --plugin <path>     Load a plugin from the specified path (can be repeated)
 *   -h, --help          Show this help message
 */

#include "behaviortree_cpp/bt_factory.h"
#include "behaviortree_cpp/xml_parsing.h"

#include <cstdio>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

void printUsage(const char* program_name)
{
  std::printf("Usage: %s [OPTIONS]\n\n", program_name);
  std::printf("Generate TreeNodesModel XML for BehaviorTree.CPP nodes.\n\n");
  std::printf("Options:\n");
  std::printf("  --include-builtin   Include builtin nodes in the output\n");
  std::printf("  --plugin <path>     Load a plugin from the specified path\n");
  std::printf("                      (can be specified multiple times)\n");
  std::printf("  -h, --help          Show this help message\n\n");
  std::printf("Examples:\n");
  std::printf("  %s --include-builtin\n", program_name);
  std::printf("  %s --plugin ./libmy_nodes.so\n", program_name);
  std::printf("  %s --include-builtin --plugin ./libplugin1.so --plugin "
              "./libplugin2.so\n",
              program_name);
}

int main(int argc, char* argv[])
{
  bool include_builtin = false;
  std::vector<std::string> plugins;

  // Parse command line arguments
  for(int i = 1; i < argc; ++i)
  {
    if(std::strcmp(argv[i], "--include-builtin") == 0)
    {
      include_builtin = true;
    }
    else if(std::strcmp(argv[i], "--plugin") == 0)
    {
      if(i + 1 >= argc)
      {
        std::fprintf(stderr, "Error: --plugin requires a path argument\n");
        return 1;
      }
      plugins.push_back(argv[++i]);
    }
    else if(std::strcmp(argv[i], "-h") == 0 || std::strcmp(argv[i], "--help") == 0)
    {
      printUsage(argv[0]);
      return 0;
    }
    else
    {
      std::fprintf(stderr, "Error: Unknown option '%s'\n", argv[i]);
      printUsage(argv[0]);
      return 1;
    }
  }

  BT::BehaviorTreeFactory factory;

  // Load plugins
  for(const auto& plugin_path : plugins)
  {
    try
    {
      factory.registerFromPlugin(plugin_path);
    }
    catch(const std::exception& e)
    {
      std::fprintf(stderr, "Error loading plugin '%s': %s\n", plugin_path.c_str(),
                   e.what());
      return 1;
    }
  }

  // Generate and print the TreeNodesModel XML
  std::string xml = BT::writeTreeNodesModelXML(factory, include_builtin);
  std::cout << xml << std::endl;

  return 0;
}
