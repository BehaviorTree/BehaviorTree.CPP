#include <gtest/gtest.h>
#include "action_test_node.h"
#include "condition_test_node.h"
#include "behaviortree_cpp_v3/xml_parsing.h"
#include "environment.h"
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

static const char* xml_text_subtree_part1 = R"(

<root>
  <BehaviorTree ID="MainTree">
    <Fallback name="root_selector">
      <SubTree ID="CrossDoorSubtree" />
      <Action ID="PassThroughWindow" />
    </Fallback>
  </BehaviorTree>
</root>  )";

static const char* xml_text_subtree_part2 = R"(

<root>
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
</root>  )";

// clang-format on

TEST(BehaviorTreeFactory, XMLParsingOrder)
{
  BehaviorTreeFactory factory;
  CrossDoor::RegisterNodes(factory);

  {
    XMLParser parser(factory);
    parser.loadFromText(xml_text_subtree);
    auto trees = parser.registeredBehaviorTrees();
    ASSERT_EQ(trees.size(), 2);
    ASSERT_EQ(trees[0], "CrossDoorSubtree");
    ASSERT_EQ(trees[1], "MainTree");
  }
  {
    XMLParser parser(factory);
    parser.loadFromText(xml_text_subtree_part1);
    parser.loadFromText(xml_text_subtree_part2);
    auto trees = parser.registeredBehaviorTrees();
    ASSERT_EQ(trees.size(), 2);
    ASSERT_EQ(trees[0], "CrossDoorSubtree");
    ASSERT_EQ(trees[1], "MainTree");
  }
  {
    XMLParser parser(factory);
    parser.loadFromText(xml_text_subtree_part2);
    parser.loadFromText(xml_text_subtree_part1);
    auto trees = parser.registeredBehaviorTrees();
    ASSERT_EQ(trees.size(), 2);
    ASSERT_EQ(trees[0], "CrossDoorSubtree");
    ASSERT_EQ(trees[1], "MainTree");
  }
}

TEST(BehaviorTreeFactory, VerifyLargeTree)
{
  BehaviorTreeFactory factory;
  CrossDoor::RegisterNodes(factory);

  Tree tree = factory.createTreeFromText(xml_text);

  printTreeRecursively(tree.rootNode());

  ASSERT_EQ(tree.rootNode()->name(), "root_selector");

  auto fallback = dynamic_cast<const FallbackNode*>(tree.rootNode());
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

  printTreeRecursively(tree.rootNode());

  ASSERT_EQ(tree.rootNode()->name(), "root_selector");

  auto root_selector = dynamic_cast<const FallbackNode*>(tree.rootNode());
  ASSERT_TRUE(root_selector != nullptr);
  ASSERT_EQ(root_selector->children().size(), 2);
  ASSERT_EQ(root_selector->child(0)->name(), "CrossDoorSubtree");
  ASSERT_EQ(root_selector->child(1)->name(), "PassThroughWindow");

  auto subtree = dynamic_cast<const SubtreeNode*>(root_selector->child(0));
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

  // We expect that an incorrectly-constructed behavior tree will fail to load
  EXPECT_THROW(parser.loadFromText(xml_text_issue), RuntimeError);

  // We expect that no behavior trees will be registered after we unsuccessfully attempt to load a single tree
  auto trees = parser.registeredBehaviorTrees();
  EXPECT_TRUE( trees.empty() );
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

  ASSERT_EQ(main_bb->portInfo("talk_hello")->type(), &typeid(std::string));
  ASSERT_EQ(main_bb->portInfo("talk_bye")->type(), &typeid(std::string));
  ASSERT_EQ(main_bb->portInfo("talk_out")->type(), &typeid(std::string));

  ASSERT_EQ(talk_bb->portInfo("bye_msg")->type(), &typeid(std::string));
  ASSERT_EQ(talk_bb->portInfo("hello_msg")->type(), &typeid(std::string));

  // Should not throw
  tree.tickRoot();

  std::cout << "\n --------------------------------- \n" << std::endl;
  main_bb->debugMessage();
  std::cout << "\n ----- \n" << std::endl;
  talk_bb->debugMessage();
  std::cout << "\n --------------------------------- \n" << std::endl;

  ASSERT_EQ(main_bb->portInfo("talk_hello")->type(), &typeid(std::string));
  ASSERT_EQ(main_bb->portInfo("talk_bye")->type(), &typeid(std::string));
  ASSERT_EQ(main_bb->portInfo("talk_out")->type(), &typeid(std::string));

  ASSERT_EQ(talk_bb->portInfo("bye_msg")->type(), &typeid(std::string));
  ASSERT_EQ(talk_bb->portInfo("hello_msg")->type(), &typeid(std::string));
  ASSERT_EQ(talk_bb->portInfo("output")->type(), &typeid(std::string));

  ASSERT_EQ(main_bb->get<std::string>("talk_hello"), "hello");
  ASSERT_EQ(main_bb->get<std::string>("talk_bye"), "bye bye");
  ASSERT_EQ(main_bb->get<std::string>("talk_out"), "done!");

  // these ports should not be present in the subtree TalkToMe
  ASSERT_FALSE(talk_bb->getAny("talk_hello"));
  ASSERT_FALSE(talk_bb->getAny("talk_bye"));
  ASSERT_FALSE(talk_bb->getAny("talk_out"));
}

