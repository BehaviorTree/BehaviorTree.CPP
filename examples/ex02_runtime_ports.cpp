#include "behaviortree_cpp/bt_factory.h"
using namespace BT;

// clang-format off
static const char* xml_text = R"(
 <root BTCPP_format="4" >
     <BehaviorTree ID="MainTree">
        <Sequence name="root">
            <ThinkRuntimePort   text="{the_answer}"/>
            <SayRuntimePort     message="{the_answer}" />
        </Sequence>
     </BehaviorTree>
 </root>
 )";
// clang-format on

class ThinkRuntimePort : public BT::SyncActionNode
{
public:
  ThinkRuntimePort(const std::string& name, const BT::NodeConfig& config)
    : BT::SyncActionNode(name, config)
  {}

  BT::NodeStatus tick() override
  {
    setOutput("text", "The answer is 42");
    return NodeStatus::SUCCESS;
  }
};

class SayRuntimePort : public BT::SyncActionNode
{
public:
  SayRuntimePort(const std::string& name, const BT::NodeConfig& config)
    : BT::SyncActionNode(name, config)
  {}

  // You must override the virtual function tick()
  BT::NodeStatus tick() override
  {
    auto msg = getInput<std::string>("message");
    if(!msg)
    {
      throw BT::RuntimeError("missing required input [message]: ", msg.error());
    }
    std::cout << "Robot says: " << msg.value() << std::endl;
    return BT::NodeStatus::SUCCESS;
  }
};

int main()
{
  BehaviorTreeFactory factory;

  //-------- register ports that might be defined at runtime --------
  // more verbose way
  PortsList think_ports = { BT::OutputPort<std::string>("text") };
  factory.registerBuilder(
      CreateManifest<ThinkRuntimePort>("ThinkRuntimePort", think_ports),
      CreateBuilder<ThinkRuntimePort>());
  // less verbose way
  PortsList say_ports = { BT::InputPort<std::string>("message") };
  factory.registerNodeType<SayRuntimePort>("SayRuntimePort", say_ports);

  factory.registerBehaviorTreeFromText(xml_text);
  auto tree = factory.createTree("MainTree");
  tree.tickWhileRunning();

  return 0;
}
