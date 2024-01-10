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

  NodeStatus tick() override
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

  NodeStatus tick() override
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

  NodeStatus tick() override
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

  NodeStatus tick() override
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


class DefaultTestAction : public SyncActionNode
{
public:
  struct Point2D {
    int x;
    int y;
  };

  DefaultTestAction(const std::string& name, const NodeConfig& config) :
    SyncActionNode(name, config)
  {}

  NodeStatus tick() override
  {
    const int answer = getInput<int>("answer").value();
    if(answer != 42) {
      return NodeStatus::FAILURE;
    }

    const std::string greet = getInput<std::string>("greeting").value();
    if(greet != "hello") {
      return NodeStatus::FAILURE;
    }

    const Point2D point = getInput<Point2D>("pos").value();
    if(point.x != 1 || point.y != 2) {
      return NodeStatus::FAILURE;
    }

    return NodeStatus::SUCCESS;
  }

  static PortsList providedPorts()
  {
    return {BT::InputPort<int>("answer", 42, "the answer"),
            BT::InputPort<std::string>("greeting", "hello", "be polite"),
            BT::InputPort<Point2D>("pos", {1,2}, "where")};
  }
};

TEST(PortTest, DefaultInput)
{
  std::string xml_txt = R"(
    <root BTCPP_format="4" >
      <BehaviorTree>
        <DefaultTestAction/>
      </BehaviorTree>
    </root>)";

  BehaviorTreeFactory factory;
  factory.registerNodeType<DefaultTestAction>("DefaultTestAction");
  auto tree = factory.createTreeFromText(xml_txt);
  auto status = tree.tickOnce();
  ASSERT_EQ(status, NodeStatus::SUCCESS);
}


class GetAny : public SyncActionNode
{
public:
  GetAny(const std::string& name, const NodeConfig& config) :
      SyncActionNode(name, config)
  {}

  NodeStatus tick() override
  {
    // case 1: the port is Any, but we can cast dirrectly to string
    auto res_str = getInput<std::string>("val_str");
    // case 2: the port is Any, and we retrieve an Any (to be casted later)
    auto res_int = getInput<BT::Any>("val_int");

    // case 3: port is double and we get a double
    auto res_real_A = getInput<double>("val_real");
    // case 4: port is double and we get an Any
    auto res_real_B = getInput<BT::Any>("val_real");

    bool expected = res_str.value() == "hello" &&
                    res_int->cast<int>() == 42 &&
                    res_real_A.value() == 3.14 &&
                    res_real_B->cast<double>() == 3.14;

    return expected ? NodeStatus::SUCCESS : NodeStatus::FAILURE;
  }

  static PortsList providedPorts()
  {
    return {BT::InputPort<BT::Any>("val_str"),
            BT::InputPort<BT::Any>("val_int"),
            BT::InputPort<double>("val_real")};
  }
};

class SetAny : public SyncActionNode
{
public:
  SetAny(const std::string& name, const NodeConfig& config) :
      SyncActionNode(name, config)
  {}

  NodeStatus tick() override
  {
    // check that the port can contain different types
    setOutput("val_str", BT::Any(1.0));
    setOutput("val_str", BT::Any(1));
    setOutput("val_str", BT::Any("hello"));

    setOutput("val_int", 42);
    setOutput("val_real", 3.14);
    return NodeStatus::SUCCESS;
  }

  static PortsList providedPorts()
  {
    return {BT::OutputPort<BT::Any>("val_str"),
            BT::OutputPort<int>("val_int"),
            BT::OutputPort<BT::Any>("val_real")};
  }
};

TEST(PortTest, AnyPort)
{
  std::string xml_txt = R"(
    <root BTCPP_format="4" >
      <BehaviorTree>
        <Sequence>
          <SetAny val_str="{val_str}" val_int="{val_int}" val_real="{val_real}"/>
          <GetAny val_str="{val_str}" val_int="{val_int}" val_real="{val_real}"/>
        </Sequence>
      </BehaviorTree>
    </root>)";

  BehaviorTreeFactory factory;
  factory.registerNodeType<SetAny>("SetAny");
  factory.registerNodeType<GetAny>("GetAny");
  auto tree = factory.createTreeFromText(xml_txt);
  auto status = tree.tickOnce();
  ASSERT_EQ(status, NodeStatus::SUCCESS);
}


