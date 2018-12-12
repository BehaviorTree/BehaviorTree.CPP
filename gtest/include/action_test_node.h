#ifndef ACTIONTEST_H
#define ACTIONTEST_H

#include "behaviortree_cpp/action_node.h"

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
    AsyncActionTest(const std::string& name);

    ~AsyncActionTest();

    // The method that is going to be executed by the thread
    BT::NodeStatus tick() override;

    void setTime(int time);

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
    int time_;
    bool boolean_value_;
    int tick_count_;
    std::atomic_bool stop_loop_;
};
}

#endif
