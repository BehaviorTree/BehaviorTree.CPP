#include "behavior_tree_core/xml_parsing.h"
#include "behavior_tree_logger/bt_cout_logger.h"
#include "Blackboard/blackboard_local.h"

using namespace BT;

struct Pose2D
{
    double x,y,theta;
};

// This action will read the desired robot position
// and store it on the BlackBoard (key: "GoalPose")
class PullGoalPose: public ActionNodeBase
{
public:
    PullGoalPose(const std::string& name): ActionNodeBase(name) {}

    NodeStatus tick() override
    {
        //In this example we hardcode a fixed value.
        //In a real application we would read it from an external
        // source (GUI, fleet manager, etc.)
        Pose2D goal = { 1, 2, M_PI};

        // store it in the blackboard
        blackboard()->set("GoalPose", goal);

        return NodeStatus::SUCCESS;
    }
};


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

namespace BT{

// this template specialization is needed if you want
// to automatically convert a NodeParameter into a Pose2D

template <>
Pose2D convertFromString(const std::string& str)
{
    auto parts = splitString(str, ';');
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


const std::string xml_text = R"(

 <root main_tree_to_execute = "MainTree" >

     <BehaviorTree ID="MainTree">
        <Sequence name="root">
            <PullGoalPose/>
            <MoveAction_A />
            <MoveAction_B  goal="2;4;0" />
            <PullGoalPose/>
            <MoveAction_B  goal="${GoalPose}" />
        </Sequence>
     </BehaviorTree>

 </root>
 )";


// clang-format on

int main(int argc, char** argv)
{
    using namespace BT;

    BehaviorTreeFactory factory;
    factory.registerNodeType<PullGoalPose>("PullGoalPose");
    factory.registerNodeType<MoveAction_A>("MoveAction_A");
    factory.registerNodeType<MoveAction_B>("MoveAction_B");

    XMLParser parser(factory);
    parser.loadFromText(xml_text);

    std::vector<TreeNodePtr> nodes;
    TreeNodePtr root_node = parser.instantiateTree(nodes);

    // create a Blackboard and assign it to your tree
    auto bb = Blackboard::create<BlackboardLocal>();
    for(auto& node: nodes)
    {
        node->setBlackboard(bb);
    }

    root_node->executeTick();

    return 0;
}
