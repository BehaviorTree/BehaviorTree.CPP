#include "behavior_tree_core/xml_parsing.h"
#include "behavior_tree_logger/bt_cout_logger.h"
#include "Blackboard/blackboard_local.h"

using namespace BT;

struct Pose2D
{
    double x,y,theta;
};

namespace BT{

// this template specialization is needed ONLY if you want
// to AUTOMATICALLY convert a NodeParameter into a Pose2D
template <> Pose2D convertFromString(const std::string& str)
{
    auto parts = BT::splitString(str, ';');
    if( parts.size() != 3)
    {
        throw std::runtime_error("invalid input)");
    }
    else{
        Pose2D output;
        output.x     = convertFromString<double>( parts[0] );
        output.y     = convertFromString<double>( parts[1] );
        output.theta = convertFromString<double>( parts[2] );
        return output;
    }
}

}

//-----------------------------------------

// This action will read the desired robot position
// and store it on the BlackBoard (key: "GoalPose")
NodeStatus PullGoalPose(const std::shared_ptr<Blackboard>& blackboard)
{
    //In this example we store a fixed value. In a real application
    // we would read it from an external source (GUI, fleet manager, etc.)
    Pose2D goal = { 1, 2, M_PI};

    // store it in the blackboard
    blackboard->set("GoalPose", goal);

    return NodeStatus::SUCCESS;
}

// First approach: read ALWAYS from the same
// blackboard key: [GoalPose]
class MoveAction_A: public ActionNodeBase
{
public:
    MoveAction_A(const std::string& name): ActionNodeBase(name) {}

    NodeStatus tick() override
    {
        Pose2D goal;
        if( blackboard()->get("GoalPose", goal) ) // return true if succesful
        {
            printf("[MoveBase] [Taget: x=%.ff y=%.1f theta=%.2f\n",
                   goal.x, goal.y, goal.theta);
            return NodeStatus::SUCCESS;
        }
        else{
           printf("The blackboard does not contain the key [GoalPose]\n");
           return NodeStatus::FAILURE;
        }
    }
};

// Second approach: read the goal from the NodeParameter "goal".
// This value can be static or point to the key of a blackboard.
// A pointer to a Blackboard entry is written as ${key}
class MoveAction_B: public ActionNodeBase
{
public:
    MoveAction_B(const std::string& name, const NodeParameters& params):
        ActionNodeBase(name, params) {}

    NodeStatus tick() override
    {
        Pose2D goal;
        if( getParam<Pose2D>("goal", goal) )
        {
            printf("[MoveBase] [Taget: x=%.ff y=%.1f theta=%.2f\n",
                   goal.x, goal.y, goal.theta);
            return NodeStatus::SUCCESS;
        }
        else{
           printf("The NodeParameter does not contain the key [goal] "
                  " or the blackboard does not contain the provided key\n");
           return NodeStatus::FAILURE;
        }
    }
    static const NodeParameters& requiredNodeParameters()
    {
        static NodeParameters params = {{"goal","0;0;0"}};
        return params;
    }
};




/** Example: simple sequence of 4 actions

  1) Store a value of Pose2D in the key "GoalPose" of the blackboard.
  2) Call MoveAction_A. It will read "GoalPose" from the blackboard.
  3) Call MoveAction_B that reads the NodeParameter "goal". It's value "2;4;0" is converted
     to Pose2D using the function [ Pose2D convertFromString(const std::string& str) ].
  4) Call MoveAction_B. It will read "GoalPose" from the blackboard  .

*/
const std::string xml_text = R"(

 <root main_tree_to_execute = "MainTree" >

     <BehaviorTree ID="MainTree">
        <Sequence name="root">
            <PullGoalPose/>
            <MoveAction_A />
            <MoveAction_B  goal="2;4;0" />
            <MoveAction_B  goal="${GoalPose}" />
        </Sequence>
     </BehaviorTree>

 </root>
 )";


// clang-format on

int main()
{
    using namespace BT;

    BehaviorTreeFactory factory;
    factory.registerSimpleAction("PullGoalPose", PullGoalPose);
    factory.registerNodeType<MoveAction_A>("MoveAction_A");
    factory.registerNodeType<MoveAction_B>("MoveAction_B");

    // create a Blackboard from BlackboardLocal (simple local storage)
    auto blackboard = Blackboard::create<BlackboardLocal>();

    auto res = buildTreeFromText(factory, xml_text, blackboard);

    const TreeNode::Ptr& root_node = res.first;

    root_node->executeTick();

    return 0;
}
