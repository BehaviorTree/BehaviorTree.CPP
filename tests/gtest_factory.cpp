#include <gtest/gtest.h>
#include <filesystem>
#include "action_test_node.h"
#include "condition_test_node.h"
#include "behaviortree_cpp/xml_parsing.h"
#include "../sample_nodes/crossdoor_nodes.h"
#include "../sample_nodes/dummy_nodes.h"

using namespace BT;

// clang-format off

static const char* xml_text = R"(

<root BTCPP_format="4" >

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

<root BTCPP_format="4" main_tree_to_execute="MainTree" >

    <BehaviorTree ID="MainTree">
        <Sequence>
            <Fallback>
                <Inverter>
                    <IsDoorClosed/>
                </Inverter>
                <SubTree ID="DoorClosedSubtree"/>
            </Fallback>
            <PassThroughDoor/>
        </Sequence>
    </BehaviorTree>

    <BehaviorTree ID="DoorClosedSubtree">
        <Fallback>
            <OpenDoor/>
            <RetryUntilSuccessful num_attempts="5">
                <PickLock/>
            </RetryUntilSuccessful>
            <SmashDoor/>
        </Fallback>
    </BehaviorTree>

</root>  )";

static const char* xml_text_subtree_part1 = R"(

<root BTCPP_format="4">
  <BehaviorTree ID="MainTree">
    <Fallback name="root_selector">
      <SubTree ID="DoorClosedSubtree" />
      <Action ID="PassThroughWindow" />
    </Fallback>
  </BehaviorTree>
</root>  )";

static const char* xml_text_subtree_part2 = R"(

<root BTCPP_format="4">
  <BehaviorTree ID="DoorClosedSubtree">
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
  CrossDoor cross_door;
  cross_door.registerNodes(factory);

  {
    XMLParser parser(factory);
    parser.loadFromText(xml_text_subtree);
    auto trees = parser.registeredBehaviorTrees();
    ASSERT_EQ(trees[0], "DoorClosedSubtree");
    ASSERT_EQ(trees[1], "MainTree");
  }
  {
    XMLParser parser(factory);
    parser.loadFromText(xml_text_subtree_part1);
    parser.loadFromText(xml_text_subtree_part2);
    auto trees = parser.registeredBehaviorTrees();
    ASSERT_EQ(trees[0], "DoorClosedSubtree");
    ASSERT_EQ(trees[1], "MainTree");
  }
  {
    XMLParser parser(factory);
    parser.loadFromText(xml_text_subtree_part2);
    parser.loadFromText(xml_text_subtree_part1);
    auto trees = parser.registeredBehaviorTrees();
    ASSERT_EQ(trees[0], "DoorClosedSubtree");
    ASSERT_EQ(trees[1], "MainTree");
  }
}

TEST(BehaviorTreeFactory, Subtree)
{
  BehaviorTreeFactory factory;
  CrossDoor cross_door;
  cross_door.registerNodes(factory);

  Tree tree = factory.createTreeFromText(xml_text_subtree);

  printTreeRecursively(tree.rootNode());

  ASSERT_EQ(tree.subtrees.size(), 2);

  auto const& main_tree = tree.subtrees[0];
  auto const& subtree = tree.subtrees[1];

  ASSERT_EQ(main_tree->nodes.size(), 6);
  ASSERT_EQ(main_tree->nodes[0]->name(), "Sequence");
  ASSERT_EQ(main_tree->nodes[1]->name(), "Fallback");
  ASSERT_EQ(main_tree->nodes[2]->name(), "Inverter");
  ASSERT_EQ(main_tree->nodes[3]->name(), "IsDoorClosed");
  ASSERT_EQ(main_tree->nodes[4]->type(), NodeType::SUBTREE);
  ASSERT_EQ(main_tree->nodes[5]->name(), "PassThroughDoor");

  ASSERT_EQ(subtree->nodes.size(), 5);
  ASSERT_EQ(subtree->nodes[0]->name(), "Fallback");
  ASSERT_EQ(subtree->nodes[1]->name(), "OpenDoor");
  ASSERT_EQ(subtree->nodes[2]->name(), "RetryUntilSuccessful");
  ASSERT_EQ(subtree->nodes[3]->name(), "PickLock");
  ASSERT_EQ(subtree->nodes[4]->name(), "SmashDoor");
}

