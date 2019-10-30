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

    void setBoolean(bool boolean_value);

    int tickCount() const
    {
        return tick_count_;
    }

    void resetTicks()
    {
        tick_count_ = 0;
    }

  private:
    bool boolean_value_;
    int tick_count_;
};

class AsyncActionTest : public AsyncActionNode
{
  public:
    AsyncActionTest(const std::string& name, BT::Duration deadline_ms);

    ~AsyncActionTest();

    // The method that is going to be executed by the thread
    BT::NodeStatus tick() override;

    void setTime(BT::Duration time);

    // The method used to interrupt the execution of the node
    virtual void halt() override;

    void setBoolean(bool boolean_value);

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
    std::atomic_bool boolean_value_;
    std::atomic<int> tick_count_;
    std::atomic_bool stop_loop_;
};
}

#endif
