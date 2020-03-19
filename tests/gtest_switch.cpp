#include <gtest/gtest.h>
#include "action_test_node.h"
#include "behaviortree_cpp_v3/behavior_tree.h"
#include "behaviortree_cpp_v3/tree_node.h"
#include "behaviortree_cpp_v3/bt_factory.h"

using BT::NodeStatus;
using std::chrono::milliseconds;

static const char* xml_text = R"(

<root main_tree_to_execute = "MainTree" >
    <BehaviorTree ID="MainTree">
        <Switch3 name="simple_switch" variable="{my_var}"  case_1="1" case_2="42" case_3="666" >
            <AsyncActionTest name="action_1"/>
            <AsyncActionTest name="action_42"/>
            <AsyncActionTest name="action_666"/>
            <AsyncActionTest name="action_default"/>
        </Switch3>
    </BehaviorTree>
</root>
        )";

struct SwitchTestFactory : testing::Test
{
    using Switch3 = BT::SwitchNode<3>;
    using AsyncActionTest = BT::AsyncActionTest;

    BT::BehaviorTreeFactory factory;
    std::unique_ptr<BT::Tree> tree;

    SwitchTestFactory() 
    {
        factory.registerNodeType<AsyncActionTest>("AsyncActionTest");
        tree = std::make_unique<BT::Tree>(factory.createTreeFromText(xml_text));
    }
};

struct SwitchTest : testing::Test
{
    using Switch2 = BT::SwitchNode<2>;
    std::unique_ptr<Switch2> root;
    BT::AsyncActionTest action_1;
    BT::AsyncActionTest action_42;
    BT::AsyncActionTest action_def;
    BT::Blackboard::Ptr bb = BT::Blackboard::create();
    BT::NodeConfiguration simple_switch_config_;