TEST(BehaviorTreeFactory, Issue7)
{
  const std::string xml_text_issue = R"(
<root BTCPP_format="4">
    <BehaviorTree ID="ReceiveGuest">
    </BehaviorTree>
</root> )";

  BehaviorTreeFactory factory;
  XMLParser parser(factory);

  EXPECT_THROW(parser.loadFromText(xml_text_issue), RuntimeError);
}

// clang-format off

static const char* xml_ports_subtree = R"(

<root BTCPP_format="4" main_tree_to_execute="MainTree">

  <BehaviorTree ID="TalkToMe">
    <Sequence>
      <SaySomething message="{hello_msg}" />
      <SaySomething message="{bye_msg}" />
      <Script code=" output='done!' " />
    </Sequence>
  </BehaviorTree>

  <BehaviorTree ID="MainTree">
    <Sequence>
      <Script code = " talk_hello='hello' " />
      <Script code = " talk_bye='bye bye' " />
      <SubTree ID="TalkToMe" hello_msg="{talk_hello}"
                             bye_msg="{talk_bye}"
                             output="{talk_out}" />
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

  auto main_bb = tree.subtrees.at(0)->blackboard;
  auto talk_bb = tree.subtrees.at(1)->blackboard;

  std::cout << "\n --------------------------------- \n" << std::endl;
  main_bb->debugMessage();
  std::cout << "\n ----- \n" << std::endl;
  talk_bb->debugMessage();
  std::cout << "\n --------------------------------- \n" << std::endl;

  // Should not throw
  tree.tickWhileRunning();

  ASSERT_EQ(main_bb->portInfo("talk_hello")->type(), typeid(std::string));
  ASSERT_EQ(main_bb->portInfo("talk_bye")->type(), typeid(std::string));
  ASSERT_EQ(main_bb->portInfo("talk_out")->type(), typeid(std::string));

  ASSERT_EQ(talk_bb->portInfo("bye_msg")->type(), typeid(std::string));
  ASSERT_EQ(talk_bb->portInfo("hello_msg")->type(), typeid(std::string));

  std::cout << "\n --------------------------------- \n" << std::endl;
  main_bb->debugMessage();
  std::cout << "\n ----- \n" << std::endl;
  talk_bb->debugMessage();
  std::cout << "\n --------------------------------- \n" << std::endl;

  ASSERT_EQ(main_bb->portInfo("talk_hello")->type(), typeid(std::string));
  ASSERT_EQ(main_bb->portInfo("talk_bye")->type(), typeid(std::string));
  ASSERT_EQ(main_bb->portInfo("talk_out")->type(), typeid(std::string));

  ASSERT_EQ(talk_bb->portInfo("bye_msg")->type(), typeid(std::string));
  ASSERT_EQ(talk_bb->portInfo("hello_msg")->type(), typeid(std::string));
  ASSERT_EQ(talk_bb->portInfo("output")->type(), typeid(std::string));

  ASSERT_EQ(main_bb->get<std::string>("talk_hello"), "hello");
  ASSERT_EQ(main_bb->get<std::string>("talk_bye"), "bye bye");
  ASSERT_EQ(main_bb->get<std::string>("talk_out"), "done!");

  // these ports should not be present in the subtree TalkToMe
  ASSERT_FALSE(talk_bb->getAny("talk_hello"));
  ASSERT_FALSE(talk_bb->getAny("talk_bye"));
  ASSERT_FALSE(talk_bb->getAny("talk_out"));
}

