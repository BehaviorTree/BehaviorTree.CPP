#ifndef ACTIONTEST_H
#define ACTIONTEST_H

#include "behavior_tree_core/action_node.h"

namespace BT
{
class ActionTestNode : public AsyncActionNode
{
  public:
    // Constructor
    ActionTestNode(std::string name);

    ~ActionTestNode();

    // The method that is going to be executed by the thread
    virtual BT::NodeStatus asyncTick() override;

    void set_time(int time);

    // The method used to interrupt the execution of the node
    virtual void halt() override;

    void set_boolean_value(bool boolean_value);

  private:
    int time_;
    bool boolean_value_;

    ///ReturnStatus status_;
};
}

#endif
