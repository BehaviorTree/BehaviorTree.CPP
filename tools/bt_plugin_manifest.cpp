#include <stdio.h>
#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include "behaviortree_cpp/bt_factory.h"

int main(int argc, char* argv[])
{
  if(argc != 2)
  {
    printf("Wrong number of command line arguments\nUsage: %s [filename]\n", argv[0]);
    return 1;
  }

  BT::BehaviorTreeFactory factory;

  std::unordered_set<std::string> default_nodes;
  for(auto& it : factory.manifests())
  {
    const auto& manifest = it.second;
    default_nodes.insert(manifest.registration_ID);
  }

  factory.registerFromPlugin(argv[1]);

  for(auto& it : factory.manifests())
  {
    const auto& manifest = it.second;
    if(default_nodes.count(manifest.registration_ID) > 0)
    {
      continue;
    }
    auto& params = manifest.ports;
    std::cout << "---------------\n"
              << manifest.registration_ID << " [" << manifest.type
              << "]\n  NodeConfig: " << params.size();

    if(params.size() > 0)
    {
      std::cout << ":";
    }

    std::cout << std::endl;

    for(auto& param : params)
    {
      std::cout << "    - [Key]: \"" << param.first << "\"" << std::endl;
    }
  }

  return 0;
}
