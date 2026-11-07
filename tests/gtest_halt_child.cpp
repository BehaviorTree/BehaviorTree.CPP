#include "behaviortree_cpp/bt_factory.h"
#include "behaviortree_cpp/control_node.h"

#include <stdexcept>

#include <gtest/gtest.h>

using BT::NodeStatus;

namespace
{
// RUNNING until halted, and its onHalted() throws. Counts the halts.
class ThrowOnHalt : public BT::StatefulActionNode
{
public:
  ThrowOnHalt(const std::string& name, const BT::NodeConfig& config, int* halts)
    : StatefulActionNode(name, config), halts_(halts)
  {}
  static BT::PortsList providedPorts()
  {
    return {};
  }
  NodeStatus onStart() override
  {
    return NodeStatus::RUNNING;
  }
  NodeStatus onRunning() override
  {
    return NodeStatus::RUNNING;
  }
  void onHalted() override
  {
    (*halts_)++;
    throw std::runtime_error("halt failed");
  }

private:
  int* halts_;
};

// Ticks its only child and halts it on request through haltChild().
class HaltsChild : public BT::ControlNode
{
public:
  HaltsChild(const std::string& name, const BT::NodeConfig& config)
    : ControlNode(name, config)
  {}
  static BT::PortsList providedPorts()
  {
    return {};
  }
  NodeStatus tick() override
  {
    setStatus(NodeStatus::RUNNING);
    return children_nodes_[0]->executeTick();
  }
  void halt() override
  {
    haltChild(0);
    resetStatus();
  }
};
}  // namespace

// A child whose halt() throws must not stay RUNNING, or the next haltChild()
// halts it again and throws a second time.
TEST(ControlNodeHaltChild, ThrowingHaltIsNotRepeated)
{
  int halts = 0;
  BT::BehaviorTreeFactory factory;
  factory.registerNodeType<ThrowOnHalt>("ThrowOnHalt", &halts);
  factory.registerNodeType<HaltsChild>("HaltsChild");
  auto tree = factory.createTreeFromText(R"(
    <root BTCPP_format="4"><BehaviorTree>
      <HaltsChild><ThrowOnHalt/></HaltsChild>
    </BehaviorTree></root>)");

  EXPECT_EQ(tree.tickOnce(), NodeStatus::RUNNING);
  auto* node = dynamic_cast<HaltsChild*>(tree.rootNode());
  EXPECT_THROW(node->halt(), std::runtime_error);
  EXPECT_EQ(halts, 1);
  EXPECT_NO_THROW(node->halt());
  EXPECT_EQ(halts, 1);
  EXPECT_EQ(tree.subtrees[0]->nodes[1]->status(), NodeStatus::IDLE);
}

namespace
{
// Built-in parents halt their children via resetChildren()/resetChild(), not
// haltChild(): the failed child must end up IDLE there too.
void checkBuiltinParentThrowingHalt(const char* body)
{
  int halts = 0;
  BT::BehaviorTreeFactory factory;
  factory.registerNodeType<ThrowOnHalt>("ThrowOnHalt", &halts);
  auto tree =
      factory.createTreeFromText(std::string(R"(<root BTCPP_format="4"><BehaviorTree>)") +
                                 body + "</BehaviorTree></root>");

  EXPECT_EQ(tree.tickOnce(), NodeStatus::RUNNING);
  EXPECT_ANY_THROW(tree.haltTree());
  EXPECT_EQ(halts, 1);
  for(const auto& node : tree.subtrees[0]->nodes)
  {
    EXPECT_EQ(node->status(), NodeStatus::IDLE) << node->name();
  }
  EXPECT_NO_THROW(tree.haltTree());
  EXPECT_EQ(halts, 1);
  // ~Tree() calls haltTree() again: it must not throw (-> terminate).
}
}  // namespace

TEST(ControlNodeHaltChild, ThrowingHaltInSequence)
{
  checkBuiltinParentThrowingHalt("<Sequence><ThrowOnHalt/></Sequence>");
}

TEST(ControlNodeHaltChild, ThrowingHaltInRetryDecorator)
{
  checkBuiltinParentThrowingHalt(
      R"(<RetryUntilSuccessful num_attempts="3"><ThrowOnHalt/></RetryUntilSuccessful>)");
}
