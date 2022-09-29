#include <gtest/gtest.h>
#include "behaviortree_cpp_v3/bt_factory.h"

using namespace BT;

class FastAction : public BT::AsyncActionNode
{
public:
  // Any TreeNode with ports must have a constructor with this signature
  FastAction(const std::string& name, const BT::NodeConfiguration& config) :
    AsyncActionNode(name, config)
  {}

  static BT::PortsList providedPorts()
  {
    return {};
  }

  BT::NodeStatus tick() override
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    return BT::NodeStatus::SUCCESS;
  }
};

TEST(WakeUp, BasicTest)
{
  static const char* xml_text = R"(

<root>
    <BehaviorTree ID="MainTree">
        <FastAction/>
    </BehaviorTree>
</root> )";

  BehaviorTreeFactory factory;
  factory.registerNodeType<FastAction>("FastAction");

  Tree tree = factory.createTreeFromText(xml_text);

  using namespace std::chrono;

  auto t1 = system_clock::now();
  tree.tickRoot();
  tree.sleep(milliseconds(200));
  auto t2 = system_clock::now();

  auto dT = duration_cast<milliseconds>(t2 - t1).count();
  std::cout << "Woke up after msec: " << dT << std::endl;

  ASSERT_LT(dT, 25);
}
