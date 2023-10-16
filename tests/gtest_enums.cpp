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
  switch (c) {
    case Color::Red: return "Red";
    case Color::Blue: return "Blue";
    case Color::Green: return "Green";
    case Color::Undefined: return "Undefined";
  }
  return nullptr;
}


class ActionEnum : public SyncActionNode
{
public:
  ActionEnum(const std::string& name, const NodeConfig& config) :
    SyncActionNode(name, config)
  {}

  NodeStatus tick() override
  {
    getInput("color", color);
    std::cout << "Node: " << name() << " has color : " << ToStr(color) << std::endl;
    return NodeStatus::SUCCESS;
  }

  static PortsList providedPorts()
  {
    return {BT::InputPort<Color>("color")};
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

  for(const auto& node: tree.subtrees.front()->nodes)
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

