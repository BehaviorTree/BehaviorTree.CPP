#include <gtest/gtest.h>
#include "behaviortree_cpp_v3/bt_factory.h"
#include "test_helper.hpp"
#include "behaviortree_cpp_v3/loggers/bt_cout_logger.h"

using BT::NodeStatus;
using std::chrono::milliseconds;

class SleepNode : public BT::StatefulActionNode
{
public:

  SleepNode(const std::string& name, const BT::NodeConfiguration& config):
    StatefulActionNode(name, config) {}

  NodeStatus onStart() override {
    count_ = 0;
    return NodeStatus::RUNNING;
  }

  NodeStatus onRunning() override {
    return ++count_ < 10 ? NodeStatus::RUNNING : NodeStatus::SUCCESS;
  }

  void onHalted() override {}

  static BT::PortsList providedPorts(){
    return {};
  }

private:
  int count_ = 0;
};


TEST(Reactive, TestLogging)
{
  using namespace BT;

  static const char* reactive_xml_text = R"(
<root>
  <BehaviorTree ID="Main">
    <ReactiveSequence>
      <TestA name="testA"/>
      <AlwaysSuccess name="success"/>
      <Sleep/>
    </ReactiveSequence>
  </BehaviorTree>
</root>
)";

  BehaviorTreeFactory factory;

  factory.registerNodeType<SleepNode>("Sleep");

  std::array<int, 1> counters;
  RegisterTestTick(factory, "Test", counters);

  auto tree = factory.createTreeFromText(reactive_xml_text);
  StdCoutLogger logger(tree);

  auto ret = tree.tickRootWhileRunning();
  ASSERT_EQ(ret, NodeStatus::SUCCESS);

  int num_ticks = counters[0];
  ASSERT_GE(num_ticks, 10);
}


