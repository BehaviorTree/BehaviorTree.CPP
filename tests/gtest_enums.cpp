#include <gtest/gtest.h>
#include "behaviortree_cpp/bt_factory.h"

using namespace BT;

enum class Color
{
  Red = 0,
  Blue = 1,
  Green = 2,
  Undefined
};

static const char* ToStr(const Color& c)
{
  switch(c)
  {
    case Color::Red:
      return "Red";
    case Color::Blue:
      return "Blue";
    case Color::Green:
      return "Green";
    case Color::Undefined:
      return "Undefined";
  }
  return nullptr;
}

class ActionEnum : public SyncActionNode
{
public:
  ActionEnum(const std::string& name, const NodeConfig& config)
    : SyncActionNode(name, config)
  {}

  NodeStatus tick() override
  {
    getInput("color", color);
    std::cout << "Node: " << name() << " has color : " << ToStr(color) << std::endl;
    return NodeStatus::SUCCESS;
  }

  static PortsList providedPorts()
  {
    return { BT::InputPort<Color>("color") };
  }

  Color color = Color::Undefined;
};

TEST(Enums, StrintToEnum)
{
  std::string xml_txt = R"(
    <root BTCPP_format="4" >
      <BehaviorTree ID="Main">
        <Sequence>
          <Script code=" my_color := Red "/>
          <ActionEnum name="maybe_blue" color="Blue"/>
          <ActionEnum name="maybe_green" color="2"/>
          <ActionEnum name="maybe_red" color="{my_color}"/>
        </Sequence>
      </BehaviorTree>
    </root>)";

  BehaviorTreeFactory factory;
  factory.registerNodeType<ActionEnum>("ActionEnum");
  factory.registerScriptingEnums<Color>();

  auto tree = factory.createTreeFromText(xml_txt);

  NodeStatus status = tree.tickWhileRunning();

  ASSERT_EQ(status, NodeStatus::SUCCESS);

  for(const auto& node : tree.subtrees.front()->nodes)
  {
    if(auto enum_node = dynamic_cast<ActionEnum*>(node.get()))
    {
      if(enum_node->name() == "maybe_red")
      {
        ASSERT_EQ(Color::Red, enum_node->color);
      }
      else if(enum_node->name() == "maybe_green")
      {
        ASSERT_EQ(Color::Green, enum_node->color);
      }
      else if(enum_node->name() == "maybe_blue")
      {
        ASSERT_EQ(Color::Blue, enum_node->color);
      }
    }
  }
}

TEST(Enums, SwitchNodeWithEnum)
{
  const std::string xml_txt = R"(
    <root BTCPP_format="4" >
      <BehaviorTree ID="Main">
        <Sequence>
          <Script code=" my_color := Blue "/>
          <Switch4 variable="{my_color}"
            case_1="Red"
            case_2="Blue"
            case_3="Green"
            case_4="Undefined">
            <AlwaysFailure name="case_red" />
            <AlwaysSuccess name="case_blue" />
            <AlwaysFailure name="case_green" />
            <AlwaysFailure name="case_undefined" />
            <AlwaysFailure name="default_case" />
          </Switch4>
        </Sequence>
      </BehaviorTree>
    </root>)";

  BehaviorTreeFactory factory;
  factory.registerScriptingEnums<Color>();

  auto tree = factory.createTreeFromText(xml_txt);

  NodeStatus status = tree.tickWhileRunning();

  ASSERT_EQ(status, NodeStatus::SUCCESS);
}

enum BatteryStatus
{
  NO_FAULT = 0,
  LOW_BATTERY = 1
};

class PrintEnum : public BT::ConditionNode
{
public:
  explicit PrintEnum(const std::string& name, const BT::NodeConfig& config)
    : ConditionNode(name, config)
  {}

  ~PrintEnum() override = default;

  static BT::PortsList providedPorts()
  {
    return {
      BT::InputPort<BatteryStatus>("enum", "Name of the check"),
    };
  }

private:
  BT::NodeStatus tick() override
  {
    auto enum_value = getInput<BatteryStatus>("enum");
    if(!enum_value)
    {
      std::cout << "missing required input [enum]" << std::endl;
      return BT::NodeStatus::FAILURE;
    }
    std::cout << "Enum value: " << (enum_value == NO_FAULT ? "NO_FAULT" : "LOW_BATTERY")
              << std::endl;
    return BT::NodeStatus::SUCCESS;
  }
};

class IsHealthOk : public BT::ConditionNode
{
public:
  explicit IsHealthOk(const std::string& name, const BT::NodeConfig& config)
    : BT::ConditionNode(name, config)
  {}

  ~IsHealthOk() override = default;

  static BT::PortsList providedPorts()
  {
    return { BT::InputPort<std::string>("check_name"), BT::InputPort<bool>("health") };
  }

private:
  BT::NodeStatus tick() override
  {
    auto health = getInput<bool>("health");
    if(!health)
    {
      std::cout << "missing required input [health]" << std::endl;
      return BT::NodeStatus::FAILURE;
    }

    if(health.value())
    {
      return BT::NodeStatus::SUCCESS;
    }
    else
    {
      std::cerr << "IsHealthOk FAILED " << std::endl;
      return BT::NodeStatus::FAILURE;
    }
    return BT::NodeStatus::SUCCESS;
  }
};

TEST(Enums, SubtreeRemapping)
{
  const std::string xml_txt = R"(
    <root BTCPP_format="4" >
      <BehaviorTree ID="MainTree">
        <Sequence>
          <Script code=" fault_status := NO_FAULT " />
          <PrintEnum enum="{fault_status}"/>

          <SubTree ID="FailsafeCheck"
            health="false"
            trigger_fault_status="LOW_BATTERY"
            fault_status="{=}" />

          <PrintEnum enum="{fault_status}"/>
        </Sequence>
      </BehaviorTree>

      <BehaviorTree ID="FailsafeCheck">
        <ForceSuccess>
          <IsHealthOk
              health="{health}"
              _onFailure="fault_status = trigger_fault_status"/>
        </ForceSuccess>
      </BehaviorTree>
    </root>)";

  BehaviorTreeFactory factory;
  factory.registerScriptingEnums<BatteryStatus>();
  factory.registerNodeType<PrintEnum>("PrintEnum");
  factory.registerNodeType<IsHealthOk>("IsHealthOk");

  factory.registerBehaviorTreeFromText(xml_txt);

  auto tree = factory.createTree("MainTree");
  NodeStatus status = tree.tickWhileRunning();

  ASSERT_EQ(status, NodeStatus::SUCCESS);
  ASSERT_EQ(tree.rootBlackboard()->get<BatteryStatus>("fault_status"), LOW_BATTERY);
}
