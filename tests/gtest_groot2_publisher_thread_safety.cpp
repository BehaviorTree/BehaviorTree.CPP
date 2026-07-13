#include "groot2_test_utils.hpp"

#include <atomic>
#include <chrono>
#include <future>
#include <thread>

#include <behaviortree_cpp/bt_factory.h>
#include <gtest/gtest.h>

using namespace std::chrono_literals;

namespace
{
constexpr auto kTreeXml = R"(
<root BTCPP_format="4">
  <BehaviorTree ID="MainTree">
    <AlwaysSuccess/>
  </BehaviorTree>
</root>
)";

}  // namespace

// This is an end-to-end TSan reproducer rather than a deterministic functional
// failure: without TSan the data races may remain invisible and the test passes.
TEST(Groot2PublisherThreadSafety, UpdateHeartbeatDelayWhileHeartbeatThreadIsRunning)
{
  BT::BehaviorTreeFactory factory;
  auto tree = factory.createTreeFromText(kTreeXml);
  auto publisher_and_port = Groot2Test::makePublisher(tree);

  // The publisher's heartbeat thread reads this setting concurrently.
  for(size_t i = 0; i < 100; ++i)
  {
    publisher_and_port.publisher->setMaxHeartbeatDelay((i % 2 == 0) ? 1ms : 1h);
    std::this_thread::sleep_for(1ms);
  }
}

TEST(Groot2PublisherThreadSafety, RecordTransitionsWhileTreeIsTicking)
{
  BT::BehaviorTreeFactory factory;
  auto tree = factory.createTreeFromText(kTreeXml);
  auto [publisher, port] = Groot2Test::makePublisher(tree);
  Groot2Test::Client client(port);

  std::atomic_bool keep_ticking = true;
  std::thread tick_thread([&] {
    while(keep_ticking.load())
    {
      tree.tickExactlyOnce();
      tree.haltTree();
    }
  });

  try
  {
    // Starting while callbacks are already active also covers initialization of
    // the recording timestamp relative to the recording flag.
    client.request(BT::Monitor::RequestType::TOGGLE_RECORDING, "start");

    // GET_TRANSITIONS runs on the publisher's server thread while status-change
    // callbacks append transitions from tick_thread.
    for(size_t i = 0; i < 100; ++i)
    {
      client.request(BT::Monitor::RequestType::GET_TRANSITIONS);
    }

    client.request(BT::Monitor::RequestType::TOGGLE_RECORDING, "stop");
  }
  catch(...)
  {
    keep_ticking = false;
    tick_thread.join();
    throw;
  }

  keep_ticking = false;
  tick_thread.join();
}

TEST(Groot2PublisherThreadSafety, DestroyWhileStatusCallbacksAreRunning)
{
  BT::BehaviorTreeFactory factory;
  auto tree = factory.createTreeFromText(kTreeXml);
  auto publisher_and_port = Groot2Test::makePublisher(tree);

  std::atomic_bool keep_ticking = true;
  std::thread tick_thread([&] {
    while(keep_ticking.load())
    {
      tree.tickExactlyOnce();
    }
  });

  std::this_thread::sleep_for(20ms);
  publisher_and_port.publisher.reset();
  keep_ticking = false;
  tick_thread.join();
}

// TSan's lock-order detector reports an inversion in the old implementation:
// disabling took hooks_map_mutex -> hook->mutex, while one-shot removal took
// hook->mutex -> hooks_map_mutex.
TEST(Groot2PublisherThreadSafety, DisableThenRemoveOneShotHookUsesConsistentLockOrder)
{
  BT::BehaviorTreeFactory factory;
  auto tree = factory.createTreeFromText(kTreeXml);
  auto [publisher, port] = Groot2Test::makePublisher(tree);
  Groot2Test::Client client(port);

  auto hook = nlohmann::json{ { "enabled", true },
                              { "uid", tree.rootNode()->UID() },
                              { "mode", int(BT::Monitor::Hook::Mode::REPLACE) },
                              { "once", false },
                              { "desired_status", "FAILURE" },
                              { "position", int(BT::Monitor::Hook::Position::PRE) } };

  client.request(BT::Monitor::RequestType::HOOK_INSERT, hook.dump());
  client.request(BT::Monitor::RequestType::DISABLE_ALL_HOOKS);

  hook["enabled"] = true;
  hook["once"] = true;
  client.request(BT::Monitor::RequestType::HOOK_INSERT, hook.dump());

  EXPECT_EQ(tree.tickExactlyOnce(), BT::NodeStatus::FAILURE);
  EXPECT_EQ(tree.tickExactlyOnce(), BT::NodeStatus::SUCCESS);
}

TEST(Groot2PublisherThreadSafety, DestroyWhileBreakpointCallbackIsBlocked)
{
  BT::BehaviorTreeFactory factory;
  auto tree = factory.createTreeFromText(kTreeXml);
  auto [publisher, port] = Groot2Test::makePublisher(tree);
  Groot2Test::Client client(port);

  const auto hook =
      nlohmann::json{ { "enabled", true },
                      { "uid", tree.rootNode()->UID() },
                      { "mode", int(BT::Monitor::Hook::Mode::BREAKPOINT) },
                      { "once", false },
                      { "desired_status", "FAILURE" },
                      { "position", int(BT::Monitor::Hook::Position::PRE) } };
  client.request(BT::Monitor::RequestType::HOOK_INSERT, hook.dump());

  auto tick_result =
      std::async(std::launch::async, [&tree] { return tree.tickExactlyOnce(); });
  std::this_thread::sleep_for(20ms);

  publisher.reset();

  ASSERT_EQ(tick_result.wait_for(1s), std::future_status::ready);
  EXPECT_EQ(tick_result.get(), BT::NodeStatus::SUCCESS);
}
