#include "behavior_tree_core/xml_parsing.h"
#include "Blackboard/blackboard_local.h"
#include "dummy_nodes.h"

using namespace BT;

const std::string xml_text = R"(

 <root main_tree_to_execute = "MainTree" >

     <BehaviorTree ID="MainTree">
        <Sequence name="root_sequence">
            <SayHello       name="action_hello"/>
            <ActionOne      name="action_one"/>
            <ActionTwo      name="action_two"/>
            <CustomAction   name="my_action"/>
        </Sequence>
     </BehaviorTree>

 </root>
 )";

int main()
{
    BehaviorTreeFactory factory;

    Foo foo;
    // This is the syntax to register SimpleActionNodes
    factory.registerSimpleAction("SayHello", std::bind(SayHello) );
    factory.registerSimpleAction("ActionOne", std::bind( &Foo::actionOne, &foo));
    factory.registerSimpleAction("ActionTwo", std::bind( &Foo::actionTwo, &foo));

    // If you want to register a class that inherits from a TReeNode, use this method instead
    factory.registerNodeType<CustomAction>("CustomAction");

    // IMPORTANT: when the object tree goes out of scope, all the TreeNodes are destroyed
    auto tree = buildTreeFromText(factory, xml_text);
    const TreeNode::Ptr& root_node = tree.first;

    // The tick is propagated to all the children.
    // until one of the returns FAILURE or RUNNING.
    // In this case all of the return SUCCESS
    root_node->executeTick();

    return 0;
}
