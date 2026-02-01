#ifndef CONDITIONTEST_H
#define CONDITIONTEST_H

#include "behaviortree_cpp/condition_node.h"

namespace BT
{
class ConditionTestNode : public ConditionNode
{
public:
  ConditionTestNode(const std::string& name);

  void setExpectedResult(NodeStatus res);

  // The method that is going to be executed by the thread
  virtual BT::NodeStatus tick() override;

  int tickCount() const
  {
    return tick_count_;
  }

private:
  NodeStatus expected_result_;
  int tick_count_;
};
}  // namespace BT

#endif
