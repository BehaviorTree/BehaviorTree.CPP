#ifndef ACTIONTEST_H
#define ACTIONTEST_H

#include "behavior_tree_core/action_node.h"

namespace BT
{
class ActionTestNode : public ActionNode
{
  public:
    // Constructor
    ActionTestNode(std::string name);

    ~ActionTestNode();

    // The method that is going to be executed by the thread
    BT::NodeStatus tick() override;

    void set_time(int time);

    // The method used to interrupt the execution of the node
    virtual void halt() override;

    void set_boolean_value(bool boolean_value);

  private:
    int time_;
    bool boolean_value_;
    std::atomic_bool stop_loop_;
};
}

#endif
