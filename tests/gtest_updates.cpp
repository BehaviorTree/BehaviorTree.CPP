#include <gtest/gtest.h>
#include <string>
#include "behaviortree_cpp/basic_types.h"
#include "behaviortree_cpp/bt_factory.h"
#include "test_helper.hpp"

using namespace BT;

const std::string xml_text_check = R"(

  <root BTCPP_format="4" >
    <BehaviorTree ID="Check">
      <Sequence>

        <Fallback>
          <WasEntryUpdated entry="A"/>
          <TestA/>
        </Fallback>

        <SkipUnlessUpdated entry="A">
          <TestB/>
        </SkipUnlessUpdated>

      </Sequence>
    </BehaviorTree>
  </root>)";

TEST(EntryUpdates, NoEntry)
{
  BehaviorTreeFactory factory;
  std::array<int, 2> counters;
  RegisterTestTick(factory, "Test", counters);

  const std::string xml_text = R"(
    <root BTCPP_format="4" >
      <BehaviorTree ID="Main">
        <Sequence>
          <SubTree ID="Check" _autoremap="true"/>
        </Sequence>
      </BehaviorTree>
    </root>)";

  factory.registerBehaviorTreeFromText(xml_text_check);
  factory.registerBehaviorTreeFromText(xml_text);
  auto tree = factory.createTree("Main");
  const auto status = tree.tickWhileRunning();
  ASSERT_EQ(status, NodeStatus::SUCCESS);
  EXPECT_EQ(1, counters[0]);  // fallback!
  EXPECT_EQ(0, counters[1]);  // skipped
}

TEST(EntryUpdates, Initialized)
{
  BehaviorTreeFactory factory;
  std::array<int, 2> counters;
  RegisterTestTick(factory, "Test", counters);

  const std::string xml_text = R"(
    <root BTCPP_format="4" >
      <BehaviorTree ID="Main">
        <Sequence>
          <Script code="A:=1;B:=1"/>
          <SubTree ID="Check" _autoremap="true"/>
        </Sequence>
      </BehaviorTree>
    </root>)";

  factory.registerBehaviorTreeFromText(xml_text_check);
  factory.registerBehaviorTreeFromText(xml_text);
  auto tree = factory.createTree("Main");
  const auto status = tree.tickWhileRunning();
  ASSERT_EQ(status, NodeStatus::SUCCESS);
  EXPECT_EQ(0, counters[0]);  // fallback!
  EXPECT_EQ(1, counters[1]);  // skipped
}

TEST(EntryUpdates, UpdateOnce)
{
  BehaviorTreeFactory factory;
  std::array<int, 2> counters;
  RegisterTestTick(factory, "Test", counters);

  const std::string xml_text = R"(
    <root BTCPP_format="4" >
      <BehaviorTree ID="Main">
        <Sequence>
          <Script code="A:=1"/>
          <Repeat num_cycles="2" >
            <SubTree ID="Check" _autoremap="true"/>
          </Repeat>
        </Sequence>
      </BehaviorTree>
    </root>)";

  factory.registerBehaviorTreeFromText(xml_text_check);
  factory.registerBehaviorTreeFromText(xml_text);
  auto tree = factory.createTree("Main");
  const auto status = tree.tickWhileRunning();
  ASSERT_EQ(status, NodeStatus::SUCCESS);
  EXPECT_EQ(1, counters[0]);  // fallback!
  EXPECT_EQ(1, counters[1]);  // skipped
}

TEST(EntryUpdates, UpdateTwice)
{
  BehaviorTreeFactory factory;
  std::array<int, 2> counters;
  RegisterTestTick(factory, "Test", counters);

  const std::string xml_text = R"(
    <root BTCPP_format="4" >
      <BehaviorTree ID="Main">
        <Repeat num_cycles="2" >
          <Sequence>
            <Script code="A:=1"/>
            <SubTree ID="Check" _autoremap="true"/>
          </Sequence>
        </Repeat>
      </BehaviorTree>
    </root>)";

  factory.registerBehaviorTreeFromText(xml_text_check);
  factory.registerBehaviorTreeFromText(xml_text);
  auto tree = factory.createTree("Main");
  const auto status = tree.tickWhileRunning();
  ASSERT_EQ(status, NodeStatus::SUCCESS);
  EXPECT_EQ(0, counters[0]);  // fallback!
  EXPECT_EQ(2, counters[1]);  // skipped
}
