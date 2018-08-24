#include "behavior_tree_core/xml_parsing.h"
#include "behavior_tree_logger/bt_cout_logger.h"
#include "Blackboard/blackboard_local.h"

#include "movebase_node.h"

using namespace BT;

// Write into the blackboard key: [GoalPose]
NodeStatus CalculateGoalPose(const Blackboard::Ptr& blackboard)
{
    //In this example we store a fixed value. In a real application
    // we would read it from an external source (GUI, fleet manager, etc.)
    Pose2D goal = { 1, 2, M_PI};

    // RECOMMENDED: check if the blackboard is empty
    if( blackboard )
    {
        // store it in the blackboard
        blackboard->set("GoalPose", goal);
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
};


/** Example: simple sequence of 4 actions

  1) Store a value of Pose2D in the key "GoalPose" of the blackboard.
  2) Call PrintGoalPose. It will read "GoalPose" from the blackboard.
  3) Call MoveAction that reads the NodeParameter "goal". It's value "2;4;0" is converted
     to Pose2D using the function [ Pose2D convertFromString(const std::string& str) ].
  4) Call MoveAction. It will read "GoalPose" from the blackboard.

*/
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

int main()
{
    using namespace BT;

    BehaviorTreeFactory factory;
    factory.registerSimpleAction("CalculateGoalPose", CalculateGoalPose);
    factory.registerNodeType<PrintGoalPose>("PrintGoalPose");
    factory.registerNodeType<MoveBaseAction>("MoveBase");

    // create a Blackboard from BlackboardLocal (simple local storage)
    auto blackboard = Blackboard::create<BlackboardLocal>();

    // Important: when the object tree goes out of scope, all the TreeNodes are destroyed
    auto res = buildTreeFromText(factory, xml_text, blackboard);
    const TreeNode::Ptr& root_node = res.first;

    NodeStatus status = NodeStatus::RUNNING;
    while( status == NodeStatus::RUNNING )
    {
        status = root_node->executeTick();
    }

    return 0;
}
