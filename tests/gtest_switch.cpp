#include <gtest/gtest.h>
#include "action_test_node.h"
#include "condition_test_node.h"
#include "behaviortree_cpp_v3/behavior_tree.h"
#include "behaviortree_cpp_v3/tree_node.h"

using BT::NodeStatus;
using std::chrono::milliseconds;

struct SwitchTest : testing::Test
{
    
    std::unique_ptr<BT::SwitchNode<2>> root;
    BT::AsyncActionTest action_1;
    BT::AsyncActionTest action_2;
    BT::AsyncActionTest action_3;
    BT::Blackboard::Ptr bb = BT::Blackboard::create();
    BT::NodeConfiguration simple_switch_config_;

    SwitchTest() :
      action_1("action_1", milliseconds(100)),
      action_2("action_2", milliseconds(100)),
      action_3("action_3", milliseconds(100))
    {
        BT::PortsRemapping input;
        input.insert(std::make_pair("case_1", "1"));
        input.insert(std::make_pair("case_2", "2"));

        BT::NodeConfiguration simple_switch_config_;
        simple_switch_config_.blackboard = bb;
        simple_switch_config_.input_ports = input;

        root = std::make_unique<BT::SwitchNode<2>>
            ("simple_switch", simple_switch_config_);
        root->addChild(&action_1);
        root->addChild(&action_2);
        root->addChild(&action_3);
    }
    ~SwitchTest()
    {
        root->halt();
    }
};

TEST_F(SwitchTest, DefaultCase)
{
    BT::NodeStatus state = root->executeTick();
    
    ASSERT_EQ(NodeStatus::IDLE, action_1.status());
    ASSERT_EQ(NodeStatus::IDLE, action_2.status());
    ASSERT_EQ(NodeStatus::RUNNING, action_3.status());
    ASSERT_EQ(NodeStatus::RUNNING, state);

    std::this_thread::sleep_for(milliseconds(110));
    state = root->executeTick();

    ASSERT_EQ(NodeStatus::IDLE, action_1.status());
    ASSERT_EQ(NodeStatus::IDLE, action_2.status());
    ASSERT_EQ(NodeStatus::IDLE, action_3.status());
    ASSERT_EQ(NodeStatus::SUCCESS, state);
}

TEST_F(SwitchTest, Case1)
{
    bb->set("variable", "1");
    BT::NodeStatus state = root->executeTick();
    
    ASSERT_EQ(NodeStatus::RUNNING, action_1.status());
    ASSERT_EQ(NodeStatus::IDLE, action_2.status());
    ASSERT_EQ(NodeStatus::IDLE, action_3.status());
    ASSERT_EQ(NodeStatus::RUNNING, state);

    std::this_thread::sleep_for(milliseconds(110));
    state = root->executeTick();

    ASSERT_EQ(NodeStatus::IDLE, action_1.status());
    ASSERT_EQ(NodeStatus::IDLE, action_2.status());
    ASSERT_EQ(NodeStatus::IDLE, action_3.status());
    ASSERT_EQ(NodeStatus::SUCCESS, state);
}

TEST_F(SwitchTest, Case2)
{
    bb->set("variable", "2");
    BT::NodeStatus state = root->executeTick();
    
    ASSERT_EQ(NodeStatus::IDLE, action_1.status());
    ASSERT_EQ(NodeStatus::RUNNING, action_2.status());
    ASSERT_EQ(NodeStatus::IDLE, action_3.status());
    ASSERT_EQ(NodeStatus::RUNNING, state);

    std::this_thread::sleep_for(milliseconds(110));
    state = root->executeTick();

    ASSERT_EQ(NodeStatus::IDLE, action_1.status());
    ASSERT_EQ(NodeStatus::IDLE, action_2.status());
    ASSERT_EQ(NodeStatus::IDLE, action_3.status());
    ASSERT_EQ(NodeStatus::SUCCESS, state);
}

TEST_F(SwitchTest, CaseNone)
{
    bb->set("variable", "none");
    BT::NodeStatus state = root->executeTick();
    
    ASSERT_EQ(NodeStatus::IDLE, action_1.status());
    ASSERT_EQ(NodeStatus::IDLE, action_2.status());
    ASSERT_EQ(NodeStatus::RUNNING, action_3.status());
    ASSERT_EQ(NodeStatus::RUNNING, state);

    std::this_thread::sleep_for(milliseconds(110));
    state = root->executeTick();

    ASSERT_EQ(NodeStatus::IDLE, action_1.status());
    ASSERT_EQ(NodeStatus::IDLE, action_2.status());
    ASSERT_EQ(NodeStatus::IDLE, action_3.status());
    ASSERT_EQ(NodeStatus::SUCCESS, state);
}

