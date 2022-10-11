#include "crossdoor_nodes.h"

#include "behaviortree_cpp_v3/loggers/bt_cout_logger.h"
#include "behaviortree_cpp_v3/loggers/bt_minitrace_logger.h"
#include "behaviortree_cpp_v3/loggers/bt_file_logger.h"
#include "behaviortree_cpp_v3/bt_factory.h"

#ifdef ZMQ_FOUND
#include "behaviortree_cpp_v3/loggers/bt_zmq_publisher.h"
#endif

/** This is a more complex example that uses Fallback,
 * Decorators and Subtrees
 *
 * For the sake of simplicity, we aren't focusing on ports remapping to the time being.
 *
 * Furthermore, we introduce Loggers, which are a mechanism to
 * trace the state transitions in the tree for debugging purposes.
 */

// clang-format off

static const char* xml_text = R"(
<root main_tree_to_execute = "MainTree">
	<!--------------------------------------->
    <BehaviorTree ID="DoorClosed">
        <Sequence name="door_closed_sequence">
            <Inverter>
                <Condition ID="IsDoorOpen"/>
            </Inverter>
            <RetryUntilSuccessful num_attempts="4">
                <OpenDoor/>
            </RetryUntilSuccessful>
            <PassThroughDoor/>
        </Sequence>
    </BehaviorTree>
    <!--------------------------------------->
    <BehaviorTree ID="MainTree">
        <Sequence>
            <Fallback name="root_Fallback">
                <Sequence name="door_open_sequence">
                    <IsDoorOpen/>
                    <PassThroughDoor/>
                </Sequence>
                <SubTree ID="DoorClosed"/>
                <PassThroughWindow/>
            </Fallback>
            <CloseDoor/>
        </Sequence>
    </BehaviorTree>
    <!---------------------------------------> 
</root>
 )";

// clang-format on

using namespace BT;

int main(int argc, char** argv)
{
  BT::BehaviorTreeFactory factory;

  // Register our custom nodes into the factory.
  // Any default nodes provided by the BT library (such as Fallback) are registered by
  // default in the BehaviorTreeFactory.
  CrossDoor::RegisterNodes(factory);

  // Important: when the object tree goes out of scope, all the TreeNodes are destroyed
  auto tree = factory.createTreeFromText(xml_text);

  // This logger prints state changes on console
  StdCoutLogger logger_cout(tree);

  // This logger saves state changes on file
  FileLogger logger_file(tree, "bt_trace.fbl");

  // This logger stores the execution time of each node
  MinitraceLogger logger_minitrace(tree, "bt_trace.json");

#ifdef ZMQ_FOUND
  // This logger publish status changes using ZeroMQ. Used by Groot
  PublisherZMQ publisher_zmq(tree);
#endif

  printTreeRecursively(tree.rootNode());

  const bool LOOP = (argc == 2 && strcmp(argv[1], "loop") == 0);

  NodeStatus status = tree.tickRoot();
  while(LOOP || status == NodeStatus::RUNNING)
  {
    tree.sleep(std::chrono::milliseconds(10));
    status = tree.tickRoot();
  }

  return 0;
}
