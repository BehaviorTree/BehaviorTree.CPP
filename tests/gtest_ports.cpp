#include "behaviortree_cpp/basic_types.h"
#include "behaviortree_cpp/bt_factory.h"
#include "behaviortree_cpp/json_export.h"
#include "behaviortree_cpp/xml_parsing.h"

#include <gtest/gtest.h>

using namespace BT;

class NodeWithPorts : public SyncActionNode
{
public:
  NodeWithPorts(const std::string& name, const NodeConfig& config)
    : SyncActionNode(name, config)
  {
    std::cout << "ctor" << std::endl;
  }

  NodeStatus tick() override
  {
    int val_A = 0;
    int val_B = 0;
    if(!getInput("in_port_A", val_A))
    {
      throw RuntimeError("missing input [in_port_A]");
    }
    if(!getInput("in_port_B", val_B))
    {
      throw RuntimeError("missing input [in_port_B]");
    }

    if(val_A == 42 && val_B == 66)
    {
      return NodeStatus::SUCCESS;
    }
    return NodeStatus::FAILURE;
  }

  static PortsList providedPorts()
  {
    return { BT::InputPort<int>("in_port_A", 42, "magic_number"),
             BT::InputPort<int>("in_port_B") };
  }
};

TEST(PortTest, WrongNodeConfig)
{
  NodeConfig config;
  config.input_ports["in_port_A"] = "42";
  // intentionally missing:
  // config.input_ports["in_port_B"] = "69";
  NodeWithPorts node("will_fail", config);
  ASSERT_ANY_THROW(node.tick());
}

TEST(PortTest, DefaultPorts)
{
  std::string xml_txt = R"(
    <root BTCPP_format="4" >
        <BehaviorTree ID="MainTree">
            <NodeWithPorts in_port_B="66" />
        </BehaviorTree>
    </root>)";

  BehaviorTreeFactory factory;
  factory.registerNodeType<NodeWithPorts>("NodeWithPorts");
  auto tree = factory.createTreeFromText(xml_txt);
  NodeStatus status = tree.tickWhileRunning();
  ASSERT_EQ(status, NodeStatus::SUCCESS);
}

TEST(PortTest, MissingPort)
{
  std::string xml_txt = R"(
    <root BTCPP_format="4" >
        <BehaviorTree ID="MainTree">
            <NodeWithPorts/>
        </BehaviorTree>
    </root>)";

  BehaviorTreeFactory factory;
  factory.registerNodeType<NodeWithPorts>("NodeWithPorts");
  auto tree = factory.createTreeFromText(xml_txt);
  ASSERT_ANY_THROW(tree.tickWhileRunning());
}

TEST(PortTest, WrongPort)
{
  std::string xml_txt = R"(
    <root BTCPP_format="4" >
        <BehaviorTree ID="MainTree">
            <NodeWithPorts da_port="66" />
        </BehaviorTree>
    </root>)";

  BehaviorTreeFactory factory;
  factory.registerNodeType<NodeWithPorts>("NodeWithPorts");

  EXPECT_ANY_THROW(auto tree = factory.createTreeFromText(xml_txt));
}

TEST(PortTest, Descriptions)
{
  std::string xml_txt = R"(
    <root BTCPP_format="4" >
        <BehaviorTree ID="MainTree" _description="this is my tree" >
            <Sequence>
                <NodeWithPorts name="first"  in_port_B="66" _description="this is my action" />
                <SubTree ID="mySubTree" name="second" _description="this is a subtree"/>
            </Sequence>
        </BehaviorTree>

        <BehaviorTree ID="mySubTree" _description="this is a subtree" >
            <NodeWithPorts name="third" in_port_B="99" />
        </BehaviorTree>

    </root>)";

  BehaviorTreeFactory factory;
  factory.registerNodeType<NodeWithPorts>("NodeWithPorts");

  factory.registerBehaviorTreeFromText(xml_txt);
  auto tree = factory.createTree("MainTree");

  NodeStatus status = tree.tickWhileRunning();
  while(status == NodeStatus::RUNNING)
  {
    status = tree.tickWhileRunning();
  }

  ASSERT_EQ(status, NodeStatus::FAILURE);  // failure because in_port_B="99"
}

