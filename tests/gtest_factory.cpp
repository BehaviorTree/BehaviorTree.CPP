#include <gtest/gtest.h>
#include "action_test_node.h"
#include "condition_test_node.h"
#include "behaviortree_cpp_v3/xml_parsing.h"
#include "../sample_nodes/crossdoor_nodes.h"
#include "../sample_nodes/dummy_nodes.h"

using namespace BT;

// clang-format off

static const char* xml_text = R"(

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

static const char* xml_text_subtree = R"(

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

// clang-format on

TEST(BehaviorTreeFactory, VerifyLargeTree)
{
    BehaviorTreeFactory factory;
    CrossDoor::RegisterNodes(factory);

    Tree tree = factory.createTreeFromText(xml_text);

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

    Tree tree = factory.createTreeFromText(xml_text_subtree);

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


// clang-format off

static const char* xml_ports_subtree = R"(

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
      <SetBlackboard output_key="talk_hello" value="hello" />
      <SetBlackboard output_key="talk_bye"   value="bye bye" />
      <SubTree ID="TalkToMe" hello_msg="talk_hello"
                             bye_msg="talk_bye"
                             output="talk_out" />
      <SaySomething message="{talk_out}" />
    </Sequence>
  </BehaviorTree>

</root> )";

// clang-format on

TEST(BehaviorTreeFactory, SubTreeWithRemapping)
{
    BehaviorTreeFactory factory;
    factory.registerNodeType<DummyNodes::SaySomething>("SaySomething");

    Tree tree = factory.createTreeFromText(xml_ports_subtree);

    auto main_bb = tree.blackboard_stack.at(0);
    auto talk_bb = tree.blackboard_stack.at(1);

    std::cout << "\n --------------------------------- \n" << std::endl;
    main_bb->debugMessage();
    std::cout << "\n ----- \n" << std::endl;
    talk_bb->debugMessage();
    std::cout << "\n --------------------------------- \n" << std::endl;

    ASSERT_EQ( main_bb->portInfo("talk_hello")->type(), &typeid(std::string) );
    ASSERT_EQ( main_bb->portInfo("talk_bye")->type(),   &typeid(std::string) );
    ASSERT_EQ( main_bb->portInfo("talk_out")->type(),   &typeid(std::string) );

    ASSERT_EQ( talk_bb->portInfo("bye_msg")->type(),   &typeid(std::string) );
    ASSERT_EQ( talk_bb->portInfo("hello_msg")->type(), &typeid(std::string) );

    // Should not throw
    tree.root_node->executeTick();

    std::cout << "\n --------------------------------- \n" << std::endl;
    main_bb->debugMessage();
    std::cout << "\n ----- \n" << std::endl;
    talk_bb->debugMessage();
    std::cout << "\n --------------------------------- \n" << std::endl;

    ASSERT_EQ( main_bb->portInfo("talk_hello")->type(), &typeid(std::string) );
    ASSERT_EQ( main_bb->portInfo("talk_bye")->type(),   &typeid(std::string) );
    ASSERT_EQ( main_bb->portInfo("talk_out")->type(),   &typeid(std::string) );

    ASSERT_EQ( talk_bb->portInfo("bye_msg")->type(),   &typeid(std::string) );
    ASSERT_EQ( talk_bb->portInfo("hello_msg")->type(), &typeid(std::string) );
    ASSERT_EQ( talk_bb->portInfo("output")->type(),    &typeid(std::string) );


    ASSERT_EQ( main_bb->get<std::string>("talk_hello"), "hello");
    ASSERT_EQ( main_bb->get<std::string>("talk_bye"), "bye bye");
    ASSERT_EQ( main_bb->get<std::string>("talk_out"), "done!");

    // these ports should not be present in the subtree TalkToMe
    ASSERT_FALSE( talk_bb->getAny("talk_hello") );
    ASSERT_FALSE( talk_bb->getAny("talk_bye") );
    ASSERT_FALSE( talk_bb->getAny("talk_out") );
}

