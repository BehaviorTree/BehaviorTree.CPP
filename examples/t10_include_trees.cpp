#include "behaviortree_cpp_v3/bt_factory.h"
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

    // IMPORTANT: when the object tree goes out of scope,
    // all the TreeNodes are destroyed
    auto tree = factory.createTreeFromFile(argv[1]);

    printTreeRecursively( tree.rootNode() );

    tree.tickRoot();

    return 0;
}
