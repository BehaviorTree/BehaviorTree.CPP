#include <gtest/gtest.h>
#include "behaviortree_cpp_v3/bt_factory.h"

using namespace BT;

class NodeWithPorts : public SyncActionNode
{
public:
  NodeWithPorts(const std::string& name, const NodeConfiguration& config) :
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
    <root main_tree_to_execute = "MainTree" >
        <BehaviorTree ID="MainTree">
            <NodeWithPorts name = "first"  in_port_B="66" />
        </BehaviorTree>
    </root>)";

  BehaviorTreeFactory factory;
  factory.registerNodeType<NodeWithPorts>("NodeWithPorts");

  auto tree = factory.createTreeFromText(xml_txt);

  NodeStatus status = tree.tickRoot();
  ASSERT_EQ(status, NodeStatus::SUCCESS);
}

TEST(PortTest, Descriptions)
{
  std::string xml_txt = R"(
    <root main_tree_to_execute = "MainTree" >
        <BehaviorTree ID="MainTree" _description="this is my tree" >
            <Sequence>
                <NodeWithPorts name="first"  in_port_B="66" _description="this is my action" />
                <SubTree ID="SubTree" name="second" _description="this is a subtree"/>
            </Sequence>
        </BehaviorTree>

        <BehaviorTree ID="SubTree" _description="this is a subtre" >
            <NodeWithPorts name="third" in_port_B="99" />
        </BehaviorTree>

    </root>)";

  BehaviorTreeFactory factory;
  factory.registerNodeType<NodeWithPorts>("NodeWithPorts");

  auto tree = factory.createTreeFromText(xml_txt);

  NodeStatus status = tree.tickRoot();
  ASSERT_EQ(status, NodeStatus::FAILURE);   // failure because in_port_B="99"
}

struct MyType
{
  std::string value;
};

class NodeInPorts : public SyncActionNode
{
public:
  NodeInPorts(const std::string& name, const NodeConfiguration& config) :
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
  NodeOutPorts(const std::string& name, const NodeConfiguration& config) :
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
    <root main_tree_to_execute = "MainTree" >
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

  NodeStatus status = tree.tickRoot();
  // expect failure because port is not set yet
  ASSERT_EQ(status, NodeStatus::FAILURE);
}

class IllegalPorts : public SyncActionNode
{
public:
  IllegalPorts(const std::string& name, const NodeConfiguration& config) :
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
