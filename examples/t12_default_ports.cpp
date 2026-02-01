#include "behaviortree_cpp/bt_factory.h"
#include "behaviortree_cpp/json_export.h"

/**
 * The goal of this tutorial is to show all the possible ways
 * that we can define the default value of a port, i.e. the
 * value that it should have if not specified in the XML
 * */

// Custom type. to make things more interesting
struct Point2D
{
  int x = 0;
  int y = 0;
  bool operator==(const Point2D& p) const
  {
    return x == p.x && y == p.y;
  }
  bool operator!=(const Point2D& p) const
  {
    return !(*this == p);
  }
};

// Allow bi-directional conversion to JSON
BT_JSON_CONVERTER(Point2D, point)
{
  add_field("x", &point.x);
  add_field("y", &point.y);
}

// We can extend the traditional BT::convertFromString<Point2D>()
// to support the JSON format too (see port with name "pointE")
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

//-----------------------------------------------
using namespace BT;

class NodeWithDefaultPoints : public SyncActionNode
{
public:
  NodeWithDefaultPoints(const std::string& name, const NodeConfig& config)
    : SyncActionNode(name, config)
  {}

  NodeStatus tick() override
  {
    // Let0s check if all the portas have the expected value
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

int main()
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

  BT::NodeStatus status = tree.tickOnce();
  std::cout << "Result: " << toStr(status) << std::endl;

  return 0;
}
