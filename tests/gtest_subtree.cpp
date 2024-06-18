#include <gtest/gtest.h>
#include "behaviortree_cpp/bt_factory.h"
#include "../sample_nodes/dummy_nodes.h"
#include "../sample_nodes/movebase_node.h"
#include "test_helper.hpp"

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

  for(auto& subtree : tree.subtrees)
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
  CopyPorts(const std::string& name, const BT::NodeConfig& config)
    : BT::SyncActionNode(name, config)
  {}

  BT::NodeStatus tick() override
  {
    auto msg = getInput<std::string>("in");
    if(!msg)
    {
      throw BT::RuntimeError("missing required input [message]: ", msg.error());
    }
    setOutput("out", msg.value());
    return BT::NodeStatus::SUCCESS;
  }

  static BT::PortsList providedPorts()
  {
    return { BT::InputPort<std::string>("in"), BT::OutputPort<std::string>("out") };
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
  ReadInConstructor(const std::string& name, const BT::NodeConfig& config)
    : BT::SyncActionNode(name, config)
  {
    auto msg = getInput<std::string>("message");
    if(!msg)
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
    return { BT::InputPort<std::string>("message") };
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

  static BT::PortsList providedPorts()
  {
    return { BT::BidirectionalPort<Pose2D>("pose") };
  }

  BT::NodeStatus tick() override
  {
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

class NaughtyNav2Node : public BT::SyncActionNode
{
public:
  NaughtyNav2Node(const std::string& name, const BT::NodeConfiguration& config)
    : BT::SyncActionNode(name, config)
  {
    std::cout << "CTOR:" << config.blackboard->get<std::string>("ros_node") << std::endl;
  }

  BT::NodeStatus tick() override
  {
    std::cout << "tick:" << config().blackboard->get<std::string>("ros_node")
              << std::endl;
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
<root BTCPP_format="4" >

    <BehaviorTree ID="Tree1">
      <Sequence>
        <SetBlackboard output_key="the_message" value="hello world"/>
        <SubTree ID="Tree2" _autoremap="true"/>
        <SaySomething message="{reply}" />
      </Sequence>
    </BehaviorTree>

    <BehaviorTree ID="Tree2">
        <SubTree ID="Tree3" _autoremap="true"/>
    </BehaviorTree>

    <BehaviorTree ID="Tree3">
        <SubTree ID="Talker" _autoremap="true"/>
    </BehaviorTree>

    <BehaviorTree ID="Talker">
      <Sequence>
        <SaySomething message="{the_message}" />
        <Script code=" reply:='done' "/>
        <NaughtyNav2Node/>
      </Sequence>
    </BehaviorTree>

</root>)";

  BehaviorTreeFactory factory;
  factory.registerNodeType<DummyNodes::SaySomething>("SaySomething");
  factory.registerNodeType<NaughtyNav2Node>("NaughtyNav2Node");

  factory.registerBehaviorTreeFromText(xml_text);

  auto blackboard = BT::Blackboard::create();
  blackboard->set<std::string>("ros_node", "nav2_shouldnt_do_this");

  Tree tree = factory.createTree("Tree1", blackboard);

  auto ret = tree.tickOnce();
  ASSERT_EQ(ret, NodeStatus::SUCCESS);
}

TEST(SubTree, SubtreeNav2_Issue724)
{
  static const char* xml_text = R"(
<root BTCPP_format="4" >

    <BehaviorTree ID="Tree1">
      <Sequence>
        <SubTree ID="Tree2" ros_node="{ros_node}"/>
      </Sequence>
    </BehaviorTree>

    <BehaviorTree ID="Tree2">
        <SubTree ID="Tree3" ros_node="{ros_node}"/>
    </BehaviorTree>

    <BehaviorTree ID="Tree3">
        <SubTree ID="Talker" ros_node="{ros_node}"/>
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

  Tree tree = factory.createTree("Tree1", blackboard);

  auto ret = tree.tickOnce();
  ASSERT_EQ(ret, NodeStatus::SUCCESS);
}

TEST(SubTree, SubtreeIssue592)
{
  static const char* xml_text = R"(
<root BTCPP_format="4" >

  <BehaviorTree ID="Outer_Tree">
    <Sequence>
      <Script code="variable := 'test'"/>
      <Script code="var := 'test'"/>
      <SubTree ID="Inner_Tree" _autoremap="false" variable="{var}" />
      <SubTree ID="Inner_Tree" _autoremap="true"/>
    </Sequence>
  </BehaviorTree>

  <BehaviorTree ID="Inner_Tree">
    <Sequence>
      <TestA _skipIf="variable != 'test'"/>
    </Sequence>
  </BehaviorTree>

</root>)";

  BehaviorTreeFactory factory;
  std::array<int, 1> counters;
  RegisterTestTick(factory, "Test", counters);

  factory.registerBehaviorTreeFromText(xml_text);
  Tree tree = factory.createTree("Outer_Tree");

  auto ret = tree.tickWhileRunning();
  ASSERT_EQ(ret, NodeStatus::SUCCESS);
  ASSERT_EQ(counters[0], 2);
}

TEST(SubTree, Issue623_String_to_Pose2d)
{
  // clang-format off

  static const char* xml_text = R"(
<root main_tree_to_execute="Test" BTCPP_format="4">

  <BehaviorTree ID="Test">
    <ReactiveSequence name="MainSequence">
      <SubTree name="Visit2" ID="Visit2" tl1="1;2;3"/>
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
  tree.tickWhileRunning();
}

class Assert : public BT::SyncActionNode
{
public:
  Assert(const std::string& name, const BT::NodeConfiguration& config)
    : BT::SyncActionNode(name, config)
  {}

  static BT::PortsList providedPorts()
  {
    return { BT::InputPort<bool>("condition") };
  }

private:
  virtual BT::NodeStatus tick() override
  {
    if(getInput<bool>("condition").value())
      return BT::NodeStatus::SUCCESS;
    else
      return BT::NodeStatus::FAILURE;
  }
};

TEST(SubTree, Issue653_SetBlackboard)
{
  // clang-format off

  static const char* xml_text = R"(
<root main_tree_to_execute = "MainTree" BTCPP_format="4">
  <BehaviorTree ID="MainTree">
    <Sequence>
      <SubTree ID="Init" test="{test}" />
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
  tree.tickWhileRunning();
}

TEST(SubTree, SubtreeModels)
{
  // clang-format off

  static const char* xml_text = R"(
<root main_tree_to_execute = "MainTree" BTCPP_format="4">
  <TreeNodesModel>
    <SubTree ID="MySub">
      <input_port name="in_value" default="42"/>
      <input_port name="in_name"/>
      <output_port name="out_result" default="{output}"/>
      <output_port name="out_state"/>
    </SubTree>
  </TreeNodesModel>

  <BehaviorTree ID="MainTree">
    <Sequence>
      <Script code="my_name:= 'john' "/>
      <SubTree ID="MySub" in_name="{my_name}"  out_state="{my_state}"/>
      <ScriptCondition code=" output==69 && my_state=='ACTIVE' " />
    </Sequence>
  </BehaviorTree>

  <BehaviorTree ID="MySub">
    <Sequence>
      <ScriptCondition code="in_name=='john' && in_value==42" />
      <Script code="out_result:=69; out_state:='ACTIVE'" />
    </Sequence>
  </BehaviorTree>
</root>
 )";

  // clang-format on

  BehaviorTreeFactory factory;
  auto tree = factory.createTreeFromText(xml_text);
  tree.tickWhileRunning();
}

class PrintToConsole : public BT::SyncActionNode
{
public:
  PrintToConsole(const std::string& name, const BT::NodeConfiguration& config,
                 std::vector<std::string>* console)
    : BT::SyncActionNode(name, config), console_(console)
  {}

  static BT::PortsList providedPorts()
  {
    return { BT::InputPort<std::string>("message") };
  }

private:
  virtual BT::NodeStatus tick() override
  {
    if(auto res = getInput<std::string>("message"))
    {
      console_->push_back(res.value());
      return BT::NodeStatus::SUCCESS;
    }
    else
      return BT::NodeStatus::FAILURE;
  }
  std::vector<std::string>* console_;
};

TEST(SubTree, RemappingIssue696)
{
  // clang-format off

  static const char* xml_text = R"(
  <root BTCPP_format="4">
    <BehaviorTree ID="Subtree1">\n"
      <Sequence>
        <PrintToConsole message="{msg1}"/>
        <PrintToConsole message="{msg2}"/>
      </Sequence>
    </BehaviorTree>

    <BehaviorTree ID="Subtree2">
      <Sequence>
        <SubTree ID="Subtree1" msg1="foo1" _autoremap="true"/>
        <SubTree ID="Subtree1" msg1="foo2" _autoremap="true"/>
      </Sequence>
    </BehaviorTree>

    <BehaviorTree ID="MainTree">
      <SubTree ID="Subtree2" msg2="bar"/>
    </BehaviorTree>
  </root>
 )";

  // clang-format on

  BehaviorTreeFactory factory;
  std::vector<std::string> console;
  factory.registerNodeType<PrintToConsole>("PrintToConsole", &console);

  factory.registerBehaviorTreeFromText(xml_text);
  auto tree = factory.createTree("MainTree");
  tree.tickWhileRunning();

  ASSERT_EQ(console.size(), 4);
  ASSERT_EQ(console[0], "foo1");
  ASSERT_EQ(console[1], "bar");
  ASSERT_EQ(console[2], "foo2");
  ASSERT_EQ(console[3], "bar");
}

