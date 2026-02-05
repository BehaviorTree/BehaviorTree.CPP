#include "crossdoor_nodes.h"

#include "behaviortree_cpp/bt_factory.h"
#include "behaviortree_cpp/json_export.h"
#include "behaviortree_cpp/loggers/bt_file_logger_v2.h"
#include "behaviortree_cpp/loggers/bt_minitrace_logger.h"
#include "behaviortree_cpp/loggers/groot2_publisher.h"
#include "behaviortree_cpp/xml_parsing.h"

#include <cstdlib>
#include <string>
#include <vector>

/** We are using the same example in Tutorial 5,
 *  But this time we also show how to connect
 */

// A custom struct  that I want to visualize in Groot2
struct Position2D
{
  double x;
  double y;
};

// This macro will generate the code that is needed to convert
// the object to/from JSON.
// You still need to call BT::RegisterJsonDefinition<Position2D>()
// in main()
// NOLINTNEXTLINE(misc-use-internal-linkage)
BT_JSON_CONVERTER(Position2D, pos)
{
  add_field("x", &pos.x);
  add_field("y", &pos.y);
}

struct Waypoint
{
  std::string name;
  Position2D position;
  double speed = 1.0;
};

// NOLINTNEXTLINE(misc-use-internal-linkage)
BT_JSON_CONVERTER(Waypoint, wp)
{
  add_field("name", &wp.name);
  add_field("position", &wp.position);
  add_field("speed", &wp.speed);
}

// Simple Action that updates an instance of Position2D in the blackboard
// and also outputs vectors to reproduce Groot2 issue #55/#77
class UpdatePosition : public BT::SyncActionNode
{
public:
  UpdatePosition(const std::string& name, const BT::NodeConfig& config)
    : BT::SyncActionNode(name, config)
  {}

  BT::NodeStatus tick() override
  {
    _pos.x += 0.2;
    _pos.y += 0.1;
    setOutput("pos", _pos);

    // Vectors that trigger Groot2 issue #55/#77 (crash when viewing blackboard)
    std::vector<double> doubles = { 1.1, 2.2, 3.3, 4.4, 5.5 };
    setOutput("vec_double", doubles);

    std::vector<std::string> strings = { "hello", "world", "test" };
    setOutput("vec_string", strings);

    // Helper to generate random offset [-range, +range]
    auto randOffset = [](double range) {
      // NOLINTNEXTLINE(cert-msc30-c,cert-msc50-cpp,concurrency-mt-unsafe)
      return (static_cast<double>(std::rand()) / RAND_MAX - 0.5) * 2.0 * range;
    };

    // Vector of custom types - alternate between 2 and 3 waypoints
    std::vector<Waypoint> waypoints;
    if(_execution_count % 2 == 0)
    {
      // Even executions: 2 waypoints
      waypoints = {
        { "start", { randOffset(5.0), randOffset(5.0) }, 1.0 + randOffset(0.3) },
        { "goal",
          { 100.0 + randOffset(10.0), 50.0 + randOffset(10.0) },
          0.5 + randOffset(0.2) },
      };
    }
    else
    {
      // Odd executions: 3 waypoints
      waypoints = {
        { "start", { randOffset(5.0), randOffset(5.0) }, 1.0 + randOffset(0.3) },
        { "checkpoint",
          { 50.0 + randOffset(15.0), 25.0 + randOffset(15.0) },
          2.0 + randOffset(0.5) },
        { "goal",
          { 100.0 + randOffset(10.0), 50.0 + randOffset(10.0) },
          0.5 + randOffset(0.2) },
      };
    }
    setOutput("waypoints", waypoints);
    _execution_count++;

    return BT::NodeStatus::SUCCESS;
  }

  static BT::PortsList providedPorts()
  {
    return {
      BT::OutputPort<Position2D>("pos"),
      BT::OutputPort<std::vector<double>>("vec_double", "Vector of doubles"),
      BT::OutputPort<std::vector<std::string>>("vec_string", "Vector of strings"),
      BT::OutputPort<std::vector<Waypoint>>("waypoints", "Vector of waypoints"),
    };
  }

private:
  Position2D _pos = { 0, 0 };
  int _execution_count = 0;
};

// clang-format off
namespace
{
const char* xml_text = R"(
<root BTCPP_format="4">

  <BehaviorTree ID="MainTree">
    <Sequence>
      <Script code="door_open:=false" />
      <UpdatePosition pos="{pos_2D}" vec_double="{doubles}" vec_string="{strings}" waypoints="{waypoints}"/>
      <Fallback>
        <Inverter>
          <IsDoorClosed/>
        </Inverter>
        <SubTree ID="DoorClosed" _autoremap="true" door_open="{door_open}"/>
      </Fallback>
      <PassThroughDoor/>
    </Sequence>
  </BehaviorTree>

  <BehaviorTree ID="DoorClosed">
    <Fallback name="tryOpen" _onSuccess="door_open:=true">
      <OpenDoor/>
        <RetryUntilSuccessful num_attempts="5">
          <PickLock/>
        </RetryUntilSuccessful>
      <SmashDoor/>
    </Fallback>
  </BehaviorTree>

</root>
 )";
}  // namespace
// clang-format on

int main()
{
  BT::BehaviorTreeFactory factory;

  // Nodes registration, as usual
  CrossDoor cross_door;
  cross_door.registerNodes(factory);
  factory.registerNodeType<UpdatePosition>("UpdatePosition");

  // Groot2 editor requires a model of your registered Nodes.
  // You don't need to write that by hand, it can be automatically
  // generated using the following command.
  [[maybe_unused]] const std::string xml_models = BT::writeTreeNodesModelXML(factory);

  factory.registerBehaviorTreeFromText(xml_text);

  // Add this to allow Groot2 to visualize your custom type
  BT::RegisterJsonDefinition<Position2D>();
  BT::RegisterJsonDefinition<Waypoint>();

  auto tree = factory.createTree("MainTree");

  std::cout << "----------- XML file  ----------\n"
            << BT::WriteTreeToXML(tree, false, false)
            << "--------------------------------\n";

  // Connect the Groot2Publisher. This will allow Groot2 to
  // get the tree and poll status updates.
  const unsigned port = 1667;
  BT::Groot2Publisher publisher(tree, port);

  // Add two more loggers, to save the transitions into a file.
  // Both formats are compatible with Groot2

  // Logging with lightweight serialization
  BT::FileLogger2 logger2(tree, "t11_groot_howto.btlog");
  BT::MinitraceLogger minilog(tree, "minitrace.json");

  while(true)
  {
    std::cout << "Start" << std::endl;
    cross_door.reset();
    tree.tickWhileRunning();
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
  }

  return 0;
}
