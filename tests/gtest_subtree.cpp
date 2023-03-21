#include <gtest/gtest.h>
#include "behaviortree_cpp/bt_factory.h"
#include "../sample_nodes/dummy_nodes.h"
#include "../sample_nodes/movebase_node.h"

using namespace BT;

TEST(SubTree, SiblingPorts_Issue_72)
{
  static const char* xml_text = R"(

<root BTCPP_format="4" main_tree_to_execute="MainTree" >

    <BehaviorTree ID="MainTree">
        <Sequence>
            <Script code = " myParam = 'hello' " />
            <SubTree ID="mySubtree" param="{myParam}" />
            <Script code = " myParam = 'world' " />
            <SubTree ID="mySubtree" param="{myParam}" />
        </Sequence>
    </BehaviorTree>

    <BehaviorTree ID="mySubtree">
            <SaySomething message="{param}" />
    </BehaviorTree>
</root> )";

  BehaviorTreeFactory factory;
  factory.registerNodeType<DummyNodes::SaySomething>("SaySomething");

  Tree tree = factory.createTreeFromText(xml_text);

  for (auto& subtree : tree.subtrees)
  {
    subtree->blackboard->debugMessage();
    std::cout << "-----" << std::endl;
  }

  auto status = tree.tickWhileRunning();
  ASSERT_EQ(status, NodeStatus::SUCCESS);
  ASSERT_EQ(tree.subtrees.size(), 3);
}

class CopyPorts : public BT::SyncActionNode
{
public:
  CopyPorts(const std::string& name, const BT::NodeConfig& config) :
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

<root BTCPP_format="4" main_tree_to_execute="MainTree">

    <BehaviorTree ID="MainTree">
        <Sequence>
            <Script code = " thoughts = 'hello' " />
            <SubTree ID="CopySubtree" in_arg="{thoughts}" out_arg="{greetings}"/>
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

  auto status = tree.tickWhileRunning();
  ASSERT_EQ(status, NodeStatus::SUCCESS);
}

TEST(SubTree, BadRemapping)
{
  BehaviorTreeFactory factory;
  factory.registerNodeType<DummyNodes::SaySomething>("SaySomething");
  factory.registerNodeType<CopyPorts>("CopyPorts");

  static const char* xml_text_bad_in = R"(
<root BTCPP_format="4" >

    <BehaviorTree ID="MainTree">
        <Sequence>
            <Script code = " thoughts='hello' " />
            <SubTree ID="CopySubtree" out_arg="{greetings}"/>
            <SaySomething  message="{greetings}" />
        </Sequence>
    </BehaviorTree>

    <BehaviorTree ID="CopySubtree">
            <CopyPorts in="{in_arg}" out="{out_arg}"/>
    </BehaviorTree>
</root> )";

  factory.registerBehaviorTreeFromText(xml_text_bad_in);
  Tree tree_bad_in = factory.createTree("MainTree");
  EXPECT_ANY_THROW(tree_bad_in.tickWhileRunning());

  static const char* xml_text_bad_out = R"(
<root BTCPP_format="4" >

    <BehaviorTree ID="MainTree">
        <Sequence>
            <Script code = " thoughts='hello' " />
            <SubTree ID="CopySubtree" in_arg="{thoughts}"/>
            <SaySomething  message="{greetings}" />
        </Sequence>
    </BehaviorTree>

    <BehaviorTree ID="CopySubtree">
            <CopyPorts in="{in_arg}" out="{out_arg}"/>
    </BehaviorTree>
</root> )";

  factory.registerBehaviorTreeFromText(xml_text_bad_out);
  Tree tree_bad_out = factory.createTree("MainTree");
  EXPECT_ANY_THROW(tree_bad_out.tickWhileRunning());
}

TEST(SubTree, SubtreePlusA)
{
  static const char* xml_text = R"(

<root BTCPP_format="4" >

    <BehaviorTree ID="MainTree">
        <Sequence>
            <Script code = "myParam = 'Hello' " />
            <SubTree ID="mySubtree" param="{myParam}" />
            <SubTree ID="mySubtree" param="World" />
            <Script code = "param = 'Auto remapped' " />
            <SubTree ID="mySubtree" _autoremap="1"  />
        </Sequence>
    </BehaviorTree>

    <BehaviorTree ID="mySubtree">
            <SaySomething message="{param}" />
    </BehaviorTree>
</root> )";

  BehaviorTreeFactory factory;
  factory.registerNodeType<DummyNodes::SaySomething>("SaySomething");

  factory.registerBehaviorTreeFromText(xml_text);
  Tree tree = factory.createTree("MainTree");

  auto status = tree.tickWhileRunning();
  ASSERT_EQ(status, NodeStatus::SUCCESS);
}

