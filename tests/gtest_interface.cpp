#include <gtest/gtest.h>

#include <behaviortree_cpp/bt_factory.h>
#include <iostream>
#include <memory>

// interface
struct IMotor
{
  virtual void doMove() = 0;
};

// implementation
struct LinearMotor : public IMotor
{
  void doMove() override
  {
    std::cout << ">> doMove" << std::endl;
  }
};

static const char* xml_text = R"(
<root BTCPP_format="4">
    <BehaviorTree ID="MainTree">
        <Sequence name="root_sequence">
            <PathFollow/>
        </Sequence>
    </BehaviorTree>
</root>
)";

// node using interface
class PathFollow : public BT::StatefulActionNode
{
public:
  PathFollow(const std::string& name, const BT::NodeConfig& config, IMotor& motor)
    : BT::StatefulActionNode(name, config), imotor_(motor)
  {}
  static BT::PortsList providedPorts()
  {
    return {};
  }
  BT::NodeStatus onStart() override
  {
    std::cout << "onStart" << std::endl;
    imotor_.doMove();
    return BT::NodeStatus::RUNNING;
  }
  BT::NodeStatus onRunning() override
  {
    std::cout << "onRunning" << std::endl;
    imotor_.doMove();
    return BT::NodeStatus::SUCCESS;
  }
  void onHalted() override
  {}

private:
  IMotor& imotor_;
};

TEST(Factory, VirtualInterface_Issue_945)
{
  LinearMotor motor;
  BT::BehaviorTreeFactory factory;
  factory.registerNodeType<PathFollow>("PathFollow", std::ref(motor));

  auto tree = factory.createTreeFromText(xml_text);
  tree.tickWhileRunning();
}
