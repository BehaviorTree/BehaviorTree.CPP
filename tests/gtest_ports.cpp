#include <gtest/gtest.h>
#include "behaviortree_cpp/bt_factory.h"

using namespace BT;

class NodeWithPorts : public SyncActionNode
{
public:
  NodeWithPorts(const std::string& name, const NodeConfig& config) :
    SyncActionNode(name, config)
  {
    std::cout << "ctor" << std::endl;
  }

  NodeStatus tick()
  {
    int val_A = 0;
    int val_B = 0;
    if (getInput("in_port_A", val_A) && getInput("in_port_B", val_B) && val_A == 42 &&
        val_B == 66)
    {
      return NodeStatus::SUCCESS;
    }
    return NodeStatus::FAILURE;
  }

  static PortsList providedPorts()
  {
    return {BT::InputPort<int>("in_port_A", 42, "magic_number"), BT::InputPort<int>("in_"
                                                                                    "port"
                                                                                    "_"
                                                                                    "B")};
  }
};

TEST(PortTest, DefaultPorts)
{
  std::string xml_txt = R"(
    <root BTCPP_format="4" >
        <BehaviorTree ID="MainTree">
            <NodeWithPorts name = "first"  in_port_B="66" />
        </BehaviorTree>
    </root>)";

  BehaviorTreeFactory factory;
  factory.registerNodeType<NodeWithPorts>("NodeWithPorts");

  auto tree = factory.createTreeFromText(xml_txt);

  NodeStatus status = tree.tickWhileRunning();
  ASSERT_EQ(status, NodeStatus::SUCCESS);
}

TEST(PortTest, Descriptions)
{
  std::string xml_txt = R"(
    <root BTCPP_format="4" >
        <BehaviorTree ID="MainTree" _description="this is my tree" >
            <Sequence>
                <NodeWithPorts name="first"  in_port_B="66" _description="this is my action" />
                <SubTree ID="SubTree" name="second" _description="this is a subtree"/>
            </Sequence>
        </BehaviorTree>

        <BehaviorTree ID="SubTree" _description="this is a subtree" >
            <NodeWithPorts name="third" in_port_B="99" />
        </BehaviorTree>

    </root>)";

  BehaviorTreeFactory factory;
  factory.registerNodeType<NodeWithPorts>("NodeWithPorts");

  factory.registerBehaviorTreeFromText(xml_txt);
  auto tree = factory.createTree("MainTree");

  NodeStatus status = tree.tickWhileRunning();
  while (status == NodeStatus::RUNNING)
  {
    status = tree.tickWhileRunning();
  }

  ASSERT_EQ(status, NodeStatus::FAILURE);   // failure because in_port_B="99"
}

struct MyType
{
  std::string value;
};

class NodeInPorts : public SyncActionNode
{
public:
  NodeInPorts(const std::string& name, const NodeConfig& config) :
    SyncActionNode(name, config)
  {}

  NodeStatus tick()
  {
    int val_A = 0;
    MyType val_B;
    if (getInput("int_port", val_A) && getInput("any_port", val_B))
    {
      return NodeStatus::SUCCESS;
    }
    return NodeStatus::FAILURE;
  }

  static PortsList providedPorts()
  {
    return {BT::InputPort<int>("int_port"), BT::InputPort<MyType>("any_port")};
  }
};

class NodeOutPorts : public SyncActionNode
{
public:
  NodeOutPorts(const std::string& name, const NodeConfig& config) :
    SyncActionNode(name, config)
  {}

  NodeStatus tick()
  {
    return NodeStatus::SUCCESS;
  }

  static PortsList providedPorts()
  {
    return {BT::OutputPort<int>("int_port"), BT::OutputPort<MyType>("any_port")};
  }
};

