#include <gtest/gtest.h>
#include "behaviortree_cpp_v3/bt_factory.h"
#include "../sample_nodes/dummy_nodes.h"
#include "../sample_nodes/movebase_node.h"

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
            <SaySomething message="{param}" />
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


class NaughtyNav2Node : public BT::SyncActionNode
{
public:
  NaughtyNav2Node(const std::string& name, const BT::NodeConfiguration& config) :
    BT::SyncActionNode(name, config)
  {
    std::cout << "CTOR:" << config.blackboard->get<std::string>("ros_node") << std::endl;
  }

  BT::NodeStatus tick() override
  {
    std::cout << "tick:" << config().blackboard->get<std::string>("ros_node") << std::endl;
    return BT::NodeStatus::SUCCESS;
  }
  static BT::PortsList providedPorts()
  {
    return {};
  }
};

TEST(SubTree, SubtreeNav2_Issue563)
{
  static const char* xml_text = R"(
<root main_tree_to_execute="Tree1">

    <BehaviorTree ID="Tree1">
      <Sequence>
        <SetBlackboard output_key="the_message" value="hello world"/>
        <SubTreePlus ID="Tree2" __autoremap="true"/>
        <SaySomething message="{reply}" />
      </Sequence>
    </BehaviorTree>

    <BehaviorTree ID="Tree2">
        <SubTreePlus ID="Tree3" __autoremap="true"/>
    </BehaviorTree>

    <BehaviorTree ID="Tree3">
        <SubTreePlus ID="Talker" __autoremap="true"/>
    </BehaviorTree>

    <BehaviorTree ID="Talker">
      <Sequence>
        <SaySomething message="{the_message}" />
        <SetBlackboard output_key="reply" value="done"/>
        <NaughtyNav2Node/>
      </Sequence>
    </BehaviorTree>

</root>)";

  BehaviorTreeFactory factory;
  factory.registerNodeType<DummyNodes::SaySomething>("SaySomething");
  factory.registerNodeType<NaughtyNav2Node>("NaughtyNav2Node");

  auto blackboard = BT::Blackboard::create();
  blackboard->set<std::string>("ros_node", "nav2_shouldnt_do_this");

  Tree tree = factory.createTreeFromText(xml_text, blackboard);

  auto ret = tree.tickRoot();
  ASSERT_EQ(ret, NodeStatus::SUCCESS);
}

TEST(SubTree, SubtreeNav2_Issue724)
{
  static const char* xml_text = R"(
<root main_tree_to_execute="Tree1">

    <BehaviorTree ID="Tree1">
      <Sequence>
        <SubTreePlus ID="Tree2" ros_node="{ros_node}"/>
      </Sequence>
    </BehaviorTree>

    <BehaviorTree ID="Tree2">
        <SubTreePlus ID="Tree3" ros_node="{ros_node}"/>
    </BehaviorTree>

    <BehaviorTree ID="Tree3">
        <SubTreePlus ID="Talker" ros_node="{ros_node}"/>
    </BehaviorTree>

    <BehaviorTree ID="Talker">
      <Sequence>
        <NaughtyNav2Node/>
      </Sequence>
    </BehaviorTree>

</root>)";

  BehaviorTreeFactory factory;
  factory.registerNodeType<NaughtyNav2Node>("NaughtyNav2Node");

  factory.registerBehaviorTreeFromText(xml_text);

  auto blackboard = BT::Blackboard::create();
  blackboard->set<std::string>("ros_node", "nav2_shouldnt_do_this");

  Tree tree = factory.createTreeFromText(xml_text, blackboard);

  auto ret = tree.tickRoot();
  ASSERT_EQ(ret, NodeStatus::SUCCESS);
}

TEST(SubTree, String_to_Pose_Issue623)
{
  // clang-format off

  static const char* xml_text = R"(
<root main_tree_to_execute="Test">
  <BehaviorTree ID="Test">
    <ReactiveSequence name="MainSequence">
      <SubTreePlus name="Visit2" ID="Visit2" tl1="1;2;3"/>
    </ReactiveSequence>
  </BehaviorTree>
  <BehaviorTree ID="Visit2">
    <Sequence name="Visit2MainSequence">
      <Action name="MoveBase" ID="MoveBase" goal="{tl1}"/>
    </Sequence>
  </BehaviorTree>
</root>
 )";

  // clang-format on

  BehaviorTreeFactory factory;
  factory.registerNodeType<MoveBaseAction>("MoveBase");
  auto tree = factory.createTreeFromText(xml_text);
  tree.tickRootWhileRunning();
}

class Assert : public BT::SyncActionNode
{
public:
  Assert(const std::string& name, const BT::NodeConfiguration& config)
    : BT::SyncActionNode(name, config) {}

  static BT::PortsList providedPorts() {
    return {BT::InputPort<bool>("condition")};
  }

private:
  virtual BT::NodeStatus tick() override {
    if (getInput<bool>("condition").value())
      return BT::NodeStatus::SUCCESS;
    else
      return BT::NodeStatus::FAILURE;
  }
};

TEST(SubTree, Issue653_SetBlackboard)
{
  // clang-format off

  static const char* xml_text = R"(
<root main_tree_to_execute = "MainTree">
  <BehaviorTree ID="MainTree">
    <Sequence>
      <SubTreePlus ID="Init" test="{test}" />
      <Assert condition="{test}" />
    </Sequence>
  </BehaviorTree>

  <BehaviorTree ID="Init">
    <SetBlackboard output_key="test" value="true"/>
  </BehaviorTree>
</root>
 )";

  // clang-format on

  BehaviorTreeFactory factory;
  factory.registerNodeType<Assert>("Assert");
  auto tree = factory.createTreeFromText(xml_text);
  tree.tickRootWhileRunning();
}
