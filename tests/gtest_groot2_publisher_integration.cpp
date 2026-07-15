#include "groot2_test_utils.hpp"

#include <atomic>
#include <cstdlib>
#include <stdexcept>
#include <string>

#include <behaviortree_cpp/bt_factory.h>
#include <gtest/gtest.h>

#include <behaviortree_cpp/contrib/json.hpp>

namespace
{
constexpr auto kHookTreeXml = R"(
<root BTCPP_format="4">
  <BehaviorTree ID="MainTree">
    <CountAction/>
  </BehaviorTree>
</root>
)";

void malformedRequestScenario()
{
  BT::BehaviorTreeFactory factory;
  auto tree = factory.createTreeFromText(R"(
<root BTCPP_format="4">
  <BehaviorTree ID="MainTree">
    <AlwaysSuccess/>
  </BehaviorTree>
</root>
)");
  auto [publisher, port] = Groot2Test::makePublisher(tree);
  Groot2Test::Client client(port);

  auto error_reply = client.rawRequest(BT::Monitor::RequestType::HOOK_INSERT, "{");
  if(error_reply.size() != 2 || error_reply[0].to_string() != "error")
  {
    std::exit(EXIT_FAILURE);
  }

  // The server thread must remain usable after rejecting the bad request.
  auto valid_reply = client.request(BT::Monitor::RequestType::FULLTREE);
  if(valid_reply.size() != 2)
  {
    std::exit(EXIT_FAILURE);
  }
  std::exit(EXIT_SUCCESS);
}

BT::Tree createTreeWithNamedSubtrees(BT::BehaviorTreeFactory& factory,
                                     const BT::Blackboard::Ptr& main_blackboard)
{
  const std::string xml_text = R"(
    <root BTCPP_format="4" main_tree_to_execute="MainTree">
       <BehaviorTree ID="MainTree">
          <Sequence>
            <AlwaysSuccess name="MainAction"/>
            <SubTree ID="ChildA" name="ChildA"/>
            <SubTree ID="ChildB" name="ChildB"/>
          </Sequence>
       </BehaviorTree>

       <BehaviorTree ID="ChildA">
          <AlwaysSuccess name="ChildActionA"/>
       </BehaviorTree>

       <BehaviorTree ID="ChildB">
          <AlwaysSuccess name="ChildActionB"/>
       </BehaviorTree>
    </root>)";

  return factory.createTreeFromText(xml_text, main_blackboard);
}

nlohmann::json requestBlackboardDump(const BT::Tree& tree, const std::string& bb_list)
{
  auto [publisher, port] = Groot2Test::makePublisher(tree);
  Groot2Test::Client client(port);
  auto reply = client.request(BT::Monitor::RequestType::BLACKBOARD, bb_list);
  if(reply.size() != 2u)
  {
    throw std::runtime_error("Unexpected Groot2 blackboard reply size");
  }
  return nlohmann::json::from_msgpack(reply[1].to_string());
}

std::string requestBlackboardDumpError(const BT::Tree& tree, const std::string& bb_list)
{
  auto [publisher, port] = Groot2Test::makePublisher(tree);
  Groot2Test::Client client(port);
  auto reply = client.rawRequest(BT::Monitor::RequestType::BLACKBOARD, bb_list);
  if(reply.size() != 2u || reply[0].to_string() != "error")
  {
    throw std::runtime_error("Expected a Groot2 blackboard error reply");
  }
  return reply[1].to_string();
}
}  // namespace

TEST(Groot2PublisherIntegration, PostHookRunsAfterNodeAndCanBeRemoved)
{
  std::atomic_int tick_count = 0;
  BT::BehaviorTreeFactory factory;
  factory.registerSimpleAction("CountAction", [&](BT::TreeNode&) {
    ++tick_count;
    return BT::NodeStatus::SUCCESS;
  });
  auto tree = factory.createTreeFromText(kHookTreeXml);
  auto [publisher, port] = Groot2Test::makePublisher(tree);
  Groot2Test::Client client(port);
  const auto uid = tree.rootNode()->UID();

  const auto hook = Groot2Test::makeHook(uid, BT::Monitor::Hook::Mode::REPLACE,
                                         BT::Monitor::Hook::Position::POST);
  client.request(BT::Monitor::RequestType::HOOK_INSERT, hook.dump());

  auto dump_reply = client.request(BT::Monitor::RequestType::HOOKS_DUMP);
  ASSERT_EQ(dump_reply.size(), 2);
  const auto hooks = nlohmann::json::parse(dump_reply[1].to_string());
  ASSERT_EQ(hooks.size(), 1);
  EXPECT_EQ(hooks[0].at("position"), int(BT::Monitor::Hook::Position::POST));

  EXPECT_EQ(tree.tickExactlyOnce(), BT::NodeStatus::FAILURE);
  EXPECT_EQ(tick_count, 1);

  const auto remove =
      nlohmann::json{ { "uid", uid },
                      { "position", int(BT::Monitor::Hook::Position::POST) } };
  client.request(BT::Monitor::RequestType::HOOK_REMOVE, remove.dump());

  EXPECT_EQ(tree.tickExactlyOnce(), BT::NodeStatus::SUCCESS);
  EXPECT_EQ(tick_count, 2);
}