TEST(SubTree, SubtreePlusB)
{
  static const char* xml_text = R"(

<root BTCPP_format="4" >

    <BehaviorTree ID="MainTree">
        <Sequence>
            <Script code = "myParam = 'Hello World'; param3='Auto remapped' " />
            <SubTree ID="mySubtree" _autoremap="1" param1="{myParam}" param2="Straight Talking" />
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

  factory.registerBehaviorTreeFromText(xml_text);
  Tree tree = factory.createTree("MainTree");

  auto status = tree.tickWhileRunning();
  ASSERT_EQ(status, NodeStatus::SUCCESS);
}

class ReadInConstructor : public BT::SyncActionNode
{
public:
  ReadInConstructor(const std::string& name, const BT::NodeConfig& config) :
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
  BT::NodeConfig config;
  config.blackboard = BT::Blackboard::create();
  static const char* xml_text = R"(

<root BTCPP_format="4" >

    <BehaviorTree ID="MainTree">
        <Sequence>
            <SubTree ID="mySubtree" _autoremap="1"/>
        </Sequence>
    </BehaviorTree>
    <BehaviorTree ID="mySubtree">
            <ReadInConstructor message="{message}" />
    </BehaviorTree>
</root> )";

  BT::BehaviorTreeFactory factory;
  factory.registerNodeType<ReadInConstructor>("ReadInConstructor");
  config.blackboard->set("message", "hello");

  factory.registerBehaviorTreeFromText(xml_text);
  Tree tree = factory.createTree("MainTree", config.blackboard);

  auto status = tree.tickWhileRunning();
  ASSERT_EQ(status, BT::NodeStatus::SUCCESS);
}

// TODO: only explicit remapping work, autoremapping fails
TEST(SubTree, ScriptRemap)
{
  static const char* xml_text = R"(

<root BTCPP_format="4" >

    <BehaviorTree ID="MainTree">
        <Sequence>
            <Script code = "value:=0" />
            <SubTree ID="mySubtree" value="{value}"  />
        </Sequence>
    </BehaviorTree>

    <BehaviorTree ID="mySubtree">
        <Script code = "value:=1" />
    </BehaviorTree>
</root> )";

  BT::BehaviorTreeFactory factory;
  factory.registerBehaviorTreeFromText(xml_text);

  Tree tree = factory.createTree("MainTree");
  tree.tickOnce();

  ASSERT_EQ(tree.subtrees[1]->blackboard->get<int>("value"), 1);
  ASSERT_EQ(tree.subtrees[0]->blackboard->get<int>("value"), 1);
}

class ModifyPose : public BT::SyncActionNode
{
public:
  // Any TreeNode with ports must have a constructor with this signature
  ModifyPose(const std::string& name, const BT::NodeConfig& config)
    : SyncActionNode(name, config)
  {}

  static BT::PortsList providedPorts() {
    return{ BT::BidirectionalPort<Pose2D>("pose") };
  }

  BT::NodeStatus tick() override {
    Pose2D pose;
    getInput("pose", pose);
    pose.theta *= 2;
    setOutput("pose", pose);
    return NodeStatus::SUCCESS;
  }
};

TEST(SubTree, StringConversions_Issue530)
{
  const char* xml_text = R"(
<root BTCPP_format="4" >
  <BehaviorTree ID="MainTree">
    <Sequence>
      <Script code=" pose:='1;2;3' "/>
      <ModifyPose pose="{pose}"/>
      <Script code=" pose:='1;2;3' "/>
    </Sequence>
  </BehaviorTree>
</root>
)";

  BT::BehaviorTreeFactory factory;
  factory.registerNodeType<ModifyPose>("ModifyPose");
  factory.registerBehaviorTreeFromText(xml_text);
  Tree tree = factory.createTree("MainTree");
  tree.tickOnce();
}
