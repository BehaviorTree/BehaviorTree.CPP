#include "behaviortree_cpp/xml_parsing.h"

#include <filesystem>
#include <string>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

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
      <Action ID="PassThroughDoor" />
    </Fallback>
  </BehaviorTree>
</root>  )";

static const char* xml_text_subtree_part2 = R"(

<root BTCPP_format="4">
  <BehaviorTree ID="DoorClosedSubtree">
    <Sequence name="door_sequence">
      <Decorator ID="Inverter">
        <Action ID="IsDoorClosed" />
      </Decorator>
      <Action ID="OpenDoor" />
      <Action ID="PassThroughDoor" />
    </Sequence>
  </BehaviorTree>
</root>  )";

// clang-format on

TEST(BehaviorTreeFactory, NotRegisteredNode)
{
  BehaviorTreeFactory factory;
  ASSERT_ANY_THROW(factory.createTreeFromText(xml_text));
  ASSERT_ANY_THROW(std::make_shared<BT::Tree>(factory.createTreeFromText(xml_text)));
}

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
      <Script code=" output:='done!' " />
    </Sequence>
  </BehaviorTree>

  <BehaviorTree ID="MainTree">
    <Sequence>
      <Script code = " talk_hello:='hello' " />
      <Script code = " talk_bye:='bye bye' " />
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

  ASSERT_EQ(main_bb->entryInfo("talk_hello")->type(), typeid(std::string));
  ASSERT_EQ(main_bb->entryInfo("talk_bye")->type(), typeid(std::string));
  ASSERT_EQ(main_bb->entryInfo("talk_out")->type(), typeid(std::string));

  ASSERT_EQ(talk_bb->entryInfo("bye_msg")->type(), typeid(std::string));
  ASSERT_EQ(talk_bb->entryInfo("hello_msg")->type(), typeid(std::string));

  std::cout << "\n --------------------------------- \n" << std::endl;
  main_bb->debugMessage();
  std::cout << "\n ----- \n" << std::endl;
  talk_bb->debugMessage();
  std::cout << "\n --------------------------------- \n" << std::endl;

  ASSERT_EQ(main_bb->entryInfo("talk_hello")->type(), typeid(std::string));
  ASSERT_EQ(main_bb->entryInfo("talk_bye")->type(), typeid(std::string));
  ASSERT_EQ(main_bb->entryInfo("talk_out")->type(), typeid(std::string));

  ASSERT_EQ(talk_bb->entryInfo("bye_msg")->type(), typeid(std::string));
  ASSERT_EQ(talk_bb->entryInfo("hello_msg")->type(), typeid(std::string));
  ASSERT_EQ(talk_bb->entryInfo("output")->type(), typeid(std::string));

  ASSERT_EQ(main_bb->get<std::string>("talk_hello"), "hello");
  ASSERT_EQ(main_bb->get<std::string>("talk_bye"), "bye bye");
  ASSERT_EQ(main_bb->get<std::string>("talk_out"), "done!");

  // these ports should not be present in the subtree TalkToMe
  ASSERT_FALSE(talk_bb->getAnyLocked("talk_hello"));
  ASSERT_FALSE(talk_bb->getAnyLocked("talk_bye"));
  ASSERT_FALSE(talk_bb->getAnyLocked("talk_out"));
}