TEST_F(SwitchTest, CaseSwitchToDefault)
{
    bb->set("variable", "1");
    BT::NodeStatus state = root->executeTick();
    
    ASSERT_EQ(NodeStatus::RUNNING, action_1.status());
    ASSERT_EQ(NodeStatus::IDLE, action_2.status());
    ASSERT_EQ(NodeStatus::IDLE, action_3.status());
    ASSERT_EQ(NodeStatus::RUNNING, state);

    std::this_thread::sleep_for(milliseconds(10));
    state = root->executeTick();
    ASSERT_EQ(NodeStatus::RUNNING, action_1.status());
    ASSERT_EQ(NodeStatus::IDLE, action_2.status());
    ASSERT_EQ(NodeStatus::IDLE, action_3.status());
    ASSERT_EQ(NodeStatus::RUNNING, state);

    // Switch Node does not feels changes. Only when tick. 
    // (not reactive)
    std::this_thread::sleep_for(milliseconds(10));
    bb->set("variable", "");
    std::this_thread::sleep_for(milliseconds(10));
    ASSERT_EQ(NodeStatus::RUNNING, action_1.status());
    ASSERT_EQ(NodeStatus::IDLE, action_2.status());
    ASSERT_EQ(NodeStatus::IDLE, action_3.status());
    ASSERT_EQ(NodeStatus::RUNNING, root->status());

    std::this_thread::sleep_for(milliseconds(10));
    state = root->executeTick();
    ASSERT_EQ(NodeStatus::IDLE, action_1.status());
    ASSERT_EQ(NodeStatus::IDLE, action_2.status());
    ASSERT_EQ(NodeStatus::RUNNING, action_3.status());
    ASSERT_EQ(NodeStatus::RUNNING, state);

    std::this_thread::sleep_for(milliseconds(110));
    state = root->executeTick();

    ASSERT_EQ(NodeStatus::IDLE, action_1.status());
    ASSERT_EQ(NodeStatus::IDLE, action_2.status());
    ASSERT_EQ(NodeStatus::IDLE, action_3.status());
    ASSERT_EQ(NodeStatus::SUCCESS, root->status());
}

TEST_F(SwitchTest, CaseSwitchToAction2)
{
    bb->set("variable", "1");
    BT::NodeStatus state = root->executeTick();
    
    ASSERT_EQ(NodeStatus::RUNNING, action_1.status());
    ASSERT_EQ(NodeStatus::IDLE, action_2.status());
    ASSERT_EQ(NodeStatus::IDLE, action_3.status());
    ASSERT_EQ(NodeStatus::RUNNING, state);

    bb->set("variable", "2");
    std::this_thread::sleep_for(milliseconds(10));
    state = root->executeTick();
    ASSERT_EQ(NodeStatus::IDLE, action_1.status());
    ASSERT_EQ(NodeStatus::RUNNING, action_2.status());
    ASSERT_EQ(NodeStatus::IDLE, action_3.status());
    ASSERT_EQ(NodeStatus::RUNNING, state);

    std::this_thread::sleep_for(milliseconds(110));
    state = root->executeTick();

    ASSERT_EQ(NodeStatus::IDLE, action_1.status());
    ASSERT_EQ(NodeStatus::IDLE, action_2.status());
    ASSERT_EQ(NodeStatus::IDLE, action_3.status());
    ASSERT_EQ(NodeStatus::SUCCESS, root->status());
}

TEST_F(SwitchTest, ActionFailure)
{
    bb->set("variable", "1");
    BT::NodeStatus state = root->executeTick();
    
    ASSERT_EQ(NodeStatus::RUNNING, action_1.status());
    ASSERT_EQ(NodeStatus::IDLE, action_2.status());
    ASSERT_EQ(NodeStatus::IDLE, action_3.status());
    ASSERT_EQ(NodeStatus::RUNNING, state);

    std::this_thread::sleep_for(milliseconds(10));
    action_1.setStatus(NodeStatus::FAILURE);
    state = root->executeTick();
    ASSERT_EQ(NodeStatus::FAILURE, action_1.status());
    ASSERT_EQ(NodeStatus::IDLE, action_2.status());
    ASSERT_EQ(NodeStatus::IDLE, action_3.status());
    ASSERT_EQ(NodeStatus::IDLE, state);

    state = root->executeTick();
    state = root->executeTick();
    ASSERT_EQ(NodeStatus::FAILURE, action_1.status());
    ASSERT_EQ(NodeStatus::IDLE, action_2.status());
    ASSERT_EQ(NodeStatus::IDLE, action_3.status());
    ASSERT_EQ(NodeStatus::IDLE, state);
}