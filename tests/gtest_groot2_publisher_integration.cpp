#include "groot2_test_utils.hpp"

#include <atomic>
#include <cstdlib>

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

nlohmann::json makeReplaceHook(uint16_t uid, BT::Monitor::Hook::Position position,
                               bool once = false)
{
  return { { "enabled", true },
           { "uid", uid },
           { "mode", int(BT::Monitor::Hook::Mode::REPLACE) },
           { "once", once },
           { "desired_status", "FAILURE" },
           { "position", int(position) } };
}

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

  const auto hook = makeReplaceHook(uid, BT::Monitor::Hook::Position::POST);
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
