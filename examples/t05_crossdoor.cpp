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
            <RetryUntilSuccesful num_attempts="4">
                <OpenDoor/>
            </RetryUntilSuccesful>
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

    // register all the actions into the factory
    CrossDoor::RegisterNodes(factory);

    // Important: when the object tree goes out of scope, all the TreeNodes are destroyed
    auto tree = factory.createTreeFromText(xml_text);

    // Create some loggers
    StdCoutLogger logger_cout(tree);
    MinitraceLogger logger_minitrace(tree, "bt_trace.json");
    FileLogger logger_file(tree, "bt_trace.fbl");
#ifdef ZMQ_FOUND
    PublisherZMQ publisher_zmq(tree);
#endif

    printTreeRecursively(tree.root_node);

    const bool LOOP = ( argc == 2 && strcmp( argv[1], "loop") == 0);

    do
    {
        NodeStatus status = NodeStatus::RUNNING;
        // Keep on ticking until you get either a SUCCESS or FAILURE state
        while( status == NodeStatus::RUNNING)
        {
            status = tree.root_node->executeTick();
            CrossDoor::SleepMS(1);   // optional sleep to avoid "busy loops"
        }
        CrossDoor::SleepMS(1000);
    }
    while(LOOP);

    return 0;
}
