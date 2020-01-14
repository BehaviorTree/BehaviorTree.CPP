#ifndef CONDITIONTEST_H
#define CONDITIONTEST_H

#include "behaviortree_cpp_v3/condition_node.h"

namespace BT
{
class ConditionTestNode : public ConditionNode
{
  public:
    
    ConditionTestNode(const std::string& name);

    void setBoolean(bool boolean_value);

    // The method that is going to be executed by the thread
    virtual BT::NodeStatus tick() override;

    int tickCount() const
    {
        return tick_count_;
    }

  private:
    bool boolean_value_;
    int tick_count_;
};
}

#endif
