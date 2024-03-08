#include <gtest/gtest.h>
#include "action_test_node.h"
#include "condition_test_node.h"
#include "behaviortree_cpp/behavior_tree.h"
#include "behaviortree_cpp/tree_node.h"
#include "behaviortree_cpp/bt_factory.h"

using BT::NodeStatus;
using std::chrono::milliseconds;

static const char* xml_text = R"(

<root BTCPP_format="4" >

    <BehaviorTree ID="MainTree">
        <Switch3 name="simple_switch" variable="{my_var}"  case_1="1" case_2="42 case_3="666" >
            <AsyncActionTest name="action_1"/>
            <AsyncActionTest name="action_42"/>
            <AsyncActionTest name="action_666"/>
            <AsyncActionTest name="action_default"/>
        </Switch3>
    </BehaviorTree>
</root>
        )";

BT::NodeStatus TickWhileRunning(BT::TreeNode& node)
{
  auto status = node.executeTick();
  while(status == BT::NodeStatus::RUNNING)
  {
    status = node.executeTick();
  }
  return status;
}

struct SwitchTest : testing::Test
{
  using Switch2 = BT::SwitchNode<2>;
  std::unique_ptr<Switch2> root;
  BT::AsyncActionTest action_1;
  BT::AsyncActionTest action_42;
  BT::AsyncActionTest action_def;
  BT::Blackboard::Ptr bb = BT::Blackboard::create();
  BT::NodeConfig simple_switch_config_;

  SwitchTest()
    : action_1("action_1", milliseconds(200))
    , action_42("action_42", milliseconds(200))
    , action_def("action_default", milliseconds(200))
  {
    BT::PortsRemapping input;
    input.insert(std::make_pair("variable", "{my_var}"));
    input.insert(std::make_pair("case_1", "1"));
    input.insert(std::make_pair("case_2", "42"));

    BT::NodeConfig simple_switch_config_;
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

  std::this_thread::sleep_for(milliseconds(300));
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

  std::this_thread::sleep_for(milliseconds(300));
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

  std::this_thread::sleep_for(milliseconds(300));
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

  std::this_thread::sleep_for(milliseconds(300));
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

  std::this_thread::sleep_for(milliseconds(20));
  state = root->executeTick();
  ASSERT_EQ(NodeStatus::RUNNING, action_1.status());
  ASSERT_EQ(NodeStatus::IDLE, action_42.status());
  ASSERT_EQ(NodeStatus::IDLE, action_def.status());
  ASSERT_EQ(NodeStatus::RUNNING, state);

  // Switch Node does not feels changes. Only when tick.
  // (not reactive)
  std::this_thread::sleep_for(milliseconds(20));
  bb->set("my_var", "");
  std::this_thread::sleep_for(milliseconds(20));
  ASSERT_EQ(NodeStatus::RUNNING, action_1.status());
  ASSERT_EQ(NodeStatus::IDLE, action_42.status());
  ASSERT_EQ(NodeStatus::IDLE, action_def.status());
  ASSERT_EQ(NodeStatus::RUNNING, root->status());

  std::this_thread::sleep_for(milliseconds(20));
  state = root->executeTick();
  ASSERT_EQ(NodeStatus::IDLE, action_1.status());
  ASSERT_EQ(NodeStatus::IDLE, action_42.status());
  ASSERT_EQ(NodeStatus::RUNNING, action_def.status());
  ASSERT_EQ(NodeStatus::RUNNING, state);

  std::this_thread::sleep_for(milliseconds(300));
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
  std::this_thread::sleep_for(milliseconds(20));
  state = root->executeTick();
  ASSERT_EQ(NodeStatus::IDLE, action_1.status());
  ASSERT_EQ(NodeStatus::RUNNING, action_42.status());
  ASSERT_EQ(NodeStatus::IDLE, action_def.status());
  ASSERT_EQ(NodeStatus::RUNNING, state);

  std::this_thread::sleep_for(milliseconds(300));
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

  action_1.setExpectedResult(NodeStatus::FAILURE);

  ASSERT_EQ(NodeStatus::RUNNING, action_1.status());
  ASSERT_EQ(NodeStatus::IDLE, action_42.status());
  ASSERT_EQ(NodeStatus::IDLE, action_def.status());
  ASSERT_EQ(NodeStatus::RUNNING, state);

  std::this_thread::sleep_for(milliseconds(300));
  state = root->executeTick();

  ASSERT_EQ(NodeStatus::FAILURE, state);
  ASSERT_EQ(NodeStatus::IDLE, action_1.status());
  ASSERT_EQ(NodeStatus::IDLE, action_42.status());
  ASSERT_EQ(NodeStatus::IDLE, action_def.status());
}