TEST(SubTree, PrivateAutoRemapping)
{
  // clang-format off

  static const char* xml_text = R"(
  <root BTCPP_format="4">
    <BehaviorTree ID="Subtree">\n"
      <Sequence>
        <SetBlackboard output_key="public_value"   value="hello"/>
        <SetBlackboard output_key="_private_value" value="world"/>
      </Sequence>
    </BehaviorTree>

    <BehaviorTree ID="MainTree">
      <Sequence>
        <SubTree ID="Subtree" _autoremap="true"/>
        <PrintToConsole message="{public_value}"/>
        <PrintToConsole message="{_private_value}"/>
      </Sequence>
    </BehaviorTree>
  </root>
 )";

  // clang-format on
  BehaviorTreeFactory factory;
  std::vector<std::string> console;
  factory.registerNodeType<PrintToConsole>("PrintToConsole", &console);

  factory.registerBehaviorTreeFromText(xml_text);
  auto tree = factory.createTree("MainTree");
  const auto res = tree.tickWhileRunning();

  // should fail because _private_value is not autoremapped
  ASSERT_EQ(res, BT::NodeStatus::FAILURE);
  ASSERT_EQ(console.size(), 1);
  ASSERT_EQ(console[0], "hello");
}

TEST(SubTree, SubtreeNameNotRegistered)
{
  // clang-format off

  static const char* xml_text = R"(
  <root BTCPP_format="4">
    <BehaviorTree ID="PrintToConsole">\n"
      <Sequence>
        <PrintToConsole message="world"/>
      </Sequence>
    </BehaviorTree>

    <BehaviorTree ID="MainTree">
      <Sequence>
        <PrintToConsole message="hello"/>
        <SubTree ID="PrintToConsole"/>
      </Sequence>
    </BehaviorTree>
  </root>
 )";

  // clang-format on
  BehaviorTreeFactory factory;
  std::vector<std::string> console;
  factory.registerNodeType<PrintToConsole>("PrintToConsole", &console);

  ASSERT_ANY_THROW(auto tree = factory.createTreeFromText(xml_text));
  ASSERT_ANY_THROW(factory.registerBehaviorTreeFromText(xml_text));
}
