#include <chrono>
#include <cstdlib>
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
  factory.registerSimpleAction("ThrowRuntimeError", [](BT::TreeNode&) -> BT::NodeStatus {
    throw BT::RuntimeError("Oops!");
  });

  auto tree = factory.createTreeFromText(xml_text);
  BT::Groot2Publisher publisher(tree);
  EXPECT_THROW(tree.tickExactlyOnce(), BT::RuntimeError);
}
}  // namespace

TEST(Groot2PublisherTest, EnsureNoInfiniteLoopOnThrow)
{
  EXPECT_EXIT(
      {
        auto fut = std::async(std::launch::async, throwRuntimeError);
        auto status = fut.wait_for(1s);  // shouldn't run for more than 1 second
        if(status != std::future_status::ready)
        {
          std::cerr << "Test took too long. Possible infinite loop.\n";
          std::exit(EXIT_FAILURE);
        }
        std::exit(EXIT_SUCCESS);
      },
      ::testing::ExitedWithCode(EXIT_SUCCESS), ".*");
}
