#include <gtest/gtest.h>
#include "behaviortree_cpp_v3/bt_factory.h"
#include "../sample_nodes/dummy_nodes.h"

using namespace BT;

TEST(SubTree, SiblingPorts_Issue_72)
{
  static const char* xml_text = R"(

<root main_tree_to_execute = "MainTree" >

    <BehaviorTree ID="MainTree">
        <Sequence>
            <SetBlackboard value="hello" output_key="myParam" />
            <SubTree ID="mySubtree" param="myParam" />
            <SetBlackboard value="world" output_key="myParam" />
            <SubTree ID="mySubtree" param="myParam" />
        </Sequence>
    </BehaviorTree>

    <BehaviorTree ID="mySubtree">
            <SaySomething ID="AlwaysSuccess" message="{param}" />
    </BehaviorTree>
</root> )";

  BehaviorTreeFactory factory;
  factory.registerNodeType<DummyNodes::SaySomething>("SaySomething");

  Tree tree = factory.createTreeFromText(xml_text);

  for (auto& bb : tree.blackboard_stack)
  {
    bb->debugMessage();
    std::cout << "-----" << std::endl;
  }

  auto ret = tree.tickRoot();

  ASSERT_EQ(ret, NodeStatus::SUCCESS);
  ASSERT_EQ(tree.blackboard_stack.size(), 3);
}

class CopyPorts : public BT::SyncActionNode
{
public:
  CopyPorts(const std::string& name, const BT::NodeConfiguration& config) :
    BT::SyncActionNode(name, config)
  {}

  BT::NodeStatus tick() override
  {
    auto msg = getInput<std::string>("in");
    if (!msg)
    {
      throw BT::RuntimeError("missing required input [message]: ", msg.error());
    }
    setOutput("out", msg.value());
    return BT::NodeStatus::SUCCESS;
  }

  static BT::PortsList providedPorts()
  {
    return {BT::InputPort<std::string>("in"), BT::OutputPort<std::string>("out")};
  }
};

TEST(SubTree, GoodRemapping)
{
  static const char* xml_text = R"(

<root main_tree_to_execute = "MainTree" >

    <BehaviorTree ID="MainTree">
        <Sequence>
            <SetBlackboard value="hello" output_key="thoughts" />
            <SubTree ID="CopySubtree" in_arg="thoughts" out_arg="greetings"/>
            <SaySomething  message="{greetings}" />
        </Sequence>
    </BehaviorTree>

    <BehaviorTree ID="CopySubtree">
            <CopyPorts in="{in_arg}" out="{out_arg}"/>
    </BehaviorTree>
</root> )";

  BehaviorTreeFactory factory;
  factory.registerNodeType<DummyNodes::SaySomething>("SaySomething");
  factory.registerNodeType<CopyPorts>("CopyPorts");

  Tree tree = factory.createTreeFromText(xml_text);
  auto ret = tree.tickRoot();
  ASSERT_EQ(ret, NodeStatus::SUCCESS);
}

TEST(SubTree, BadRemapping)
{
  BehaviorTreeFactory factory;
  factory.registerNodeType<DummyNodes::SaySomething>("SaySomething");
  factory.registerNodeType<CopyPorts>("CopyPorts");

  static const char* xml_text_bad_in = R"(
<root main_tree_to_execute = "MainTree" >

    <BehaviorTree ID="MainTree">
        <Sequence>
            <SetBlackboard value="hello" output_key="thoughts" />
            <SubTree ID="CopySubtree" out_arg="greetings"/>
            <SaySomething  message="{greetings}" />
        </Sequence>
    </BehaviorTree>

    <BehaviorTree ID="CopySubtree">
            <CopyPorts in="{in_arg}" out="{out_arg}"/>
    </BehaviorTree>
</root> )";

  Tree tree_bad_in = factory.createTreeFromText(xml_text_bad_in);
  EXPECT_ANY_THROW(tree_bad_in.tickRoot());

  static const char* xml_text_bad_out = R"(
<root main_tree_to_execute = "MainTree" >

    <BehaviorTree ID="MainTree">
        <Sequence>
            <SetBlackboard value="hello" output_key="thoughts" />
            <SubTree ID="CopySubtree" in_arg="thoughts"/>
            <SaySomething  message="{greetings}" />
        </Sequence>
    </BehaviorTree>

    <BehaviorTree ID="CopySubtree">
            <CopyPorts in="{in_arg}" out="{out_arg}"/>
    </BehaviorTree>
</root> )";

  Tree tree_bad_out = factory.createTreeFromText(xml_text_bad_out);
  EXPECT_ANY_THROW(tree_bad_out.tickRoot());
}

TEST(SubTree, SubtreePlusA)
{
  static const char* xml_text = R"(

<root main_tree_to_execute = "MainTree" >

    <BehaviorTree ID="MainTree">
        <Sequence>
            <SetBlackboard value="Hello" output_key="myParam" />
            <SubTreePlus ID="mySubtree" param="{myParam}" />
            <SubTreePlus ID="mySubtree" param="World" />
            <SetBlackboard value="Auto remapped" output_key="param" />
            <SubTreePlus ID="mySubtree" __autoremap="1"  />
        </Sequence>
    </BehaviorTree>

    <BehaviorTree ID="mySubtree">
            <SaySomething message="{param}" />
    </BehaviorTree>
</root> )";

  BehaviorTreeFactory factory;
  factory.registerNodeType<DummyNodes::SaySomething>("SaySomething");

  Tree tree = factory.createTreeFromText(xml_text);
  auto ret = tree.tickRoot();
  ASSERT_EQ(ret, NodeStatus::SUCCESS);
}

