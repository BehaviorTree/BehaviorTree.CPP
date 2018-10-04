#ifndef CONDITIONTEST_H
#define CONDITIONTEST_H

#include "behavior_tree_core/condition_node.h"

namespace BT
{
class ConditionTestNode : public ConditionNode
{
  public:
    // Constructor
    ConditionTestNode(const std::string& name);
    ~ConditionTestNode();
    void set_boolean_value(bool boolean_value);

    // The method that is going to be executed by the thread
    virtual BT::NodeStatus tick() override;

  private:
    bool boolean_value_;
};
}

#endif
