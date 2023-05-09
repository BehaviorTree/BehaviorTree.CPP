#include <gtest/gtest.h>
#include "behaviortree_cpp/bt_factory.h"
#include "test_helper.hpp"

using BT::NodeStatus;
using std::chrono::milliseconds;

static const char* reactive_xml_text = R"(
<root BTCPP_format="4" >
  <BehaviorTree ID="MainTree">
    <ReactiveSequence>
      <Sequence name="first">
        <TestA/>
        <TestB/>
        <TestC/>
      </Sequence>
      <AsyncSequence name="second">
        <TestD/>
        <TestE/>
        <TestF/>
      </AsyncSequence>
    </ReactiveSequence>
  </BehaviorTree>
</root>
)";


TEST(Reactive, RunningChildren)
{
  BT::BehaviorTreeFactory factory;
  std::array<int, 6> counters;
  RegisterTestTick(factory, "Test", counters);

  auto tree = factory.createTreeFromText(reactive_xml_text);

  NodeStatus status = NodeStatus::IDLE;

  int count=0;
  while(!BT::isStatusCompleted(status) && count<100)
  {
    count++;
    status = tree.tickExactlyOnce();
  }

  ASSERT_NE(100, count);

  ASSERT_EQ(status, NodeStatus::SUCCESS);

  ASSERT_EQ(counters[0], 3);
  ASSERT_EQ(counters[1], 3);
  ASSERT_EQ(counters[2], 3);

  ASSERT_EQ(counters[3], 1);
  ASSERT_EQ(counters[4], 1);
  ASSERT_EQ(counters[5], 1);
}


