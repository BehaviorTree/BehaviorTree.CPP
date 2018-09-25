#include "behavior_tree_core/xml_parsing.h"
#include "behavior_tree_logger/bt_cout_logger.h"
#include "Blackboard/blackboard_local.h"

#include "movebase_node.h"

using namespace BT;

/** This tutorial will tech you:
 *
 *  - How to use the Blackboard to shared data between TreeNodes
 *  - How to use a Blackboard as a NodeParameter
 *
 * The tree is a Sequence of 4 actions

 *  1) Store a value of Pose2D in the key "GoalPose" of the blackboard.
 *  2) Call PrintGoalPose. It will read "GoalPose" from the blackboard.
 *  3) Call MoveAction. The NodeParameter "goal" is provided by the XML itself at the beginning.
 *  4) Call MoveAction. The NodeParameter "goal" will be read from the Blackboard at run-time.
 *
*/

// clang-format off
const std::string xml_text = R"(

 <root main_tree_to_execute = "MainTree" >

     <BehaviorTree ID="MainTree">
        <SequenceStar name="root">
            <CalculateGoalPose/>
            <PrintGoalPose />
            <MoveBase  goal="2;4;0" />
            <MoveBase  goal="${GoalPose}" />
        </SequenceStar>
     </BehaviorTree>

 </root>
 )";

// clang-format on

// Write into the blackboard key: [GoalPose]
// Use this function signature to create a SimpleAction that needs the blackboard
NodeStatus CalculateGoalPose(const Blackboard::Ptr& blackboard)
{
    const Pose2D mygoal = { 1, 2, M_PI};

    // RECOMMENDED: check if the blackboard is nullptr
    if( blackboard )
    {
        // store it in the blackboard
        blackboard->set("GoalPose", mygoal);
    }

    printf("[CalculateGoalPose]\n");
    return NodeStatus::SUCCESS;
}

// Read from the blackboard key: [GoalPose]
class PrintGoalPose: public ActionNodeBase
{
public:
    PrintGoalPose(const std::string& name): ActionNodeBase(name) {}

    NodeStatus tick() override
    {
        Pose2D goal;
        // RECOMMENDED: check if the blackboard is empty
        if( blackboard() && blackboard()->get("GoalPose", goal) )
        {
            printf("[PrintGoalPose] x=%.f y=%.1f theta=%.2f\n",
                   goal.x, goal.y, goal.theta);
            return NodeStatus::SUCCESS;
        }
        else{
           printf("The blackboard does not contain the key [GoalPose]\n");
           return NodeStatus::FAILURE;
        }
    }

    virtual void halt() override { setStatus(NodeStatus::IDLE); }
};

int main()
{
    using namespace BT;

    BehaviorTreeFactory factory;
    factory.registerSimpleAction("CalculateGoalPose", CalculateGoalPose);
    factory.registerNodeType<PrintGoalPose>("PrintGoalPose");
    factory.registerNodeType<MoveBaseAction>("MoveBase");

    // create a Blackboard from BlackboardLocal (simple, not persistent, local storage)
    auto blackboard = Blackboard::create<BlackboardLocal>();

    // Important: when the object tree goes out of scope, all the TreeNodes are destroyed
    auto tree = buildTreeFromText(factory, xml_text, blackboard);

    NodeStatus status = NodeStatus::RUNNING;
    while( status == NodeStatus::RUNNING )
    {
        status = tree.root_node->executeTick();
        SleepMS(1); // optional sleep to avoid "busy loops"
    }

    return 0;
}