#if !defined(USING_ROS) && !defined(USING_ROS2)
TEST(BehaviorTreeFactory, CreateTreeFromFile)
{
  BehaviorTreeFactory factory;

  // should not throw
  auto path = (environment->executable_path.parent_path() / "trees/"
                                                            "parent_no_include.xml");
  Tree tree = factory.createTreeFromFile(path.str());
  ASSERT_EQ(NodeStatus::SUCCESS, tree.tickRoot());
}

TEST(BehaviorTreeFactory, CreateTreeFromFileWhichIncludesFileFromSameDirectory)
{
  BehaviorTreeFactory factory;

  // should not throw
  auto path = (environment->executable_path.parent_path() / "trees/child/"
                                                            "child_include_sibling.xml");
  Tree tree = factory.createTreeFromFile(path.str());
  ASSERT_EQ(NodeStatus::SUCCESS, tree.tickRoot());
}

TEST(BehaviorTreeFactory, CreateTreeFromFileWhichIncludesFileFromChildDirectory)
{
  BehaviorTreeFactory factory;

  // should not throw
  auto path = (environment->executable_path.parent_path() / "trees/"
                                                            "parent_include_child.xml");
  Tree tree = factory.createTreeFromFile(path.str());
  ASSERT_EQ(NodeStatus::SUCCESS, tree.tickRoot());
}

TEST(
    BehaviorTreeFactory,
    CreateTreeFromFileWhichIncludesFileFromChildDirectoryWhichIncludesFileFromSameDirectory)
{
  BehaviorTreeFactory factory;

  // should not throw
  auto path = (environment->executable_path.parent_path() / "trees/"
                                                            "parent_include_child_"
                                                            "include_sibling.xml");
  Tree tree = factory.createTreeFromFile(path.str());
  ASSERT_EQ(NodeStatus::SUCCESS, tree.tickRoot());
}

TEST(
    BehaviorTreeFactory,
    CreateTreeFromFileWhichIncludesFileFromChildDirectoryWhichIncludesFileFromChildDirectory)
{
  BehaviorTreeFactory factory;

  // should not throw
  auto path = (environment->executable_path.parent_path() / "trees/"
                                                            "parent_include_child_"
                                                            "include_child.xml");
  Tree tree = factory.createTreeFromFile(path.str());
  ASSERT_EQ(NodeStatus::SUCCESS, tree.tickRoot());
}

TEST(
    BehaviorTreeFactory,
    CreateTreeFromFileWhichIncludesFileFromChildDirectoryWhichIncludesFileFromParentDirectory)
{
  BehaviorTreeFactory factory;

  // should not throw
  auto path = (environment->executable_path.parent_path() / "trees/"
                                                            "parent_include_child_"
                                                            "include_parent.xml");
  Tree tree = factory.createTreeFromFile(path.str());
  ASSERT_EQ(NodeStatus::SUCCESS, tree.tickRoot());
}
#endif

TEST(BehaviorTreeFactory, DecoratorWithoutChildThrows)
{
  BehaviorTreeFactory factory;
  const std::string tree_xml = R"(
<root>
    <BehaviorTree ID="Main">
        <ForceSuccess>
        </ForceSuccess>
    </BehaviorTree>
</root>
)";

  ASSERT_THROW(factory.createTreeFromText(tree_xml), BehaviorTreeException);
}

