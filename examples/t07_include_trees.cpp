#include "behaviortree_cpp/xml_parsing.h"
#include "behaviortree_cpp/blackboard/blackboard_local.h"
#include "dummy_nodes.h"

using namespace BT;


int main(int argc, char** argv)
{
    BehaviorTreeFactory factory;
    DummyNodes::RegisterNodes(factory);

    if( argc != 2)
    {
        std::cout <<" missing name of the XML file to open" << std::endl;
        return 1;
    }

    // IMPORTANT: when the object tree goes out of scope, all the TreeNodes are destroyed
    auto tree = buildTreeFromFile(factory, argv[1]);

    printTreeRecursively( tree.root_node );

    //TODO std::cout << writeXML(factory, tree.root_node, true) << std::endl;
    std::cout <<"-----------------------" << std::endl;

    tree.root_node->executeTick();

    return 0;
}