TEST(PortsTest, NonPorts)
{
  std::string xml_txt =
      R"(
    <root BTCPP_format="4" >
        <BehaviorTree ID="MainTree">
            <Action ID="NodeWithPorts" name="NodeWithPortsName" in_port_B="66" _not_da_port="whateva" _skipIf="true" />
        </BehaviorTree>
    </root>)";

  BehaviorTreeFactory factory;
  factory.registerNodeType<NodeWithPorts>("NodeWithPorts");

  auto tree = factory.createTreeFromText(xml_txt);

  const TreeNode* root = tree.rootNode();
  ASSERT_NE(root, nullptr);
  ASSERT_EQ(root->type(), NodeType::ACTION);

  EXPECT_EQ(root->config().other_attributes.size(), 1);
  ASSERT_EQ(root->config().other_attributes.count("_not_da_port"), 1);
  EXPECT_EQ(root->config().other_attributes.at("_not_da_port"), "whateva");
}

struct MyType
{
  std::string value;
};

class NodeInPorts : public SyncActionNode
{
public:
  NodeInPorts(const std::string& name, const NodeConfig& config)
    : SyncActionNode(name, config)
  {}

  NodeStatus tick() override
  {
    int val_A = 0;
    MyType val_B;
    if(getInput("int_port", val_A) && getInput("any_port", val_B))
    {
      return NodeStatus::SUCCESS;
    }
    return NodeStatus::FAILURE;
  }

  static PortsList providedPorts()
  {
    return { BT::InputPort<int>("int_port"), BT::InputPort<MyType>("any_port") };
  }
};

class NodeOutPorts : public SyncActionNode
{
public:
  NodeOutPorts(const std::string& name, const NodeConfig& config)
    : SyncActionNode(name, config)
  {}

  NodeStatus tick() override
  {
    return NodeStatus::SUCCESS;
  }

  static PortsList providedPorts()
  {
    return { BT::OutputPort<int>("int_port"), BT::OutputPort<MyType>("any_port") };
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

class ActionVectorStringIn : public SyncActionNode
{
public:
  ActionVectorStringIn(const std::string& name, const NodeConfig& config,
                       std::vector<std::string>* states)
    : SyncActionNode(name, config), states_(states)
  {}

  NodeStatus tick() override
  {
    getInput("states", *states_);
    return NodeStatus::SUCCESS;
  }

  static PortsList providedPorts()
  {
    return { BT::InputPort<std::vector<std::string>>("states") };
  }

private:
  std::vector<std::string>* states_;
};

TEST(PortTest, SubtreeStringInput_StringVector)
{
  std::string xml_txt = R"(
    <root BTCPP_format="4" >
      <BehaviorTree ID="Main">
        <ActionVectorStringIn states="hello;world;with spaces"/>
      </BehaviorTree>
    </root>)";

  std::vector<std::string> states;

  BehaviorTreeFactory factory;
  factory.registerNodeType<ActionVectorStringIn>("ActionVectorStringIn", &states);

  factory.registerBehaviorTreeFromText(xml_txt);
  auto tree = factory.createTree("Main");

  NodeStatus status = tree.tickWhileRunning();

  ASSERT_EQ(status, NodeStatus::SUCCESS);
  ASSERT_EQ(3, states.size());
  ASSERT_EQ("hello", states[0]);
  ASSERT_EQ("world", states[1]);
  ASSERT_EQ("with spaces", states[2]);
}

struct Point2D
{
  int x = 0;
  int y = 0;
  bool operator==(const Point2D& other) const
  {
    return x == other.x && y == other.y;
  }
  bool operator!=(const Point2D& other) const
  {
    return !(*this == other);
  }
};

template <>
[[nodiscard]] Point2D BT::convertFromString<Point2D>(StringView str)
{
  if(StartWith(str, "json:"))
  {
    str.remove_prefix(5);
    return convertFromJSON<Point2D>(str);
  }
  const auto parts = BT::splitString(str, ',');
  if(parts.size() != 2)
  {
    throw BT::RuntimeError("invalid input)");
  }
  int x = convertFromString<int>(parts[0]);
  int y = convertFromString<int>(parts[1]);
  return { x, y };
}

template <>
[[nodiscard]] std::string BT::toStr<Point2D>(const Point2D& point)
{
  return std::to_string(point.x) + "," + std::to_string(point.y);
}

BT_JSON_CONVERTER(Point2D, point)
{
  add_field("x", &point.x);
  add_field("y", &point.y);
}

class DefaultTestAction : public SyncActionNode
{
public:
  DefaultTestAction(const std::string& name, const NodeConfig& config)
    : SyncActionNode(name, config)
  {}

