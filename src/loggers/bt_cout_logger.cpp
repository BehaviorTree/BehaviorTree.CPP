#include "behaviortree_cpp/loggers/bt_cout_logger.h"

namespace BT
{

StdCoutLogger::StdCoutLogger(const BT::Tree& tree) : StatusChangeLogger(tree.rootNode())
{}
StdCoutLogger::~StdCoutLogger()
{}

void StdCoutLogger::callback(Duration timestamp, const TreeNode& node,
                             NodeStatus prev_status, NodeStatus status)
{
  using namespace std::chrono;

  constexpr const char* whitespaces = "                         ";
  constexpr const size_t ws_count = 25;

  double since_epoch = duration<double>(timestamp).count();
  printf("[%.3f]: %s%s %s -> %s", since_epoch, node.name().c_str(),
         &whitespaces[std::min(ws_count, node.name().size())],
         toStr(prev_status, true).c_str(), toStr(status, true).c_str());
  std::cout << std::endl;
}

void StdCoutLogger::flush()
{
  std::cout << std::flush;
}

}  // namespace BT
