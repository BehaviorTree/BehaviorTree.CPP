#include "behaviortree_cpp/bt_factory.h"

using namespace BT;

/** This tutorial will tech you how to deal with ports which type
 * is different from std:string.
*/


// In this example we want to be able to use the type Position2D
struct Position2D { double x,y; };

// It ie recommended (and in some case mandatory) to define a template
// specialization of convertFromString that convert a string to Position2D.

namespace BT
{
template <> inline Position2D convertFromString(StringView key)
{
    printf("Converting string: \"%s\"\n", key.data() );
    // real numbers separated by semicolons
    auto parts = BT::splitString(key, ';');
    if (parts.size() != 2)
    {
        throw BT::RuntimeError("invalid input)");
    }
    else{
        Position2D output;
        output.x     = convertFromString<double>(parts[0]);
        output.y     = convertFromString<double>(parts[1]);
        return output;
    }
}
} // end namespace BT


class CalculateGoal: public SyncActionNode
{
public:
    CalculateGoal(const std::string& name, const NodeConfiguration& config):
        SyncActionNode(name,config)
    {}

    NodeStatus tick() override
    {
        Position2D mygoal = {1.1, 2.3};
        setOutput("goal", mygoal);
        return NodeStatus::SUCCESS;
    }
    static BT::PortsList providedPorts()
    {
        return { BT::OutputPort<Position2D>("goal") };
    }
};


// Write into the blackboard.
class PrintGoal: public SyncActionNode
{
public:
    PrintGoal(const std::string& name, const NodeConfiguration& config):
        SyncActionNode(name,config)
    {}

    NodeStatus tick() override
    {
        auto res = getInput<Position2D>("goal");
        if( !res )
        {
            throw BT::RuntimeError("error reading port [goal]:", res.error() );
        }
        Position2D goal = res.value();
        printf("Goal positions: %.1f %.1f\n", goal.x, goal.y );
        return NodeStatus::SUCCESS;
    }

    static BT::PortsList providedPorts()
    {
        // Optionally, a port can have a human readable description
        const char*  description = "Simply print the goal on console...";
        return { BT::InputPort<Position2D>("goal", description) };
    }
};

//----------------------------------------------------------------

/** The tree is a Sequence of 4 actions

*  1) Store a value of Position2D in the key "GoalPosition" of the blackboard
*     using the action CalculateGoal.
*
*  2) Call PrintGoal. The input "GoalPosition"  will be read from the
*     Blackboard at run-time.
*
*  3) Use the built-in action SetBlackboard to write the key "OtherGoal".
*     A conversion from string to Position2D will be done under the hood.
*
*  4) Call MoveAction. The input "goal" will be read from the Blackboard
*     entry "OtherGoal".
*/

// clang-format off
static const char* xml_text = R"(

 <root main_tree_to_execute = "MainTree" >
     <BehaviorTree ID="MainTree">
        <SequenceStar name="root">
            <CalculateGoal goal="{GoalPosition}" />
            <PrintGoal  goal="{GoalPosition}" />
            <SetBlackboard output_key="OtherGoal" value="-1;3" />
            <PrintGoal  goal="{OtherGoal}" />
        </SequenceStar>
     </BehaviorTree>
 </root>
 )";

// clang-format on

int main()
{
    using namespace BT;

    BehaviorTreeFactory factory;
    factory.registerNodeType<CalculateGoal>("CalculateGoal");
    factory.registerNodeType<PrintGoal>("PrintGoal");

    auto tree = factory.createTreeFromText(xml_text);
    tree.root_node->executeTick();

    return 0;
}

/* Expected output:
 *
    Goal positions: 1.1 2.3
    Converting string: "-1;3"
    Goal positions: -1.0 3.0
*/
