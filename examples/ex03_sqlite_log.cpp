#include "dummy_nodes.h"
#include "behaviortree_cpp/bt_factory.h"
#include "behaviortree_cpp/loggers/bt_sqlite_logger.h"
#include "behaviortree_cpp/xml_parsing.h"

struct TaskA
{
  int type;
  std::string name;
};

struct TaskB
{
  double value;
  std::string name;
};

using Command = std::variant<TaskA, TaskB>;

// Simple Action that updates an instance of Position2D in the blackboard
class SetTask : public BT::SyncActionNode
{
public:
  SetTask(const std::string& name, const BT::NodeConfig& config)
    : BT::SyncActionNode(name, config)
  {}

  BT::NodeStatus tick() override
  {
    auto type = getInput<std::string>("type").value();
    if(type == "A")
    {
      setOutput<Command>("task", TaskA{ 43, type });
    }
    else if(type == "B")
    {
      setOutput<Command>("task", TaskB{ 3.14, type });
    }
    return BT::NodeStatus::SUCCESS;
  }

  static BT::PortsList providedPorts()
  {
    return { BT::InputPort<std::string>("type"), BT::OutputPort<Command>("task") };
  }

private:
};

// clang-format off

static const char* xml_text = R"(
<root BTCPP_format="4">

  <BehaviorTree ID="MainTree">
    <Sequence>

      <Script code="type:='A'" />
      <SetTask type="{type}" task="{task}" />
      <SubTree ID="ExecuteTaskA" task="{task}" _skipIf=" type!='A' " />

      <Script code="type:='B'" />
      <SetTask type="{type}" task="{task}" />
      <SubTree ID="ExecuteTaskB" task="{task}" _skipIf=" type!='B' " />

    </Sequence>
  </BehaviorTree>

  <BehaviorTree ID="ExecuteTaskA">
    <Sequence>
      <Sleep msec="1000"/>
      <SaySomething message="executed command A" />
    </Sequence>
  </BehaviorTree>

  <BehaviorTree ID="ExecuteTaskB">
    <Sequence>
      <Sleep msec="1000"/>
      <SaySomething message="executed command B" />
    </Sequence>
  </BehaviorTree>

</root>
 )";

// clang-format on

int main()
{
  BT::BehaviorTreeFactory factory;

  // Nodes registration, as usual
  factory.registerNodeType<DummyNodes::SaySomething>("SaySomething");
  factory.registerNodeType<SetTask>("SetTask");

  // Groot2 editor requires a model of your registered Nodes.
  // You don't need to write that by hand, it can be automatically
  // generated using the following command.
  std::string xml_models = BT::writeTreeNodesModelXML(factory);

  factory.registerBehaviorTreeFromText(xml_text);

  auto tree = factory.createTree("MainTree");

  std::cout << "----------- XML file  ----------\n"
            << BT::WriteTreeToXML(tree, false, false)
            << "--------------------------------\n";

  BT::SqliteLogger sqlite_logger(tree, "ex08_sqlitelog.db3", false);

  //------------------------------------------------------------------------
  // Write some data (from the blackboard) and write into the
  // extra column called "extra_data". We will use JSON serialization

  auto meta_callback = [&](BT::Duration timestamp, const BT::TreeNode& node,
                           BT::NodeStatus prev_status,
                           BT::NodeStatus status) -> std::string {
    if(prev_status == BT::NodeStatus::RUNNING && BT::isStatusCompleted(status))
    {
      if(node.name() == "ExecuteTaskA")
      {
        auto task = node.config().blackboard->get<Command>("task");
        auto taskA = std::get<TaskA>(task);
        nlohmann::json json;
        json["taskA"] = { { "name", taskA.name }, { "type", taskA.type } };
        return json.dump();
      }
      if(node.name() == "ExecuteTaskB")
      {
        auto task = node.config().blackboard->get<Command>("task");
        auto taskB = std::get<TaskB>(task);
        nlohmann::json json;
        json["taskB"] = { { "name", taskB.name }, { "value", taskB.value } };
        return json.dump();
      }
    }
    return {};
  };
  sqlite_logger.setAdditionalCallback(meta_callback);
  //------------------------------------------------------------------------
  while(1)
  {
    std::cout << "Start" << std::endl;
    tree.tickWhileRunning();
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
  }

  return 0;
}
