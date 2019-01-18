#include <gtest/gtest.h>
#include "action_test_node.h"
#include "condition_test_node.h"
#include "behaviortree_cpp/xml_parsing.h"
#include "../sample_nodes/crossdoor_nodes.h"
#include "../sample_nodes/dummy_nodes.h"

using namespace BT;

// clang-format off

const std::string xml_text = R"(

<root main_tree_to_execute = "MainTree" >

    <BehaviorTree ID="MainTree">
        <Fallback name="root_selector">

            <Sequence name="door_open_sequence">
                <Action ID="IsDoorOpen" />
                <Action ID="PassThroughDoor" />
            </Sequence>

            <Sequence name="door_closed_sequence">
                <Decorator ID="Inverter">
                     <Action ID="IsDoorOpen" />
                </Decorator>
                <Action ID="OpenDoor" />
                <Action ID="PassThroughDoor" />
                <Action ID="CloseDoor" />
            </Sequence>

            <Action ID="PassThroughWindow" />

        </Fallback>
    </BehaviorTree>

    <!-- TreeNodesModel is used only by the Graphic interface -->
    <TreeNodesModel>
        <Action ID="IsDoorOpen" />
        <Action ID="PassThroughDoor" />
        <Action ID="CloseDoor" />
        <Action ID="OpenDoor" />
        <Action ID="PassThroughWindow" />
    </TreeNodesModel>
</root>
        )";

const std::string xml_text_subtree = R"(

<root main_tree_to_execute = "MainTree" >

  <BehaviorTree ID="CrossDoorSubtree">
    <Sequence name="door_sequence">
      <Decorator ID="Inverter">
        <Action ID="IsDoorLocked" />
      </Decorator>
      <Action ID="OpenDoor" />
      <Action ID="PassThroughDoor" />
      <Action ID="CloseDoor" />
    </Sequence>
  </BehaviorTree>

  <!-- This tree will include the other one -->
  <BehaviorTree ID="MainTree">
    <Fallback name="root_selector">
      <SubTree ID="CrossDoorSubtree" />
      <Action ID="PassThroughWindow" />
    </Fallback>
  </BehaviorTree>

</root>  )";

const std::string xml_ports_subtree = R"(

<root main_tree_to_execute = "MainTree" >

  <BehaviorTree ID="TalkToMe">
    <Sequence>
      <SaySomething message="{hello_msg}" />
      <SaySomething message="{bye_msg}" />
      <SetBlackboard output_key="output" value="done!" />
    </Sequence>
  </BehaviorTree>

  <BehaviorTree ID="MainTree">
    <Sequence>
      <SetBlackboard output_key="talk__hello" value="hello" />
      <SetBlackboard output_key="talk__bye"   value="bye bye" />
      <SubTree ID="TalkToMe">
            <remap external="talk_hello" internal="hello_msg" />
            <remap external="talk_bye"   internal="bye_msg" />
            <remap external="talk_out"   internal="output" />
      </SubTree>
      <SaySomething message="{talk_out}" />
    </Sequence>
  </BehaviorTree>

</root> )";


// clang-format on

TEST(BehaviorTreeFactory, VerifyLargeTree)
{
    BehaviorTreeFactory factory;
    CrossDoor::RegisterNodes(factory);

    Tree tree = buildTreeFromText(factory, xml_text);

    printTreeRecursively(tree.root_node);

    ASSERT_EQ(tree.root_node->name(), "root_selector");

    auto fallback = dynamic_cast<const FallbackNode*>(tree.root_node);
    ASSERT_TRUE(fallback != nullptr);

    ASSERT_EQ(fallback->children().size(), 3);
    ASSERT_EQ(fallback->child(0)->name(), "door_open_sequence");
    ASSERT_EQ(fallback->child(1)->name(), "door_closed_sequence");
    ASSERT_EQ(fallback->child(2)->name(), "PassThroughWindow");

    auto sequence_open = dynamic_cast<const SequenceNode*>(fallback->child(0));
    ASSERT_TRUE(sequence_open != nullptr);

    ASSERT_EQ(sequence_open->children().size(), 2);
    ASSERT_EQ(sequence_open->child(0)->name(), "IsDoorOpen");
    ASSERT_EQ(sequence_open->child(1)->name(), "PassThroughDoor");

    auto sequence_closed = dynamic_cast<const SequenceNode*>(fallback->child(1));
    ASSERT_TRUE(sequence_closed != nullptr);

    ASSERT_EQ(sequence_closed->children().size(), 4);
    ASSERT_EQ(sequence_closed->child(0)->name(), "Inverter");
    ASSERT_EQ(sequence_closed->child(1)->name(), "OpenDoor");
    ASSERT_EQ(sequence_closed->child(2)->name(), "PassThroughDoor");
    ASSERT_EQ(sequence_closed->child(3)->name(), "CloseDoor");

    auto decorator = dynamic_cast<const InverterNode*>(sequence_closed->child(0));
    ASSERT_TRUE(decorator != nullptr);

    ASSERT_EQ(decorator->child()->name(), "IsDoorOpen");
}

TEST(BehaviorTreeFactory, Subtree)
{
    BehaviorTreeFactory factory;
    CrossDoor::RegisterNodes(factory);

    Tree tree = buildTreeFromText(factory, xml_text_subtree);

    printTreeRecursively(tree.root_node);

    ASSERT_EQ(tree.root_node->name(), "root_selector");

    auto root_selector = dynamic_cast<const FallbackNode*>(tree.root_node);
    ASSERT_TRUE(root_selector != nullptr);
    ASSERT_EQ(root_selector->children().size(), 2);
    ASSERT_EQ(root_selector->child(0)->name(), "CrossDoorSubtree");
    ASSERT_EQ(root_selector->child(1)->name(), "PassThroughWindow");

    auto subtree = dynamic_cast<const DecoratorSubtreeNode*>(root_selector->child(0));
    ASSERT_TRUE(subtree != nullptr);

    auto sequence = dynamic_cast<const SequenceNode*>(subtree->child());
    ASSERT_TRUE(sequence != nullptr);

    ASSERT_EQ(sequence->children().size(), 4);
    ASSERT_EQ(sequence->child(0)->name(), "Inverter");
    ASSERT_EQ(sequence->child(1)->name(), "OpenDoor");
    ASSERT_EQ(sequence->child(2)->name(), "PassThroughDoor");
    ASSERT_EQ(sequence->child(3)->name(), "CloseDoor");

    auto decorator = dynamic_cast<const InverterNode*>(sequence->child(0));
    ASSERT_TRUE(decorator != nullptr);

    ASSERT_EQ(decorator->child()->name(), "IsDoorLocked");
}

TEST(BehaviorTreeFactory, Issue7)
{
const std::string xml_text_issue = R"(
<root>
    <BehaviorTree ID="ReceiveGuest">
    </BehaviorTree>
</root> )";

    BehaviorTreeFactory factory;
    XMLParser parser(factory);

    EXPECT_THROW( parser.loadFromText(xml_text_issue), RuntimeError );
}



TEST(BehaviorTreeFactory, SubTreeWithRemapping)
{
    BehaviorTreeFactory factory;
    factory.registerNodeType<DummyNodes::SaySomething>("SaySomething");

    Tree tree = buildTreeFromText(factory, xml_ports_subtree);

    tree.root_node->executeTick();
}

