#include <chrono>
#include <future>

#include <behaviortree_cpp/loggers/groot2_protocol.h>
#include <behaviortree_cpp/loggers/groot2_publisher.h>
#include <gtest/gtest.h>

using namespace std::chrono_literals;

namespace
{
static const char* xml_text = R"(
<root BTCPP_format="4">
    <BehaviorTree ID="MainTree">
        <ThrowRuntimeError/>
    </BehaviorTree>
</root>
)";

void throwRuntimeError()
{
  BT::BehaviorTreeFactory factory;
  factory.registerSimpleAction("ThrowRuntimeError", [](BT::TreeNode&) {
    throw BT::RuntimeError("Oops!");
    return BT::NodeStatus::FAILURE;
  });

  auto tree = factory.createTreeFromText(xml_text);
  BT::Groot2Publisher publisher(tree);
  EXPECT_THROW(tree.tickOnce(), BT::RuntimeError);
}
}  // namespace

TEST(Groot2PublisherTest, EnsureNoInfiniteLoopOnThrow)
{
  auto fut = std::async(std::launch::async, throwRuntimeError);
  auto status = fut.wait_for(1s);
  EXPECT_EQ(status, std::future_status::ready);

  if(status != std::future_status::ready)
  {
    std::exit(1);  // Force exit to avoid destructor blocking
  }
}
