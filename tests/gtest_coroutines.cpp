#include "behaviortree_cpp_v3/decorators/timeout_node.h"
#include "behaviortree_cpp_v3/behavior_tree.h"
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
        start_time_ = Timepoint::min();
        halted_ = true;
        BT::CoroActionNode::halt();
    }

    bool wasHalted()
    {
        return halted_;
    }

    void setRequiredTime(Millisecond ms)
    {
        timeout_ = ms;
    }

  protected:
    virtual BT::NodeStatus tick() override
    {
        std::cout << "Starting action " << std::endl;
        halted_ = false;

        if (start_time_ == Timepoint::min())
        {
            start_time_ = std::chrono::steady_clock::now();
        }

        while (std::chrono::steady_clock::now() < (start_time_ + timeout_))
        {
            setStatusRunningAndYield();
        }

        halted_ = false;

        std::cout << "Done" << std::endl;
        start_time_ = Timepoint::min();
        return (will_fail_ ? BT::NodeStatus::FAILURE : BT::NodeStatus::SUCCESS);
    }

  public:
    bool will_fail_;

  private:
    std::chrono::milliseconds timeout_;
    Timepoint start_time_;
    bool halted_;
};

BT::NodeStatus executeWhileRunning(BT::TreeNode &node)
{
    auto status = node.executeTick();
    while (status == BT::NodeStatus::RUNNING)
    {
        status = node.executeTick();
        std::this_thread::sleep_for(Millisecond(1));
    }
    return status;
}


TEST(CoroTest, do_action)
{
    BT::NodeConfiguration node_config_;
    node_config_.blackboard = BT::Blackboard::create();
    BT::assignDefaultRemapping<SimpleCoroAction>(node_config_);
    SimpleCoroAction node( milliseconds(200), false, "Action", node_config_);

    EXPECT_EQ(BT::NodeStatus::SUCCESS, executeWhileRunning(node));
    EXPECT_FALSE( node.wasHalted() );

    EXPECT_EQ(BT::NodeStatus::SUCCESS, executeWhileRunning(node)) << "Second call to coro action";
    EXPECT_FALSE( node.wasHalted() );

    node.will_fail_ = true;
    EXPECT_EQ(BT::NodeStatus::FAILURE, executeWhileRunning(node))
        << "Should execute again and retun failure";
    EXPECT_FALSE( node.wasHalted() );


    EXPECT_EQ(BT::NodeStatus::FAILURE, executeWhileRunning(node))
        << "Shoudln't fail because we set status to idle";
    EXPECT_FALSE( node.wasHalted() );
}


TEST(CoroTest, do_action_timeout)
{
    BT::NodeConfiguration node_config_;
    node_config_.blackboard = BT::Blackboard::create();
    BT::assignDefaultRemapping<SimpleCoroAction>(node_config_);

    SimpleCoroAction node( milliseconds(300), false, "Action", node_config_);
    BT::TimeoutNode timeout("TimeoutAction", 200);

    timeout.setChild(&node);

    EXPECT_EQ(BT::NodeStatus::FAILURE, executeWhileRunning(timeout) ) << "should timeout";
    EXPECT_TRUE( node.wasHalted() );

    node.setRequiredTime( Millisecond(100) );

    EXPECT_EQ(BT::NodeStatus::SUCCESS, executeWhileRunning(timeout) );
    EXPECT_FALSE( node.wasHalted() );
}

TEST(CoroTest, sequence_child)
{
    BT::NodeConfiguration node_config_;
    node_config_.blackboard = BT::Blackboard::create();
    BT::assignDefaultRemapping<SimpleCoroAction>(node_config_);

    SimpleCoroAction actionA( milliseconds(200), false, "action_A", node_config_);
    SimpleCoroAction actionB( milliseconds(200), false, "action_B", node_config_);
    BT::TimeoutNode timeout("timeout", 300);
    BT::SequenceNode sequence("sequence");

    timeout.setChild(&sequence);
    sequence.addChild(&actionA);
    sequence.addChild(&actionB);

    EXPECT_EQ(BT::NodeStatus::FAILURE, executeWhileRunning(timeout) ) << "should timeout";
    EXPECT_FALSE( actionA.wasHalted() );
    EXPECT_TRUE( actionB.wasHalted() );

}

