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

    for( auto& bb: tree.blackboard_stack)
    {
        bb->debugMessage();
        std::cout << "-----" << std::endl;
    }

    auto ret = tree.tickRoot();

    ASSERT_EQ(ret, NodeStatus::SUCCESS );
    ASSERT_EQ(tree.blackboard_stack.size(), 3 );
}

class CopyPorts : public BT::SyncActionNode
{
public:
  CopyPorts(const std::string& name, const BT::NodeConfiguration& config)
  : BT::SyncActionNode(name, config)
  {
  }

  BT::NodeStatus tick() override
  {
    auto msg = getInput<std::string>("in");
    if (!msg)
    {
      throw BT::RuntimeError( "missing required input [message]: ", msg.error() );
    }
    setOutput("out", msg.value());
    return BT::NodeStatus::SUCCESS;
  }

  static BT::PortsList providedPorts()
  {
    return{ BT::InputPort<std::string>("in"),
            BT::OutputPort<std::string>("out")};
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
  ASSERT_EQ(ret, NodeStatus::SUCCESS );
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
  EXPECT_ANY_THROW( tree_bad_in.tickRoot() );

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
  EXPECT_ANY_THROW( tree_bad_out.tickRoot() );
}

TEST(SubTree, SubtreeWrapper)
{
  BehaviorTreeFactory factory;
  factory.registerNodeType<DummyNodes::SaySomething>("SaySomething");
  factory.registerNodeType<CopyPorts>("CopyPorts");

  static const char* xml_text = R"(
<root main_tree_to_execute = "MainTree" >

    <BehaviorTree ID="MainTree">
        <Sequence>
            <SetBlackboard value="hello" output_key="thoughts" />
            <SubTreeWrapper ID="CopySubtree" />
            <SaySomething  message="{greetings}" />
        </Sequence>
    </BehaviorTree>

    <BehaviorTree ID="CopySubtree">
            <CopyPorts in="{thoughts}" out="{greetings}"/>
    </BehaviorTree>
</root> )";

  Tree tree = factory.createTreeFromText(xml_text);
  auto ret = tree.tickRoot();
  ASSERT_EQ(ret, NodeStatus::SUCCESS );
}
