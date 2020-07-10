#ifndef ACTIONTEST_H
#define ACTIONTEST_H

#include "behaviortree_cpp_v3/action_node.h"

namespace BT
{
class SyncActionTest : public SyncActionNode
{
  public:
    SyncActionTest(const std::string& name);

    BT::NodeStatus tick() override;

    void setExpectedResult(NodeStatus res);

    int tickCount() const
    {
        return tick_count_;
    }

    void resetTicks()
    {
        tick_count_ = 0;
    }

  private:
    NodeStatus expected_result_;
    int tick_count_;
};

class AsyncActionTest : public AsyncActionNode
{
  public:
    AsyncActionTest(const std::string& name, BT::Duration deadline_ms = std::chrono::milliseconds(100) );

    virtual ~AsyncActionTest()
    {
        halt();
    }

    // The method that is going to be executed by the thread
    BT::NodeStatus tick() override;

    void setTime(BT::Duration time);

    // The method used to interrupt the execution of the node
    virtual void halt() override;

    void setExpectedResult(NodeStatus res);

    int tickCount() const
    {
        return tick_count_;
    }

    void resetTicks()
    {
        tick_count_ = 0;
    }

  private:
    // using atomic because these variables might be accessed from different threads
    BT::Duration time_;
    std::atomic<NodeStatus> expected_result_;
    std::atomic<int> tick_count_;

};
}

#endif