TEST(Groot2PublisherIntegration, MalformedRequestDoesNotTerminateServer)
{
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";
  EXPECT_EXIT(malformedRequestScenario(), ::testing::ExitedWithCode(EXIT_SUCCESS), ".*");
}

TEST(Groot2PublisherIntegration, BlackboardDump_NoRootWithoutExternalBlackboard)
{
  BT::BehaviorTreeFactory factory;
  factory.registerBehaviorTreeFromText(R"(
    <root BTCPP_format="4">
      <BehaviorTree ID="MainTree">
        <AlwaysSuccess/>
      </BehaviorTree>
    </root>)");

  auto main_blackboard = BT::Blackboard::create();
  main_blackboard->set("local_value", 7);
  auto tree = factory.createTree("MainTree", main_blackboard);

  auto json = requestBlackboardDump(tree, "MainTree");
  ASSERT_TRUE(json.contains("MainTree"));
  EXPECT_FALSE(json.contains("ROOT"));

  ASSERT_TRUE(json["MainTree"].contains("local_value"));
  EXPECT_EQ(json["MainTree"]["local_value"].get<int>(), 7);
}

TEST(Groot2PublisherIntegration, BlackboardDump_ExportsExternalRootBlackboard)
{
  BT::BehaviorTreeFactory factory;
  factory.registerBehaviorTreeFromText(R"(
    <root BTCPP_format="4">
      <BehaviorTree ID="MainTree">
        <AlwaysSuccess/>
      </BehaviorTree>
    </root>)");

  auto external_root = BT::Blackboard::create();
  external_root->set("shared_value", 42);

  auto main_blackboard = BT::Blackboard::create(external_root);
  main_blackboard->set("local_value", 7);
  auto tree = factory.createTree("MainTree", main_blackboard);

  auto json = requestBlackboardDump(tree, "MainTree");
  ASSERT_TRUE(json.contains("MainTree"));
  ASSERT_TRUE(json.contains("ROOT"));

  ASSERT_TRUE(json["MainTree"].contains("local_value"));
  EXPECT_EQ(json["MainTree"]["local_value"].get<int>(), 7);
  EXPECT_FALSE(json["MainTree"].contains("shared_value"));

  ASSERT_TRUE(json["ROOT"].contains("shared_value"));
  EXPECT_EQ(json["ROOT"]["shared_value"].get<int>(), 42);
  EXPECT_FALSE(json["ROOT"].contains("local_value"));
}

TEST(Groot2PublisherIntegration, BlackboardDump_DeduplicatesSharedExternalRoot)
{
  BT::BehaviorTreeFactory factory;
  auto external_root = BT::Blackboard::create();
  external_root->set("shared_value", 99);

  auto main_blackboard = BT::Blackboard::create(external_root);
  auto tree = createTreeWithNamedSubtrees(factory, main_blackboard);
  ASSERT_EQ(tree.subtrees.size(), 3u);

  auto json = requestBlackboardDump(tree, "MainTree;ChildA;ChildB");
  ASSERT_TRUE(json.contains("MainTree"));
  ASSERT_TRUE(json.contains("ChildA"));
  ASSERT_TRUE(json.contains("ChildB"));
  ASSERT_TRUE(json.contains("ROOT"));
  EXPECT_EQ(json.size(), 4u);

  ASSERT_TRUE(json["ROOT"].contains("shared_value"));
  EXPECT_EQ(json["ROOT"]["shared_value"].get<int>(), 99);
}

TEST(Groot2PublisherIntegration, BlackboardDump_RejectsReservedRootNameCollision)
{
  BT::BehaviorTreeFactory factory;
  factory.registerBehaviorTreeFromText(R"(
    <root BTCPP_format="4">
      <BehaviorTree ID="ROOT">
        <AlwaysSuccess/>
      </BehaviorTree>
    </root>)");

  auto external_root = BT::Blackboard::create();
  external_root->set("shared_value", 42);

  auto main_blackboard = BT::Blackboard::create(external_root);
  main_blackboard->set("local_value", 7);
  auto tree = factory.createTree("ROOT", main_blackboard);

  auto error = requestBlackboardDumpError(tree, "ROOT");
  EXPECT_NE(error.find("reserved name [ROOT]"), std::string::npos);
}

TEST(Groot2PublisherIntegration, BlackboardDump_RejectsConflictingExternalRoots)
{
  BT::BehaviorTreeFactory factory;
  auto first_root = BT::Blackboard::create();
  first_root->set("shared_value", 99);

  auto main_blackboard = BT::Blackboard::create(first_root);
  auto tree = createTreeWithNamedSubtrees(factory, main_blackboard);

  auto second_root = BT::Blackboard::create();
  second_root->set("other_shared_value", 123);

  bool replaced_child_blackboard = false;
  for(auto& subtree : tree.subtrees)
  {
    if(subtree->instance_name == "ChildB")
    {
      subtree->blackboard = BT::Blackboard::create(second_root);
      replaced_child_blackboard = true;
      break;
    }
  }
  ASSERT_TRUE(replaced_child_blackboard);

  auto error = requestBlackboardDumpError(tree, "MainTree;ChildB");
  EXPECT_NE(error.find("multiple external root blackboards"), std::string::npos);
}
