#include "crossdoor_dummy_nodes.h"
#include "behavior_tree_core/xml_parsing.h"
#include "behavior_tree_logger/bt_cout_logger.h"
#include "behavior_tree_logger/bt_minitrace_logger.h"
#include "behavior_tree_logger/bt_file_logger.h"

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

int main()
{
    using namespace BT;

    BT::BehaviorTreeFactory factory;

    // register all the actions into the factory
    CrossDoor cross_door(factory, false);

    XMLParser parser(factory);
    parser.loadFromText(xml_text);

    std::vector<BT::TreeNode::Ptr> nodes;
    BT::TreeNode::Ptr root_node = parser.instantiateTree(nodes);

    StdCoutLogger logger_cout(root_node.get());
    MinitraceLogger logger_minitrace(root_node.get(), "bt_trace.json");
    FileLogger logger_file(root_node.get(), "bt_trace.fbl", 32);

#ifdef ZMQ_FOUND
    PublisherZMQ publisher_zmq(root_node.get());
#endif

    cross_door.CloseDoor();

    std::cout << "\n-------\n";
    XMLWriter writer(factory);
    std::cout << writer.writeXML( root_node.get(), false) << std::endl;

    std::cout << "---------------" << std::endl;
    root_node->executeTick();

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    std::cout << "---------------" << std::endl;
    while (1)
        root_node->executeTick();
    std::cout << "---------------" << std::endl;
    return 0;
}
