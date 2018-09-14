#include "crossdoor_dummy_nodes.h"
#include "behavior_tree_core/xml_parsing.h"
#include "behavior_tree_logger/bt_cout_logger.h"
#include "behavior_tree_logger/bt_minitrace_logger.h"
#include "behavior_tree_logger/bt_file_logger.h"
#include "Blackboard/blackboard_local.h"
#include "behavior_tree_logger/bt_zmq_publisher.h"

// clang-format off

const std::string xml_text = R"(

<root main_tree_to_execute = "MainTree" >

    <BehaviorTree ID="MainTree">
        <FallbackStar name="root_selector">

            <Sequence name="door_open_sequence">
                <Condition ID="IsDoorOpen"/>
                <Action ID="PassThroughDoor"/>
            </Sequence>

            <SequenceStar name="door_closed_sequence">
                <Negation>
                    <Condition ID="IsDoorOpen"/>
                </Negation>
                <RetryUntilSuccesful num_attempts="2" >
                    <Action ID="OpenDoor"/>
                </RetryUntilSuccesful>
                <Action ID="PassThroughDoor" />
                <Action ID="CloseDoor" />
            </SequenceStar>

        <Action ID="PassThroughWindow" />

        </FallbackStar>
    </BehaviorTree>

</root>
 )";

// clang-format on

using namespace BT;

int main()
{
    BT::BehaviorTreeFactory factory;

    auto blackboard = Blackboard::create<BlackboardLocal>();
    blackboard->set("door_open", false);
    blackboard->set("door_locked", true);

    // register all the actions into the factory
    CrossDoor::RegisterNodes(factory);

    // Important: when the object tree goes out of scope, all the TreeNodes are destroyed
    auto tree = buildTreeFromText(factory, xml_text, blackboard);

    // Create some loggers
    StdCoutLogger   logger_cout(tree.root_node);
    MinitraceLogger logger_minitrace(tree.root_node, "bt_trace.json");
    FileLogger      logger_file(tree.root_node, "bt_trace.fbl");
    PublisherZMQ    publisher_zmq(tree.root_node);

    // Keep on ticking until you get either a SUCCESS or FAILURE state
    NodeStatus status = NodeStatus::RUNNING;
    while( status == NodeStatus::RUNNING )
    {
        status = tree.root_node->executeTick();
    }
    return 0;
}
