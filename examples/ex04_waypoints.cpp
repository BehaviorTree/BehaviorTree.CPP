#include "behaviortree_cpp_v3/bt_factory.h"
#include "behaviortree_cpp_v3/actions/pop_from_queue.hpp"
#include "behaviortree_cpp_v3/decorators/consume_queue.h"
#include <list>

using namespace BT;

/*
 * In this example we will show how a common design pattern could be implemented.
 * We want to iterate through the elements of a queue, for instance a list of waypoints.
 *
 * Two ways to create a "loop" are presented, one using the actions "QueueSize" and "PopFromQueue"
 * and the other using the decorator "ConsumeQueue".
 */

struct Pose2D
{
  double x, y, theta;
};

/**
 * @brief Dummy action that generates a list of poses.
 */
class GenerateWaypoints : public SyncActionNode
{
public:
  GenerateWaypoints(const std::string& name, const NodeConfiguration& config) :
    SyncActionNode(name, config)
  {}

  NodeStatus tick() override
  {
    auto queue = std::make_shared<ProtectedQueue<Pose2D>>();
    for (int i = 0; i < 10; i++)
    {
      queue->items.push_back(Pose2D{double(i), double(i), 0});
    }
    setOutput("waypoints", queue);
    return NodeStatus::SUCCESS;
  }

  static PortsList providedPorts()
  {
    return {OutputPort<std::shared_ptr<ProtectedQueue<Pose2D>>>("waypoints")};
  }
};
//--------------------------------------------------------------
class UseWaypointQueue : public AsyncActionNode
{
public:
  UseWaypointQueue(const std::string& name, const NodeConfiguration& config) :
    AsyncActionNode(name, config)
  {}

  NodeStatus tick() override
  {
    std::shared_ptr<ProtectedQueue<Pose2D>> queue;
    if (getInput("waypoints", queue) && queue)
    {
      Pose2D wp;
      {
        // Since we are using reference semantic (the queue is wrapped in
        // a shared_ptr) to modify the queue inside the blackboard,
        // we are effectively bypassing the thread safety of the BB.
        // This is the reason why we need to use a mutex explicitly.
        std::unique_lock<std::mutex> lk(queue->mtx);

        auto& waypoints = queue->items;
        if (waypoints.empty())
        {
          return NodeStatus::FAILURE;
        }
        wp = waypoints.front();
        waypoints.pop_front();

      }   // end mutex lock

      std::this_thread::sleep_for(std::chrono::milliseconds(100));
      std::cout << "Using waypoint: " << wp.x << "/" << wp.y << std::endl;

      return NodeStatus::SUCCESS;
    }
    else
    {
      return NodeStatus::FAILURE;
    }
  }

  static PortsList providedPorts()
  {
    return {InputPort<std::shared_ptr<ProtectedQueue<Pose2D>>>("waypoints")};
  }
};

/**
 * @brief Simple Action that uses the output of PopFromQueue<Pose2D> or ConsumeQueue<Pose2D>
 */
class UseWaypoint : public AsyncActionNode
{
public:
  UseWaypoint(const std::string& name, const NodeConfiguration& config) :
    AsyncActionNode(name, config)
  {}

  NodeStatus tick() override
  {
    Pose2D wp;
    if (getInput("waypoint", wp))
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
      std::cout << "Using waypoint: " << wp.x << "/" << wp.y << std::endl;
      return NodeStatus::SUCCESS;
    }
    else
    {
      return NodeStatus::FAILURE;
    }
  }

  static PortsList providedPorts()
  {
    return {InputPort<Pose2D>("waypoint")};
  }
};

// clang-format off

static const char* xml_implicit = R"(
 <root main_tree_to_execute = "TreeImplicit" >
     <BehaviorTree ID="TreeImplicit">
        <Sequence>
            <GenerateWaypoints waypoints="{waypoints}" />
            <KeepRunningUntilFailure>
                <UseWaypointQueue waypoints="{waypoints}" />
            </KeepRunningUntilFailure>
        </Sequence>
     </BehaviorTree>
 </root>
 )";


static const char* xml_A = R"(
 <root main_tree_to_execute = "TreeA" >
     <BehaviorTree ID="TreeA">
        <Sequence>
            <GenerateWaypoints waypoints="{waypoints}" />
            <QueueSize queue="{waypoints}" size="{wp_size}" />
            <Repeat num_cycles="{wp_size}" >
                <Sequence>
                    <PopFromQueue  queue="{waypoints}" popped_item="{wp}" />
                    <UseWaypoint waypoint="{wp}" />
                </Sequence>
            </Repeat>
        </Sequence>
     </BehaviorTree>
 </root>
 )";

static const char* xml_B = R"(
 <root main_tree_to_execute = "TreeB" >
     <BehaviorTree ID="TreeB">
        <Sequence>
            <GenerateWaypoints waypoints="{waypoints}" />
            <ConsumeQueue queue="{waypoints}" popped_item="{wp}">
                <UseWaypoint waypoint="{wp}" />
            </ConsumeQueue>
        </Sequence>
     </BehaviorTree>
 </root>
 )";

// clang-format on

int main()
{
  BehaviorTreeFactory factory;

  factory.registerNodeType<PopFromQueue<Pose2D>>("PopFromQueue");
  factory.registerNodeType<QueueSize<Pose2D>>("QueueSize");
  factory.registerNodeType<ConsumeQueue<Pose2D>>("ConsumeQueue");

  factory.registerNodeType<UseWaypoint>("UseWaypoint");
  factory.registerNodeType<UseWaypointQueue>("UseWaypointQueue");
  factory.registerNodeType<GenerateWaypoints>("GenerateWaypoints");

  for (const auto& xml_text : {xml_implicit, xml_A, xml_B})
  {
    auto tree = factory.createTreeFromText(xml_text);
    while (tree.tickRoot() == NodeStatus::RUNNING)
    {
      tree.sleep(std::chrono::milliseconds(10));
    }
    std::cout << "--------------" << std::endl;
  }

  return 0;
}
