#include "crossdoor_dummy_nodes.h"
#include "behavior_tree_core/xml_parsing.h"
#include "behavior_tree_logger/bt_cout_logger.h"

// clang-format off

const std::string xml_text = R"(

                             <root main_tree_to_execute = "MainTree" >

                             <BehaviorTree ID="MainTree">
                             <Fallback name="root_selector">

                             <Sequence name="door_open_sequence">
                             <Action ID="IsDoorOpen" />
                             <Action ID="PassThroughDoor" />
                             </Sequence>

                             <Sequence name="door_closed_sequence">
                             <Decorator ID="Negation">
                             <Action ID="IsDoorOpen" />
                             </Decorator>
                             <Action ID="OpenDoor" />
                             <Action ID="PassThroughDoor" />
                             <Action ID="CloseDoor" />
                             </Sequence>

                             <Action ID="PassThroughWindow" />

                             </Fallback>
                             </BehaviorTree>

                             </root>
                             )";

// clang-format on

int main(int argc, char** argv)
{
    using namespace BT;

    BT::BehaviorTreeFactory factory;

    // register all the actions into the factory
    CrossDoor cross_door(factory);

    XMLParser parser;
    parser.loadFromText(xml_text);

    std::vector<BT::TreeNodePtr> nodes;
    BT::TreeNodePtr root_node = parser.instantiateTree(factory, nodes);

    StdCoutLogger logger(root_node.get());

    cross_door.CloseDoor();

    std::cout << "---------------" << std::endl;
    root_node->executeTick();
    std::cout << "---------------" << std::endl;
    root_node->executeTick();
    std::cout << "---------------" << std::endl;
    return 0;
}