std::string FilePath(const std::filesystem::path& relative_path)
{
  // clang-format off
  static const std::filesystem::path search_paths[] = {
      BT_TEST_FOLDER,
      std::filesystem::current_path() / "tests"};
  // clang-format on

  for(auto const& path : search_paths)
  {
    if(std::filesystem::exists(path / relative_path))
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

TEST(BehaviorTreeFactory, WrongTreeName)
{
  const char* xmlA = R"(
  <root BTCPP_format="4" >
    <BehaviorTree ID="MainTree">
      <AlwaysSuccess/>
    </BehaviorTree>
  </root> )";

  BehaviorTreeFactory factory;

  factory.registerBehaviorTreeFromText(xmlA);
  EXPECT_ANY_THROW(auto tree = factory.createTree("Wrong Name"));
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

KeyValueVector makeTestMetadata()
{
  return {
    std::make_pair<std::string, std::string>("foo", "hello"),
    std::make_pair<std::string, std::string>("bar", "42"),
  };
}

class ActionWithMetadata : public SyncActionNode
{
public:
  ActionWithMetadata(const std::string& name, const NodeConfig& config)
    : SyncActionNode(name, config)
  {}

  BT::NodeStatus tick() override
  {
    return NodeStatus::SUCCESS;
  }

  static PortsList providedPorts()
  {
    return {};
  }

  static KeyValueVector metadata()
  {
    return makeTestMetadata();
  }
};

TEST(BehaviorTreeFactory, ManifestMethod)
{
  const char* expectedXML = R"(
        <Action ID="ActionWithMetadata">
            <MetadataFields>
                <Metadata foo="hello"/>
                <Metadata bar="42"/>
            </MetadataFields>
        </Action>)";

  BehaviorTreeFactory factory;
  factory.registerNodeType<ActionWithMetadata>("ActionWithMetadata");
  const auto& manifest = factory.manifests().at("ActionWithMetadata");
  EXPECT_EQ(manifest.metadata, makeTestMetadata());

  auto xml = writeTreeNodesModelXML(factory, false);
  std::cout << xml << std::endl;

  EXPECT_NE(xml.find(expectedXML), std::string::npos);
}

TEST(BehaviorTreeFactory, addMetadataToManifest)
{
  BehaviorTreeFactory factory;
  factory.registerNodeType<DummyNodes::SaySomething>("SaySomething");
  const auto& initial_manifest = factory.manifests().at("SaySomething");
  EXPECT_TRUE(initial_manifest.metadata.empty());
  factory.addMetadataToManifest("SaySomething", makeTestMetadata());
  const auto& modified_manifest = factory.manifests().at("SaySomething");
  EXPECT_EQ(modified_manifest.metadata, makeTestMetadata());
}

// Action node used to reproduce issue #1046 (use-after-free on
// manifest pointer). It calls getInput() for a port name that is
// NOT in the XML, so getInputStamped falls through to the
// manifest lookup.
class ActionIssue1046 : public BT::SyncActionNode
{
public:
  ActionIssue1046(const std::string& name, const BT::NodeConfig& config)
    : BT::SyncActionNode(name, config)
  {}

  BT::NodeStatus tick() override
  {
    // "value" is declared in providedPorts() but NOT set in the
    // XML, so getInput() will look it up in the manifest.
    // If the manifest pointer is dangling, this is a
    // use-after-free.
    int value = 0;
    auto res = getInput("value", value);
    // We expect this to fail (no default, not in XML), but it
    // must not crash.
    (void)res;
    return BT::NodeStatus::SUCCESS;
  }

  static BT::PortsList providedPorts()
  {
    return { BT::InputPort<int>("value", "a test port") };
  }
};

// Test for issue #1046: heap use-after-free when
// BehaviorTreeFactory is destroyed before the tree is ticked.
TEST(BehaviorTreeFactory, FactoryDestroyedBeforeTick)
{
  // clang-format off
  // The XML deliberately does NOT set the "value" port so
  // that getInput falls through to the manifest to read
  // the port info.  This triggers the dangling-pointer bug.
  static const char* xml_text_issue_1046 = R"(
  <root BTCPP_format="4" >
    <BehaviorTree ID="Main">
      <ActionIssue1046/>
    </BehaviorTree>
  </root> )";
  // clang-format on

  BT::Tree tree;
  {
    // Factory created in an inner scope
    BT::BehaviorTreeFactory factory;
    factory.registerNodeType<ActionIssue1046>("ActionIssue1046");
    factory.registerBehaviorTreeFromText(xml_text_issue_1046);
    tree = factory.createTree("Main");
    // factory is destroyed here
  }

  // Verify that every node's manifest pointer points into the
  // tree's own copy, not into freed factory memory.
  for(const auto& subtree : tree.subtrees)
  {
    for(const auto& node : subtree->nodes)
    {
      const auto* manifest_ptr =
          static_cast<const BT::TreeNode*>(node.get())->config().manifest;
      if(manifest_ptr)
      {
        auto it = tree.manifests.find(manifest_ptr->registration_ID);
        ASSERT_NE(it, tree.manifests.end());
        EXPECT_EQ(manifest_ptr, &(it->second));
      }
    }
  }

  // Ticking after factory destruction should not crash.
  // Before the fix, NodeConfig::manifest pointed to a manifest
  // owned by the now-destroyed factory, causing use-after-free
  // when getInput() looked up the port in the manifest.
  EXPECT_EQ(BT::NodeStatus::SUCCESS, tree.tickWhileRunning());
}
