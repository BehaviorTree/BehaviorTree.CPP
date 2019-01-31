#include "behaviortree_cpp/decorators/timeout_node.h"
#include "behaviortree_cpp/action_node.h"
#include <chrono>
#include <gtest/gtest.h>

using namespace std::chrono;

using Millisecond = std::chrono::milliseconds;
using Timepoint = std::chrono::time_point<std::chrono::steady_clock>;

class SimpleCoroAction : public BT::CoroActionNode
{
  public:
    SimpleCoroAction(milliseconds timeout, bool will_fail,
                     const std::string &node_name,
                     const BT::NodeConfiguration &config)
      : BT::CoroActionNode(node_name, config)
      , will_fail_(will_fail)
      , timeout_(timeout)
      , start_time_(Timepoint::min())
    {
    }

    virtual void halt() override
    {
        std::cout << "Action was halted. doing cleanup here" << std::endl;
     //   start_time_ = Timepoint::min();
     //  setStatus(BT::NodeStatus::FAILURE);
     //   BT::CoroActionNode::halt();
    }

    void setRequiredTime(Millisecond ms)
    {
        timeout_ = ms;
    }

  protected:
    virtual BT::NodeStatus tick() override
    {
        std::cout << "Starting action " << std::endl;

        if (start_time_ == Timepoint::min())
        {
            start_time_ = std::chrono::steady_clock::now();
        }

        while (std::chrono::steady_clock::now() < (start_time_ + timeout_))
        {
            setStatusRunningAndYield();
        }

        std::cout << "Done" << std::endl;
        start_time_ = Timepoint::min();
        return (will_fail_ ? BT::NodeStatus::FAILURE : BT::NodeStatus::SUCCESS);
    }

  public:
    bool will_fail_;

  private:
    std::chrono::milliseconds timeout_;
    Timepoint start_time_;
};

BT::NodeStatus executeWhileRunning(BT::TreeNode &node)
{
    auto status = node.executeTick();
    while (status == BT::NodeStatus::RUNNING)
    {
        status = node.executeTick();
    }
    return status;
}


TEST(SimpleCoroTest, do_action)
{
    BT::NodeConfiguration node_config_;
    node_config_.blackboard = BT::Blackboard::create();
    BT::assignDefaultRemapping<SimpleCoroAction>(node_config_);
    SimpleCoroAction node( milliseconds(1000), false, "Action", node_config_);

    EXPECT_EQ(BT::NodeStatus::SUCCESS, executeWhileRunning(node));
    EXPECT_EQ(BT::NodeStatus::SUCCESS, executeWhileRunning(node)) << "Second call to coro action";
    node.will_fail_ = true;
    EXPECT_EQ(BT::NodeStatus::FAILURE, executeWhileRunning(node))
        << "Should execute again and retun failure";

    node.setStatus(BT::NodeStatus::IDLE);  // We are forced to set this to ensure the action
                                            // is run again
    EXPECT_EQ(BT::NodeStatus::FAILURE, executeWhileRunning(node))
        << "Shoudln't fail because we set status to idle";
}


TEST(SimpleCoroTest, do_action_timeout)
{
    BT::NodeConfiguration node_config_;
    node_config_.blackboard = BT::Blackboard::create();
    BT::assignDefaultRemapping<SimpleCoroAction>(node_config_);

    SimpleCoroAction node( milliseconds(1000), false, "Action", node_config_);
    BT::TimeoutNode timeout("TimeoutAction", 500);

    timeout.setChild(&node);

    EXPECT_EQ(BT::NodeStatus::FAILURE, executeWhileRunning(timeout) ) << "should timeout";

    node.setRequiredTime( Millisecond(300) );

    timeout.setStatus(BT::NodeStatus::IDLE);
    EXPECT_EQ(BT::NodeStatus::SUCCESS, executeWhileRunning(timeout) );
}