    SwitchTest() :
      action_1("action_1", milliseconds(100)),
      action_42("action_42", milliseconds(100)),
      action_def("action_default", milliseconds(100))
    {
        BT::PortsRemapping input;
        input.insert(std::make_pair("variable", "{my_var}"));
        input.insert(std::make_pair("case_1", "1"));
        input.insert(std::make_pair("case_2", "42"));

        BT::NodeConfiguration simple_switch_config_;
        simple_switch_config_.blackboard = bb;
        simple_switch_config_.input_ports = input;

        root = std::make_unique<Switch2>("simple_switch", simple_switch_config_);

        root->addChild(&action_1);
        root->addChild(&action_42);
        root->addChild(&action_def);
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
    ASSERT_EQ(NodeStatus::IDLE, action_42.status());
    ASSERT_EQ(NodeStatus::RUNNING, action_def.status());
    ASSERT_EQ(NodeStatus::RUNNING, state);

    std::this_thread::sleep_for(milliseconds(110));
    state = root->executeTick();

    ASSERT_EQ(NodeStatus::IDLE, action_1.status());
    ASSERT_EQ(NodeStatus::IDLE, action_42.status());
    ASSERT_EQ(NodeStatus::IDLE, action_def.status());
    ASSERT_EQ(NodeStatus::SUCCESS, state);
}

TEST_F(SwitchTest, Case1)
{
    bb->set("my_var", "1");
    BT::NodeStatus state = root->executeTick();
    
    ASSERT_EQ(NodeStatus::RUNNING, action_1.status());
    ASSERT_EQ(NodeStatus::IDLE, action_42.status());
    ASSERT_EQ(NodeStatus::IDLE, action_def.status());
    ASSERT_EQ(NodeStatus::RUNNING, state);

    std::this_thread::sleep_for(milliseconds(110));
    state = root->executeTick();

    ASSERT_EQ(NodeStatus::IDLE, action_1.status());
    ASSERT_EQ(NodeStatus::IDLE, action_42.status());
    ASSERT_EQ(NodeStatus::IDLE, action_def.status());
    ASSERT_EQ(NodeStatus::SUCCESS, state);
}

TEST_F(SwitchTest, Case2)
{
    bb->set("my_var", "42");
    BT::NodeStatus state = root->executeTick();
    
    ASSERT_EQ(NodeStatus::IDLE, action_1.status());
    ASSERT_EQ(NodeStatus::RUNNING, action_42.status());
    ASSERT_EQ(NodeStatus::IDLE, action_def.status());
    ASSERT_EQ(NodeStatus::RUNNING, state);

    std::this_thread::sleep_for(milliseconds(110));
    state = root->executeTick();

    ASSERT_EQ(NodeStatus::IDLE, action_1.status());
    ASSERT_EQ(NodeStatus::IDLE, action_42.status());
    ASSERT_EQ(NodeStatus::IDLE, action_def.status());
    ASSERT_EQ(NodeStatus::SUCCESS, state);
}

TEST_F(SwitchTest, CaseNone)
{
    bb->set("my_var", "none");
    BT::NodeStatus state = root->executeTick();
    
    ASSERT_EQ(NodeStatus::IDLE, action_1.status());
    ASSERT_EQ(NodeStatus::IDLE, action_42.status());
    ASSERT_EQ(NodeStatus::RUNNING, action_def.status());
    ASSERT_EQ(NodeStatus::RUNNING, state);

    std::this_thread::sleep_for(milliseconds(110));
    state = root->executeTick();

    ASSERT_EQ(NodeStatus::IDLE, action_1.status());
    ASSERT_EQ(NodeStatus::IDLE, action_42.status());
    ASSERT_EQ(NodeStatus::IDLE, action_def.status());
    ASSERT_EQ(NodeStatus::SUCCESS, state);
}

TEST_F(SwitchTest, CaseSwitchToDefault)
{
    bb->set("my_var", "1");
    BT::NodeStatus state = root->executeTick();
    
    ASSERT_EQ(NodeStatus::RUNNING, action_1.status());
    ASSERT_EQ(NodeStatus::IDLE, action_42.status());
    ASSERT_EQ(NodeStatus::IDLE, action_def.status());
    ASSERT_EQ(NodeStatus::RUNNING, state);

    std::this_thread::sleep_for(milliseconds(10));
    state = root->executeTick();
    ASSERT_EQ(NodeStatus::RUNNING, action_1.status());
    ASSERT_EQ(NodeStatus::IDLE, action_42.status());
    ASSERT_EQ(NodeStatus::IDLE, action_def.status());
    ASSERT_EQ(NodeStatus::RUNNING, state);

    // Switch Node does not feels changes. Only when tick. 
    // (not reactive)
    std::this_thread::sleep_for(milliseconds(10));
    bb->set("my_var", "");
    std::this_thread::sleep_for(milliseconds(10));
    ASSERT_EQ(NodeStatus::RUNNING, action_1.status());
    ASSERT_EQ(NodeStatus::IDLE, action_42.status());
    ASSERT_EQ(NodeStatus::IDLE, action_def.status());
    ASSERT_EQ(NodeStatus::RUNNING, root->status());

    std::this_thread::sleep_for(milliseconds(10));
    state = root->executeTick();
    ASSERT_EQ(NodeStatus::IDLE, action_1.status());
    ASSERT_EQ(NodeStatus::IDLE, action_42.status());
    ASSERT_EQ(NodeStatus::RUNNING, action_def.status());
    ASSERT_EQ(NodeStatus::RUNNING, state);

    std::this_thread::sleep_for(milliseconds(110));
    state = root->executeTick();

    ASSERT_EQ(NodeStatus::IDLE, action_1.status());
    ASSERT_EQ(NodeStatus::IDLE, action_42.status());
    ASSERT_EQ(NodeStatus::IDLE, action_def.status());
    ASSERT_EQ(NodeStatus::SUCCESS, root->status());
}

TEST_F(SwitchTest, CaseSwitchToAction2)
{
    bb->set("my_var", "1");
    BT::NodeStatus state = root->executeTick();
    
    ASSERT_EQ(NodeStatus::RUNNING, action_1.status());
    ASSERT_EQ(NodeStatus::IDLE, action_42.status());
    ASSERT_EQ(NodeStatus::IDLE, action_def.status());
    ASSERT_EQ(NodeStatus::RUNNING, state);

    bb->set("my_var", "42");
    std::this_thread::sleep_for(milliseconds(10));
    state = root->executeTick();
    ASSERT_EQ(NodeStatus::IDLE, action_1.status());
    ASSERT_EQ(NodeStatus::RUNNING, action_42.status());
    ASSERT_EQ(NodeStatus::IDLE, action_def.status());
    ASSERT_EQ(NodeStatus::RUNNING, state);

    std::this_thread::sleep_for(milliseconds(110));
    state = root->executeTick();

    ASSERT_EQ(NodeStatus::IDLE, action_1.status());
    ASSERT_EQ(NodeStatus::IDLE, action_42.status());
    ASSERT_EQ(NodeStatus::IDLE, action_def.status());
    ASSERT_EQ(NodeStatus::SUCCESS, root->status());
}

TEST_F(SwitchTest, ActionFailure)
{
    bb->set("my_var", "1");
    BT::NodeStatus state = root->executeTick();
    
    ASSERT_EQ(NodeStatus::RUNNING, action_1.status());
    ASSERT_EQ(NodeStatus::IDLE, action_42.status());
    ASSERT_EQ(NodeStatus::IDLE, action_def.status());
    ASSERT_EQ(NodeStatus::RUNNING, state);

    action_1.setExpectedResult(NodeStatus::FAILURE);
    std::this_thread::sleep_for(milliseconds(110));
    state = root->executeTick();

    ASSERT_EQ(NodeStatus::FAILURE, state);
    ASSERT_EQ(NodeStatus::IDLE, action_1.status());
    ASSERT_EQ(NodeStatus::IDLE, action_42.status());
    ASSERT_EQ(NodeStatus::IDLE, action_def.status());
}

TEST_F(SwitchTest, SwitchAfterActionSucceed)
{
    bb->set("my_var", "1");
    BT::NodeStatus state = root->executeTick();
    
    ASSERT_EQ(NodeStatus::RUNNING, action_1.status());
    ASSERT_EQ(NodeStatus::IDLE, action_42.status());
    ASSERT_EQ(NodeStatus::IDLE, action_def.status());
    ASSERT_EQ(NodeStatus::RUNNING, state);

    std::this_thread::sleep_for(milliseconds(110));
    bb->set("my_var", "42");
    state = root->executeTick();

    ASSERT_EQ(NodeStatus::SUCCESS, state);
    ASSERT_EQ(NodeStatus::IDLE, action_1.status());
    ASSERT_EQ(NodeStatus::IDLE, action_42.status());
    ASSERT_EQ(NodeStatus::IDLE, action_def.status());
}

TEST_F(SwitchTestFactory, SwitchTreeTest)
{
    // Ticking Default Action
    tree->root_node->executeTick();
    for (auto& node : tree->nodes)
    {
        if(node->name() == "simple_switch" || node->name() == "action_default")
            ASSERT_EQ(node->status(), NodeStatus::RUNNING);
        else        
            ASSERT_EQ(node->status(), NodeStatus::IDLE);
    }
    
    // Switching to action_1
    std::this_thread::sleep_for(milliseconds(10));
    tree->blackboard_stack[0]->set("my_var", "1");
    tree->root_node->executeTick();
    for (auto& node : tree->nodes)
    {
        if(node->name() == "simple_switch" || node->name() == "action_1")
            ASSERT_EQ(node->status(), NodeStatus::RUNNING);
        else        
            ASSERT_EQ(node->status(), NodeStatus::IDLE);
    }

    // Switching to action_666
    std::this_thread::sleep_for(milliseconds(10));
    tree->blackboard_stack[0]->set("my_var", "666");
    tree->root_node->executeTick();
    for (auto& node : tree->nodes)
    {
        if(node->name() == "simple_switch" || node->name() == "action_666")
            ASSERT_EQ(node->status(), NodeStatus::RUNNING);
        else        
            ASSERT_EQ(node->status(), NodeStatus::IDLE);
    }

    // Switching to action_42
    std::this_thread::sleep_for(milliseconds(10));
    tree->blackboard_stack[0]->set("my_var", "42");
    tree->root_node->executeTick();
    for (auto& node : tree->nodes)
    {
        if(node->name() == "simple_switch" || node->name() == "action_42")
            ASSERT_EQ(node->status(), NodeStatus::RUNNING);
        else        
            ASSERT_EQ(node->status(), NodeStatus::IDLE);
    }

    // Switch succeed
    std::this_thread::sleep_for(milliseconds(110));
    tree->blackboard_stack[0]->set("my_var", "42");
    tree->root_node->executeTick();
    for (auto& node : tree->nodes)
    {
        if(node->name() == "simple_switch")
            ASSERT_EQ(node->status(), NodeStatus::SUCCESS);
        else        
            ASSERT_EQ(node->status(), NodeStatus::IDLE);
    }
}