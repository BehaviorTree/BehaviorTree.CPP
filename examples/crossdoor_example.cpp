#include "crossdoor_dummy_nodes.h"
#include "behavior_tree_core/xml_parsing.h"
#include "behavior_tree_logger/bt_cout_logger.h"
#include "behavior_tree_logger/bt_minitrace_logger.h"
#include "behavior_tree_logger/bt_file_logger.h"
#include "Blackboard/blackboard_local.h"

#ifdef ZMQ_FOUND
#include "behavior_tree_logger/bt_zmq_publisher.h"
#endif

// clang-format off

const std::string xml_text = R"(

 <root main_tree_to_execute = "MainTree" >

     <BehaviorTree ID="MainTree">
         <Fallback name="root_selector">

         <Sequence name="door_open_sequence">
             <IsDoorOpen/>
             <PassThroughDoor/>
         </Sequence>

         <Sequence name="door_closed_sequence">
             <Negation>
                <IsDoorOpen/>
             </Negation>
             <RetryUntilSuccesful num_attempts="2" >
                <OpenDoor/>
             </RetryUntilSuccesful>
             <Action ID="PassThroughDoor" />
             <Action ID="CloseDoor" />
         </Sequence>

         <Action ID="PassThroughWindow" />

         </Fallback>
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

    StdCoutLogger logger_cout(tree.root_node);
    MinitraceLogger logger_minitrace(tree.root_node, "bt_trace.json");
    FileLogger logger_file(tree.root_node, "bt_trace.fbl", 32);
#ifdef ZMQ_FOUND
    PublisherZMQ publisher_zmq(tree.root_node);
#endif

    std::cout << writeXML( factory, tree.root_node, false ) << std::endl;
    std::cout << "---------------" << std::endl;

    // Keep on ticking until you get either a SUCCESS or FAILURE state
    NodeStatus status = NodeStatus::RUNNING;
    while( status == NodeStatus::RUNNING )
    {
        status = tree.root_node->executeTick();
    }
    return 0;
}
