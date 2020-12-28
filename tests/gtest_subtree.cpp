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
            <SetBlackboard value="hello" output_key="base_bb_in" />
            <SubTree ID="CopySubtree" child_bb_in="base_bb_in" child_bb_out="base_bb_out"/>
            <SaySomething  message="{base_bb_out}" />
        </Sequence>
    </BehaviorTree>

    <BehaviorTree ID="CopySubtree">
            <CopyPorts in="{child_bb_in}" out="{child_bb_out}"/>
    </BehaviorTree>
</root> )";

  BehaviorTreeFactory factory;
  factory.registerNodeType<DummyNodes::SaySomething>("SaySomething");
  factory.registerNodeType<CopyPorts>("CopyPorts");

  Blackboard::Ptr blackboard = Blackboard::create();
  Tree tree = factory.createTreeFromText(xml_text, blackboard);
  auto ret = tree.tickRoot();
  ASSERT_EQ(ret, NodeStatus::SUCCESS );
  auto out_key_value = blackboard->get<std::string>("base_bb_out");
  ASSERT_EQ("hello", out_key_value);
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

TEST(SubTree, SubtreeSharedBlackboard)
{
    static const char* xml_text = R"(

<root main_tree_to_execute = "MainTree" >

    <BehaviorTree ID="MainTree">
        <Sequence>
            <SetBlackboard value="abc" output_key="shared_bb_in_1" />
            <SetBlackboard value="def" output_key="shared_bb_in_2" />
            <SubTree ID="mySubtree" __shared_blackboard="true"/>
        </Sequence>
    </BehaviorTree>

    <BehaviorTree ID="mySubtree">
        <Sequence>
            <CopyPorts in="{shared_bb_in_1}" out="{shared_bb_out_1}"/>
            <CopyPorts in="{shared_bb_in_2}" out="{shared_bb_out_2}"/>
        </Sequence>
    </BehaviorTree>
</root> )";

    BehaviorTreeFactory factory;
    factory.registerNodeType<CopyPorts>("CopyPorts");

    Blackboard::Ptr blackboard = Blackboard::create();
    Tree tree = factory.createTreeFromText(xml_text, blackboard);
    auto ret = tree.tickRoot();
    ASSERT_EQ(ret, NodeStatus::SUCCESS );

    auto shared_bb_out_1_value = blackboard->get<std::string>("shared_bb_out_1");
    auto shared_bb_out_2_value = blackboard->get<std::string>("shared_bb_out_2");
    ASSERT_EQ("abc", shared_bb_out_1_value);
    ASSERT_EQ("def", shared_bb_out_2_value);
}

TEST(SubTree, SubtreePlusA)
{
    static const char* xml_text = R"(

<root main_tree_to_execute = "MainTree" >

    <BehaviorTree ID="MainTree">
        <Sequence>
            <SetBlackboard value="abc" output_key="abc_base_bb_in" />
            <SubTreePlus ID="subtree_abc" abc_child_bb_in="{abc_base_bb_in}" abc_child_bb_out="{abc_base_bb_out}" />
            <SubTreePlus ID="subtree_def" def_child_bb_in="def" def_child_bb_out="{def_base_bb_out}" />
            <SetBlackboard value="ghi" output_key="ghi_base_bb_in" />
            <SubTreePlus ID="subtree_ghi" __autoremap="1"  />
        </Sequence>
    </BehaviorTree>

    <BehaviorTree ID="subtree_abc">
        <CopyPorts in="{abc_child_bb_in}" out="{abc_child_bb_out}"/>
    </BehaviorTree>

    <BehaviorTree ID="subtree_def">
        <CopyPorts in="{def_child_bb_in}" out="{def_child_bb_out}"/>
    </BehaviorTree>

    <BehaviorTree ID="subtree_ghi">
        <CopyPorts in="{ghi_base_bb_in}" out="{ghi_base_bb_out}"/>
    </BehaviorTree>
</root> )";

    BehaviorTreeFactory factory;
    factory.registerNodeType<CopyPorts>("CopyPorts");

    Blackboard::Ptr blackboard = Blackboard::create();
    Tree tree = factory.createTreeFromText(xml_text, blackboard);
    auto ret = tree.tickRoot();
    ASSERT_EQ(ret, NodeStatus::SUCCESS );
    
    auto abc_base_bb_out_value = blackboard->get<std::string>("abc_base_bb_out");
    auto def_base_bb_out_value = blackboard->get<std::string>("def_base_bb_out");
    auto ghi_base_bb_out_value = blackboard->get<std::string>("ghi_base_bb_out");
    ASSERT_EQ("abc", abc_base_bb_out_value);
    ASSERT_EQ("def", def_base_bb_out_value);
    ASSERT_EQ("ghi", ghi_base_bb_out_value);
}

TEST(SubTree, SubtreePlusB)
{
    static const char* xml_text = R"(

<root main_tree_to_execute = "MainTree" >

    <BehaviorTree ID="MainTree">
        <Sequence>
            <SetBlackboard value="abc" output_key="base_bb_in_1" />
            <SetBlackboard value="ghi" output_key="auto_mapped_in_3" />
            <SubTreePlus ID="mySubtree" __autoremap="1" child_bb_in_1="{base_bb_in_1}" child_bb_in_2="def" />
        </Sequence>
    </BehaviorTree>

    <BehaviorTree ID="mySubtree">
        <Sequence>
            <CopyPorts in="{child_bb_in_1}" out="{auto_mapped_out_1}"/>
            <CopyPorts in="{child_bb_in_2}" out="{auto_mapped_out_2}"/>
            <CopyPorts in="{auto_mapped_in_3}" out="{auto_mapped_out_3}"/>
        </Sequence>
    </BehaviorTree>
</root> )";

    BehaviorTreeFactory factory;
    factory.registerNodeType<CopyPorts>("CopyPorts");

    Blackboard::Ptr blackboard = Blackboard::create();
    Tree tree = factory.createTreeFromText(xml_text, blackboard);
    auto ret = tree.tickRoot();
    ASSERT_EQ(ret, NodeStatus::SUCCESS);

    auto abc_base_bb_out_value = blackboard->get<std::string>("auto_mapped_out_1");
    auto def_base_bb_out_value = blackboard->get<std::string>("auto_mapped_out_2");
    auto ghi_base_bb_out_value = blackboard->get<std::string>("auto_mapped_out_3");
    ASSERT_EQ("abc", abc_base_bb_out_value);
    ASSERT_EQ("def", def_base_bb_out_value);
    ASSERT_EQ("ghi", ghi_base_bb_out_value);
}


class ReadInConstructor : public BT::SyncActionNode
{
  public:
    ReadInConstructor(const std::string& name, const BT::NodeConfiguration& config)
      : BT::SyncActionNode(name, config)
    {
        auto msg = getInput<std::string>("message");
        if (!msg) {
            throw BT::RuntimeError("missing required input [message]: ", msg.error());
        }
    }

    BT::NodeStatus tick() override { return BT::NodeStatus::SUCCESS; }
    static BT::PortsList providedPorts() { return {BT::InputPort<std::string>("message")}; }
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