TEST(BehaviorTreeFactory, DecoratorWithTwoChildrenThrows)
{
  BehaviorTreeFactory factory;
  const std::string tree_xml = R"(
<root>
    <BehaviorTree ID="Main">
        <ForceSuccess>
          <AlwaysSuccess />
          <AlwaysSuccess />
        </ForceSuccess>
    </BehaviorTree>
</root>
)";

  ASSERT_THROW(factory.createTreeFromText(xml_text), BehaviorTreeException);
}


TEST(BehaviorTreeFactory, RegisterValidAndInvalidTrees)
{
const std::string xml_text_ok = R"(
<root>
    <BehaviorTree ID="ValidTree">
      <Sequence name="door_open_sequence">
        <Action ID="AlwaysSuccess" />
      </Sequence>
    </BehaviorTree>
</root> )";

const std::string xml_text_invalid = R"(
<root>
    <BehaviorTree ID="InvalidTreeWithNoChildren">
    </BehaviorTree>
</root> )";

    BehaviorTreeFactory factory;
    XMLParser parser(factory);

    // GIVEN that a valid tree has been loaded
    ASSERT_NO_THROW(parser.loadFromText(xml_text_ok));

    // WHEN we attempt to load an invalid tree
    ASSERT_THROW(parser.loadFromText(xml_text_invalid), RuntimeError);

    // THEN the valid tree is still registered
    auto trees = parser.registeredBehaviorTrees();
    ASSERT_EQ(trees.size(), 1);
    EXPECT_EQ(trees[0], "ValidTree");
}

TEST(BehaviorTreeFactory, RegisterInvalidXMLBadActionNodeThrows)
{
  // GIVEN an invalid tree
  // This tree contains invalid XML because the action node is missing a trailing `/`.
  // A valid line would read: <Action ID="AlwaysSuccess" />
const std::string xml_text_invalid = R"(
<root>
    <BehaviorTree ID="InvalidTreeWithBadChild">
      <Sequence name="seq">
        <Action ID="AlwaysSuccess" >
      </Sequence>
    </BehaviorTree>
</root> )";

    BehaviorTreeFactory factory;
    XMLParser parser(factory);

    // WHEN we attempt to load an invalid tree
    // THEN a RuntimeError exception is thrown
    EXPECT_THROW(parser.loadFromText(xml_text_invalid), RuntimeError);

    // THEN no tree is registered
    auto trees = parser.registeredBehaviorTrees();
    EXPECT_TRUE(trees.empty());
}

TEST(BehaviorTreeFactory, RegisterInvalidXMLNoRootThrows)
{
  // GIVEN an invalid tree
  // This tree contains invalid XML because it does not have a root node
const std::string xml_text_invalid = R"(
    <BehaviorTree ID="InvalidTreeNoRoot">
      <Sequence name="seq">
        <Action ID="AlwaysSuccess" />
      </Sequence>
    </BehaviorTree> )";

    BehaviorTreeFactory factory;
    XMLParser parser(factory);

    // WHEN we attempt to load an invalid tree
    // THEN a RuntimeError exception is thrown
    EXPECT_THROW(parser.loadFromText(xml_text_invalid), RuntimeError);

    // THEN no tree is registered
    auto trees = parser.registeredBehaviorTrees();
    EXPECT_TRUE(trees.empty());
}

TEST(BehaviorTreeFactory, ParserClearRegisteredBehaviorTrees)
{
  const std::string tree_xml = R"(
<root>
    <BehaviorTree ID="Main">
        <AlwaysSuccess />
    </BehaviorTree>
</root>
)";

  BehaviorTreeFactory factory;
  XMLParser parser(factory);

  ASSERT_NO_THROW(parser.loadFromText(tree_xml));

  const auto trees = parser.registeredBehaviorTrees();
  ASSERT_FALSE(trees.empty());

  parser.clearInternalState();

  const auto trees_after_clear = parser.registeredBehaviorTrees();
  EXPECT_TRUE(trees_after_clear.empty());
}

TEST(BehaviorTreeFactory, FactoryClearRegisteredBehaviorTrees)
{
  BehaviorTreeFactory factory;
  const std::string tree_xml = R"(
<root>
    <BehaviorTree ID="Main">
        <AlwaysSuccess />
    </BehaviorTree>
</root>
)";

  ASSERT_NO_THROW(factory.registerBehaviorTreeFromText(tree_xml));

  const auto trees = factory.registeredBehaviorTrees();
  ASSERT_FALSE(trees.empty());

  factory.clearRegisteredBehaviorTrees();

  const auto trees_after_clear = factory.registeredBehaviorTrees();
  EXPECT_TRUE(trees_after_clear.empty());
}