  NodeStatus tick() override
  {
    const int answer = getInput<int>("answer").value();
    if(answer != 42)
    {
      return NodeStatus::FAILURE;
    }

    const std::string greet = getInput<std::string>("greeting").value();
    if(greet != "hello")
    {
      return NodeStatus::FAILURE;
    }

    const Point2D point = getInput<Point2D>("pos").value();
    if(point.x != 1 || point.y != 2)
    {
      return NodeStatus::FAILURE;
    }

    return NodeStatus::SUCCESS;
  }

  static PortsList providedPorts()
  {
    return { BT::InputPort<int>("answer", 42, "the answer"),
             BT::InputPort<std::string>("greeting", "hello", "be polite"),
             BT::InputPort<Point2D>("pos", Point2D{ 1, 2 }, "where") };
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

// NOTE: GetAny, SetAny classes and AnyPort test moved to gtest_port_type_rules.cpp

class NodeWithDefaultPoints : public SyncActionNode
{
public:
  NodeWithDefaultPoints(const std::string& name, const NodeConfig& config)
    : SyncActionNode(name, config)
  {}

  NodeStatus tick() override
  {
    Point2D pointA, pointB, pointC, pointD, pointE, input;

    if(!getInput("pointA", pointA) || pointA != Point2D{ 1, 2 })
    {
      throw std::runtime_error("failed pointA");
    }
    if(!getInput("pointB", pointB) || pointB != Point2D{ 3, 4 })
    {
      throw std::runtime_error("failed pointB");
    }
    if(!getInput("pointC", pointC) || pointC != Point2D{ 5, 6 })
    {
      throw std::runtime_error("failed pointC");
    }
    if(!getInput("pointD", pointD) || pointD != Point2D{ 7, 8 })
    {
      throw std::runtime_error("failed pointD");
    }
    if(!getInput("pointE", pointE) || pointE != Point2D{ 9, 10 })
    {
      throw std::runtime_error("failed pointE");
    }
    if(!getInput("input", input) || input != Point2D{ -1, -2 })
    {
      throw std::runtime_error("failed input");
    }
    return NodeStatus::SUCCESS;
  }

  static PortsList providedPorts()
  {
    return { BT::InputPort<Point2D>("input", "no default value"),
             BT::InputPort<Point2D>("pointA", Point2D{ 1, 2 }, "default value is [1,2]"),
             BT::InputPort<Point2D>("pointB", "{point}",
                                    "default value inside blackboard {point}"),
             BT::InputPort<Point2D>("pointC", "5,6", "default value is [5,6]"),
             BT::InputPort<Point2D>("pointD", "{=}",
                                    "default value inside blackboard {pointD}"),
             BT::InputPort<Point2D>("pointE", R"(json:{"x":9,"y":10})",
                                    "default value is [9,10]") };
  }
};

TEST(PortTest, DefaultInputPoint2D)
{
  std::string xml_txt = R"(
    <root BTCPP_format="4" >
      <BehaviorTree>
        <NodeWithDefaultPoints input="-1,-2"/>
      </BehaviorTree>
    </root>)";

  JsonExporter::get().addConverter<Point2D>();

  BehaviorTreeFactory factory;
  factory.registerNodeType<NodeWithDefaultPoints>("NodeWithDefaultPoints");
  auto tree = factory.createTreeFromText(xml_txt);

  tree.subtrees.front()->blackboard->set<Point2D>("point", Point2D{ 3, 4 });
  tree.subtrees.front()->blackboard->set<Point2D>("pointD", Point2D{ 7, 8 });

  BT::NodeStatus status;
  ASSERT_NO_THROW(status = tree.tickOnce());
  ASSERT_EQ(status, NodeStatus::SUCCESS);

  std::cout << writeTreeNodesModelXML(factory) << std::endl;
}

class NodeWithDefaultStrings : public SyncActionNode
{
public:
  NodeWithDefaultStrings(const std::string& name, const NodeConfig& config)
    : SyncActionNode(name, config)
  {}

