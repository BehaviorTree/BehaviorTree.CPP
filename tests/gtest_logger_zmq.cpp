#include <gtest/gtest.h>

#include "behaviortree_cpp/bt_factory.h"
#include "behaviortree_cpp/loggers/bt_zmq_publisher.h"

TEST(LoggerZMQ, ZMQLoggerDeletesCleanlyAfterTickingTree)
{
  // GIVEN we create a behavior tree through the BT factory and attach a ZMQ publisher to it
  static constexpr auto XML = R"(
<root BTCPP_format="4">
    <BehaviorTree>
    <SetBlackboard output_key="arg1" value="1" />
    </BehaviorTree>
</root>
)";
  BT::BehaviorTreeFactory factory;
  auto tree = factory.createTreeFromText(XML);

  {
    BT::PublisherZMQ zmq_logger{ tree, 1 };
    tree.tickWhileRunning();
    // WHEN zmq_logger goes out of scope
  }

  // THEN zmq_logger is destroyed cleanly without segfaulting
  SUCCEED();
}

TEST(LoggerZMQ, ZMQLoggerDeletesCleanlyAfterNotTickingTree)
{
  // GIVEN we create a behavior tree through the BT factory and attach a ZMQ publisher to it
  static constexpr auto XML = R"(
<root BTCPP_format="4">
    <BehaviorTree>
    <SetBlackboard output_key="arg1" value="1" />
    </BehaviorTree>
</root>
)";
  BT::BehaviorTreeFactory factory;
  auto tree = factory.createTreeFromText(XML);

  {
    BT::PublisherZMQ zmq_logger{ tree, 1 };
    // GIVEN we haven't even ticked the tree, so ZMQ hasn't published any state change messages
    // (meaning no send is pending and send_future_ has not been set)
    // WHEN zmq_logger goes out of scope
  }

  // THEN zmq_logger is destroyed cleanly without segfaulting
  SUCCEED();
}