std::string FilePath(const std::filesystem::path& relative_path)
{
  // clang-format off
  static const std::filesystem::path search_paths[] = {
      BT_TEST_FOLDER,
      std::filesystem::current_path() / "tests"};
  // clang-format on

  for (auto const& path : search_paths)
  {
    if (std::filesystem::exists(path / relative_path))
    {
      return (path / relative_path).string();
    }
  }
  return {};
}

TEST(BehaviorTreeFactory, CreateTreeFromFile)
{
  BehaviorTreeFactory factory;

  // should not throw
  std::string path = FilePath("trees/parent_no_include.xml");
  Tree tree = factory.createTreeFromFile(path);
  ASSERT_EQ(NodeStatus::SUCCESS, tree.tickWhileRunning());
}

TEST(BehaviorTreeFactory, CreateTreeFromFileWhichIncludesFileFromSameDirectory)
{
  BehaviorTreeFactory factory;

  // should not throw
  std::string path = FilePath("trees/child/child_include_sibling.xml");
  Tree tree = factory.createTreeFromFile(path);
  ASSERT_EQ(NodeStatus::SUCCESS, tree.tickWhileRunning());
}

TEST(BehaviorTreeFactory, CreateTreeFromFileWhichIncludesFileFromChildDirectory)
{
  BehaviorTreeFactory factory;

  // should not throw
  std::string path = FilePath("trees/parent_include_child.xml");
  Tree tree = factory.createTreeFromFile(path);
  ASSERT_EQ(NodeStatus::SUCCESS, tree.tickWhileRunning());
}

TEST(
    BehaviorTreeFactory,
    CreateTreeFromFileWhichIncludesFileFromChildDirectoryWhichIncludesFileFromSameDirectory)
{
  BehaviorTreeFactory factory;

  // should not throw
  std::string path = FilePath("trees/parent_include_child_include_sibling.xml");
  Tree tree = factory.createTreeFromFile(path);
  ASSERT_EQ(NodeStatus::SUCCESS, tree.tickWhileRunning());
}

TEST(
    BehaviorTreeFactory,
    CreateTreeFromFileWhichIncludesFileFromChildDirectoryWhichIncludesFileFromChildDirectory)
{
  BehaviorTreeFactory factory;

  // should not throw
  std::string path = FilePath("trees/parent_include_child_include_child.xml");
  Tree tree = factory.createTreeFromFile(path);
  ASSERT_EQ(NodeStatus::SUCCESS, tree.tickWhileRunning());
}

TEST(
    BehaviorTreeFactory,
    CreateTreeFromFileWhichIncludesFileFromChildDirectoryWhichIncludesFileFromParentDirectory)
{
  BehaviorTreeFactory factory;

  // should not throw
  std::string path = FilePath("trees/parent_include_child_include_parent.xml");
  Tree tree = factory.createTreeFromFile(path);
  ASSERT_EQ(NodeStatus::SUCCESS, tree.tickWhileRunning());
}

TEST(BehaviorTreeReload, ReloadSameTree)
{
  const char* xmlA = R"(
<root BTCPP_format="4" >
  <BehaviorTree ID="MainTree">
    <AlwaysSuccess/>
  </BehaviorTree>
</root> )";

  const char* xmlB = R"(
<root BTCPP_format="4" >
  <BehaviorTree ID="MainTree">
    <AlwaysFailure/>
  </BehaviorTree>
</root> )";

  BehaviorTreeFactory factory;

  factory.registerBehaviorTreeFromText(xmlA);
  {
    auto tree = factory.createTree("MainTree");
    ASSERT_EQ(NodeStatus::SUCCESS, tree.tickWhileRunning());
  }

  factory.registerBehaviorTreeFromText(xmlB);
  {
    auto tree = factory.createTree("MainTree");
    ASSERT_EQ(NodeStatus::FAILURE, tree.tickWhileRunning());
  }
}
