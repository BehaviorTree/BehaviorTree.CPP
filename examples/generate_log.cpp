#include "behaviortree_cpp/bt_factory.h"
#include "behaviortree_cpp/loggers/groot2_publisher.h"
#include "behaviortree_cpp/loggers/bt_file_logger_v2.h"
#include "behaviortree_cpp/loggers/bt_sqlite_logger.h"
#include "behaviortree_cpp/loggers/bt_cout_logger.h"

// clang-format on

int main(int argc, char** argv)
{
  if(argc < 2 || argc > 3) {
    std::cout << "Provide a XML file as first argument. "
                 "Second argument might be the name of the tree to instantiate." << std::endl;
    return 1;
  }
  const std::string file = argv[1];

  BT::BehaviorTreeFactory factory;

  BT::Tree tree;

  if(argc == 3) {
    factory.registerBehaviorTreeFromFile(file);
    tree = factory.createTree(argv[2]);
  }
  else {
    tree = factory.createTreeFromFile(file);
  }

  BT::StdCoutLogger cout_logger(tree);
  BT::Groot2Publisher publisher(tree);
  BT::FileLogger2 file_logger(tree, "./generated_log.btlog");
  BT::SqliteLogger sqlite_logger(tree, "./generated_log.db3");

  // helper function to print the tree
  BT::printTreeRecursively(tree.rootNode());

  std::cout << "\nTree will run indefinitively. Press CTRL-C to stop\n";

  while(true) {
    tree.tickWhileRunning();
  }
}