  NodeStatus tick() override
  {
    std::string input, msgA, msgB, msgC;
    if(!getInput("input", input) || input != "from XML")
    {
      throw std::runtime_error("failed input");
    }
    if(!getInput("msgA", msgA) || msgA != "hello")
    {
      throw std::runtime_error("failed msgA");
    }
    if(!getInput("msgB", msgB) || msgB != "ciao")
    {
      throw std::runtime_error("failed msgB");
    }
    if(!getInput("msgC", msgC) || msgC != "hola")
    {
      throw std::runtime_error("failed msgC");
    }
    return NodeStatus::SUCCESS;
  }

  static PortsList providedPorts()
  {
    return { BT::InputPort<std::string>("input", "no default"),
             BT::InputPort<std::string>("msgA", "hello", "default value is 'hello'"),
             BT::InputPort<std::string>("msgB", "{msg}",
                                        "default value inside blackboard {msg}"),
             BT::InputPort<std::string>("msgC", "{=}",
                                        "default value inside blackboard {msgC}") };
  }
};

TEST(PortTest, DefaultInputStrings)
{
  std::string xml_txt = R"(
    <root BTCPP_format="4" >
      <BehaviorTree>
        <NodeWithDefaultStrings input="from XML"/>
      </BehaviorTree>
    </root>)";

  BehaviorTreeFactory factory;
  factory.registerNodeType<NodeWithDefaultStrings>("NodeWithDefaultStrings");
  auto tree = factory.createTreeFromText(xml_txt);

  tree.subtrees.front()->blackboard->set<std::string>("msg", "ciao");
  tree.subtrees.front()->blackboard->set<std::string>("msgC", "hola");

  BT::NodeStatus status;
  ASSERT_NO_THROW(status = tree.tickOnce());
  ASSERT_EQ(status, NodeStatus::SUCCESS);

  std::cout << writeTreeNodesModelXML(factory) << std::endl;
}

struct TestStruct
{
  int a;
  double b;
  std::string c;
};

class NodeWithDefaultNullptr : public SyncActionNode
{
public:
  NodeWithDefaultNullptr(const std::string& name, const NodeConfig& config)
    : SyncActionNode(name, config)
  {}

  NodeStatus tick() override
  {
    return NodeStatus::SUCCESS;
  }

  static PortsList providedPorts()
  {
    return { BT::InputPort<std::shared_ptr<TestStruct>>("input", nullptr,
                                                        "default value is nullptr") };
  }
};

TEST(PortTest, Default_Issues_767)
{
  using namespace BT;

  ASSERT_NO_THROW(auto p = InputPort<std::optional<Point2D>>("opt_A", std::nullopt,
                                                             "default nullopt"));
  ASSERT_NO_THROW(auto p = InputPort<std::optional<std::string>>("opt_B", std::nullopt,
                                                                 "default nullopt"));

  ASSERT_NO_THROW(
      auto p = InputPort<std::shared_ptr<Point2D>>("ptr_A", nullptr, "default nullptr"));
  ASSERT_NO_THROW(auto p = InputPort<std::shared_ptr<std::string>>("ptr_B", nullptr,
                                                                   "default nullptr"));
}

TEST(PortTest, DefaultWronglyOverriden)
{
  BT::BehaviorTreeFactory factory;
  factory.registerNodeType<NodeWithDefaultNullptr>("NodeWithDefaultNullptr");

  std::string xml_txt_wrong = R"(
    <root BTCPP_format="4" >
      <BehaviorTree>
        <NodeWithDefaultNullptr input=""/>
      </BehaviorTree>
    </root>)";

  std::string xml_txt_correct = R"(
    <root BTCPP_format="4" >
      <BehaviorTree>
        <NodeWithDefaultNullptr/>
      </BehaviorTree>
    </root>)";

  // this should throw, because we are NOT using the default,
  // but overriding it with an empty string instead.
  // See issue 768 for reference
  ASSERT_ANY_THROW(auto tree = factory.createTreeFromText(xml_txt_wrong));
  // This is correct
  ASSERT_NO_THROW(auto tree = factory.createTreeFromText(xml_txt_correct));
}