TEST(PortTest, EmptyPort)
{
  std::string xml_txt = R"(
    <root BTCPP_format="4" >
        <BehaviorTree ID="MainTree">
            <Sequence>
                <NodeInPorts  int_port="{ip}" any_port="{ap}" />
                <NodeOutPorts int_port="{ip}" any_port="{ap}" />
            </Sequence>
        </BehaviorTree>
    </root>)";

  BehaviorTreeFactory factory;
  factory.registerNodeType<NodeOutPorts>("NodeOutPorts");
  factory.registerNodeType<NodeInPorts>("NodeInPorts");

  auto tree = factory.createTreeFromText(xml_txt);

  NodeStatus status = tree.tickWhileRunning();
  // expect failure because port is not set yet
  ASSERT_EQ(status, NodeStatus::FAILURE);
}

class IllegalPorts : public SyncActionNode
{
public:
  IllegalPorts(const std::string& name, const NodeConfig& config) :
    SyncActionNode(name, config)
  {}

  NodeStatus tick()
  {
    return NodeStatus::SUCCESS;
  }

  static PortsList providedPorts()
  {
    return {BT::InputPort<std::string>("name")};
  }
};

TEST(PortTest, IllegalPorts)
{
  BehaviorTreeFactory factory;
  ASSERT_ANY_THROW(factory.registerNodeType<IllegalPorts>("nope"));
}


class ActionVectorIn : public SyncActionNode
{
public:
  ActionVectorIn(const std::string& name, const NodeConfig& config,
                 std::vector<double>* states) :
    SyncActionNode(name, config),
    states_(states)
  {}

  NodeStatus tick() override
  {
    getInput("states", *states_);
    return NodeStatus::SUCCESS;
  }

  static PortsList providedPorts()
  {
    return {BT::InputPort<std::vector<double>>("states")};
  }
private:
  std::vector<double>* states_;
};


TEST(PortTest, SubtreeStringInput_Issue489)
{
  std::string xml_txt = R"(
    <root BTCPP_format="4" >
      <BehaviorTree ID="Main">
        <SubTree ID="Subtree_A" states="3;7"/>
      </BehaviorTree>

      <BehaviorTree ID="Subtree_A">
        <ActionVectorIn states="{states}"/>
      </BehaviorTree>
    </root>)";

  std::vector<double> states;

  BehaviorTreeFactory factory;
  factory.registerNodeType<ActionVectorIn>("ActionVectorIn", &states);

  factory.registerBehaviorTreeFromText(xml_txt);
  auto tree = factory.createTree("Main");

  NodeStatus status = tree.tickWhileRunning();

  ASSERT_EQ(status, NodeStatus::SUCCESS);
  ASSERT_EQ(2, states.size());
  ASSERT_EQ(3, states[0]);
  ASSERT_EQ(7, states[1]);
}

enum class Color
{
  Red = 0,
  Blue = 1,
  Green = 2,
  Undefined
};

class ActionEnum : public SyncActionNode
{
public:
  ActionEnum(const std::string& name, const NodeConfig& config) :
    SyncActionNode(name, config)
  {}

  NodeStatus tick() override
  {
    getInput("color", color);
    return NodeStatus::SUCCESS;
  }

  static PortsList providedPorts()
  {
    return {BT::InputPort<Color>("color")};
  }

  Color color = Color::Undefined;
};

TEST(PortTest, StrintToEnum)
{
  std::string xml_txt = R"(
    <root BTCPP_format="4" >
      <BehaviorTree ID="Main">
        <Sequence>
          <ActionEnum color="Blue"/>
          <ActionEnum color="2"/>
        </Sequence>
      </BehaviorTree>
    </root>)";

  BehaviorTreeFactory factory;
  factory.registerNodeType<ActionEnum>("ActionEnum");
  factory.registerScriptingEnums<Color>();

  auto tree = factory.createTreeFromText(xml_txt);

  NodeStatus status = tree.tickWhileRunning();

  ASSERT_EQ(status, NodeStatus::SUCCESS);

  auto first_node = dynamic_cast<ActionEnum*>(tree.subtrees.front()->nodes[1].get());
  auto second_node = dynamic_cast<ActionEnum*>(tree.subtrees.front()->nodes[2].get());

  ASSERT_EQ(Color::Blue, first_node->color);
  ASSERT_EQ(Color::Green, second_node->color);
}



