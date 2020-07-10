
#include "behaviortree_cpp_v3/loggers/bt_minitrace_logger.h"
#include "minitrace/minitrace.h"

namespace BT
{
std::atomic<bool> MinitraceLogger::ref_count(false);

MinitraceLogger::MinitraceLogger(const Tree &tree, const char* filename_json)
  : StatusChangeLogger( tree.rootNode() )
{
    bool expected = false;
    if (!ref_count.compare_exchange_strong(expected, true))
    {
        throw LogicError("Only one instance of StdCoutLogger shall be created");
    }

    minitrace::mtr_register_sigint_handler();
    minitrace::mtr_init(filename_json);
    this->enableTransitionToIdle(true);
}

MinitraceLogger::~MinitraceLogger()
{
    minitrace::mtr_flush();
    minitrace::mtr_shutdown();
    ref_count = false;
}

const char* toConstStr(NodeType type)
{
  switch (type)
  {
    case NodeType::ACTION:
      return "Action";
    case NodeType::CONDITION:
      return "Condition";
    case NodeType::DECORATOR:
      return "Decorator";
    case NodeType::CONTROL:
      return "Control";
    case NodeType::SUBTREE:
      return "SubTree";
    default:
      return "Undefined";
  }
}

void MinitraceLogger::callback(Duration /*timestamp*/,
                               const TreeNode& node, NodeStatus prev_status,
                               NodeStatus status)
{
    using namespace minitrace;

    const bool statusCompleted = (status == NodeStatus::SUCCESS || status == NodeStatus::FAILURE);

    const char* category = toConstStr(node.type());
    const char* name = node.name().c_str();

    if (prev_status == NodeStatus::IDLE && statusCompleted)
    {
        MTR_INSTANT(category, name);
    }
    else if (status == NodeStatus::RUNNING)
    {
        MTR_BEGIN(category, name);
    }
    else if (prev_status == NodeStatus::RUNNING && statusCompleted)
    {
        MTR_END(category, name);
    }
}

void MinitraceLogger::flush()
{
    minitrace::mtr_flush();
}
}   // end namespace