TEST(SubTree, SubtreePlusB)
{
  static const char* xml_text = R"(

<root main_tree_to_execute = "MainTree" >

    <BehaviorTree ID="MainTree">
        <Sequence>
            <SetBlackboard value="Hello World" output_key="myParam" />
            <SetBlackboard value="Auto remapped" output_key="param3" />
            <SubTreePlus ID="mySubtree" __autoremap="1" param1="{myParam}" param2="Straight Talking" />
        </Sequence>
    </BehaviorTree>

    <BehaviorTree ID="mySubtree">
        <Sequence>
            <SaySomething message="{param1}" />
            <SaySomething message="{param2}" />
            <SaySomething message="{param3}" />
        </Sequence>
    </BehaviorTree>
</root> )";

  BehaviorTreeFactory factory;
  factory.registerNodeType<DummyNodes::SaySomething>("SaySomething");

  Tree tree = factory.createTreeFromText(xml_text);
  auto ret = tree.tickRoot();
  ASSERT_EQ(ret, NodeStatus::SUCCESS);
}

TEST(SubTree, SubtreePlusC)
{
  static const char* xml_text = R"(

<root main_tree_to_execute = "MainTree" >

    <BehaviorTree ID="MainTree">
        <Sequence>
            <SetBlackboard value="Hello" output_key="param1" />
            <SetBlackboard value="World" output_key="param2" />
            <SubTree ID="mySubtree" __shared_blackboard="true"/>
        </Sequence>
    </BehaviorTree>

    <BehaviorTree ID="mySubtree">
        <Sequence>
            <SaySomething message="{param1}" />
            <SaySomething message="{param2}" />
        </Sequence>
    </BehaviorTree>
</root> )";

  BehaviorTreeFactory factory;
  factory.registerNodeType<DummyNodes::SaySomething>("SaySomething");

  Tree tree = factory.createTreeFromText(xml_text);
  auto ret = tree.tickRoot();
  ASSERT_EQ(ret, NodeStatus::SUCCESS);
}

class ReadInConstructor : public BT::SyncActionNode
{
public:
  ReadInConstructor(const std::string& name, const BT::NodeConfiguration& config) :
    BT::SyncActionNode(name, config)
  {
    auto msg = getInput<std::string>("message");
    if (!msg)
    {
      throw BT::RuntimeError("missing required input [message]: ", msg.error());
    }
  }

  BT::NodeStatus tick() override
  {
    return BT::NodeStatus::SUCCESS;
  }
  static BT::PortsList providedPorts()
  {
    return {BT::InputPort<std::string>("message")};
  }
};

TEST(SubTree, SubtreePlusD)
{
  BT::NodeConfiguration config;
  config.blackboard = BT::Blackboard::create();
  static const char* xml_text = R"(

<root main_tree_to_execute = "MainTree" >

    <BehaviorTree ID="MainTree">
        <Sequence>
            <SubTreePlus ID="mySubtree" __autoremap="1"/>
        </Sequence>
    </BehaviorTree>
    <BehaviorTree ID="mySubtree">
            <ReadInConstructor message="{message}" />
    </BehaviorTree>
</root> )";

  BT::BehaviorTreeFactory factory;
  factory.registerNodeType<ReadInConstructor>("ReadInConstructor");
  config.blackboard->set("message", "hello");
  BT::Tree tree = factory.createTreeFromText(xml_text, config.blackboard);
  auto ret = tree.tickRoot();
  ASSERT_EQ(ret, BT::NodeStatus::SUCCESS);
}

TEST(SubTree, SubtreeIssue433)
{
  BT::NodeConfiguration config;
  config.blackboard = BT::Blackboard::create();
  static const char* xml_text = R"(

<root main_tree_to_execute = "TestTree" >
    <BehaviorTree ID="Subtree1">
        <Decorator ID="Repeat" num_cycles="{port_to_use}">
            <Action ID="AlwaysSuccess"/>
        </Decorator>
    </BehaviorTree>

    <BehaviorTree ID="Subtree2">
        <Action ID="SetBlackboard" output_key="test_port" value="{port_to_read}"/>
    </BehaviorTree>

    <BehaviorTree ID="TestTree">
        <Sequence>
            <Action ID="SetBlackboard" output_key="test_port" value="1"/>
            <SubTree ID="Subtree1" port_to_use="test_port"/>
            <SubTree ID="Subtree2" port_to_read="test_port"/>
        </Sequence>
    </BehaviorTree>
</root> )";

  BT::BehaviorTreeFactory factory;

  BT::Tree tree = factory.createTreeFromText(xml_text, config.blackboard);
  auto ret = tree.tickRoot();

  ASSERT_EQ(ret, BT::NodeStatus::SUCCESS);
}
