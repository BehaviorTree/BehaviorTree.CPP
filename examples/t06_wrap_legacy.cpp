#include "behaviortree_cpp/xml_parsing.h"
#include "behaviortree_cpp/loggers/bt_cout_logger.h"
#include "behaviortree_cpp/blackboard/blackboard_local.h"


/** In this tutorial we will see how to wrap legacy code into a
 * BehaviorTree in a non-intrusive way, i.e. without modifying the
 * original class
*/

// clang-format off
const std::string xml_text = R"(

 <root main_tree_to_execute = "MainTree" >
     <BehaviorTree ID="MainTree">
        <SequenceStar name="root">
            <MoveTo  goal="-1;3;0.5" />
            <MoveTo  goal="{myGoal}" />
        </SequenceStar>
     </BehaviorTree>
 </root>
 )";

// clang-format on

// This is my custom type. We won't know how to read this from a string,
// unless we implement convertFromString<Point3D>()
struct Point3D { double x,y,z; };

// We want to create an ActionNode that calls the method MyLegacyMoveTo::go
class MyLegacyMoveTo
{
public:
    bool go(Point3D goal)
    {
        printf("Going to: %f %f %f\n", goal.x, goal.y, goal.z);
        return true; // true means success in my legacy code
    }
};

// Similarly to the previous tutorials, we need to implement this parsing method,
// providing a specialization of BT::convertFromString
namespace BT
{
template <> Point3D convertFromString(StringView key)
{
    // three real numbers separated by semicolons
    auto parts = BT::splitString(key, ';');
    if (parts.size() != 3)
    {
        throw RuntimeError("invalid input)");
    }
    else
    {
        Point3D output;
        output.x  = convertFromString<double>(parts[0]);
        output.y  = convertFromString<double>(parts[1]);
        output.z  = convertFromString<double>(parts[2]);
        return output;
    }
}
}


int main()
{
    using namespace BT;

    MyLegacyMoveTo move_to;

    // Here we use a lambda that captures the reference of move_to
    auto MoveToWrapperWithLambda = [&move_to](TreeNode& parent_node) -> NodeStatus
    {
        Point3D goal;
        // thanks to paren_node, you can access easily the inpyt and output ports.
        parent_node.getInput("goal", goal);

        bool res = move_to.go( goal );
        // convert bool to NodeStatus
        return res ? NodeStatus::SUCCESS : NodeStatus::FAILURE;
    };

    BehaviorTreeFactory factory;
    // Register the lambda with BehaviorTreeFactory::registerSimpleAction
    factory.registerSimpleAction("MoveTo", MoveToWrapperWithLambda);

    auto blackboard = Blackboard::create<BlackboardLocal>();
    auto tree = buildTreeFromText(factory, xml_text, blackboard);

    // We set the entry "myGoal" in the blackboard.
    Point3D my_goal = {3,4,5};
    blackboard->set("myGoal", my_goal);

    NodeStatus status = NodeStatus::RUNNING;
    while (status == NodeStatus::RUNNING)
    {
        status = tree.root_node->executeTick();
    }
    return 0;
}

/* Expected output:

Going to: -1.000000 3.000000 0.500000
Going to: 3.000000 4.000000 5.000000

The first MoveTo read the parameter from the string "-1;3;0.5"
whilst the second from the blackboard, that contains a copy of the Point3D my_goal.

*/
