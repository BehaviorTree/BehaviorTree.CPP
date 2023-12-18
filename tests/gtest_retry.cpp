#include <gtest/gtest.h>
#include "behaviortree_cpp_v3/bt_factory.h"
#include "behaviortree_cpp_v3/decorators/retry_node.h"

using BT::NodeStatus;

class FlipN : public BT::SyncActionNode
{
public:
  FlipN(const std::string& name, int N, const NodeStatus initial_return_value):
    BT::SyncActionNode(name, {}), counter_(N), initial_return_value_(initial_return_value)
  {
  }

  NodeStatus tick() override
  {
    // flip returned value after calling tick() N times
    const NodeStatus return_val = counter_-- > 0 ? initial_return_value_ : (initial_return_value_ == NodeStatus::SUCCESS ? NodeStatus::FAILURE : NodeStatus::SUCCESS);
    const std::string msg = return_val == NodeStatus::SUCCESS ? "SUCCESS" : "FAILURE";
    std::cout << name() << "; " << msg << std::endl;
    return return_val;
  }

  private:
    NodeStatus initial_return_value_ = NodeStatus::SUCCESS;
    int counter_ = 0;
};

/**
 * This test has a reactive sequence with a condition that return SUCCESS in the first tick and FAILURE in the second tick.
 * The other child in the reactive sequence is an action with a retry decorator.
 * The Action returns FAILURE in the first tick and SUCCESS in the second tick.
 * The retry decorator has a max_attempts of 2.
 *
 * In the first tick, the condition returns SUCCESS, thus the action is ticked, but it returns FAILURE.
 * Since the decorator is a retry, we expect it to return RUNNING.
 * In the second tick, the condition return FAILURE, the retry should be halted and the action is not ticked.
 * The result of the reactive sequence should be FAILURE.
*/
TEST(Retry, test)
{
  BT::ReactiveSequence root("root");
  BT::RetryNode retry("retry", 2);
  FlipN condition_1("condition_1", 1, NodeStatus::SUCCESS);
  FlipN action_1("action_1", 1, NodeStatus::FAILURE);
  root.addChild(&condition_1);
  retry.setChild(&action_1);
  root.addChild(&retry);

  BT::NodeStatus status = root.executeTick();
  ASSERT_EQ(status, BT::NodeStatus::RUNNING);

  status = root.executeTick();
  ASSERT_EQ(status, BT::NodeStatus::FAILURE);
}


