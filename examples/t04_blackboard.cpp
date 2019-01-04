#include "behaviortree_cpp/xml_parsing.h"
#include "behaviortree_cpp/loggers/bt_cout_logger.h"
#include "behaviortree_cpp/blackboard/blackboard_local.h"

#include "movebase_node.h"

using namespace BT;

/** This tutorial will tech you:
 *
 *  - How to use the Blackboard to shared data between TreeNodes
 *
 * The tree is a Sequence of 4 actions

 *  1) Store a value of Pose2D in the key "GoalPose" of the blackboard using the action CalculateGoalPose.
 *  2) Call MoveAction. The input "GoalPose"  will be read from the Blackboard at run-time.
 *  3) Use the built-in action SetBlackboard to write the key "OtherGoal".
 *  4) Call MoveAction. The input "goal" will be read from the Blackboard entry "OtherGoal".
 *
*/

// clang-format off
const std::string xml_text = R"(

 <root main_tree_to_execute = "MainTree" >
     <BehaviorTree ID="MainTree">
        <SequenceStar name="root">
            <CalculateGoalPose goal="${GoalPose}" />
            <MoveBase  goal="${GoalPose}" />
            <SetBlackboard output_key="OtherGoal" value="-1;3;0.5" />
            <MoveBase  goal="${OtherGoal}" />
        </SequenceStar>
     </BehaviorTree>
 </root>
 )";

// clang-format on

// Write into the blackboard.
class CalculateGoalPose: public SyncActionNode
{
public:
    CalculateGoalPose(const std::string& name, const NodeConfiguration& config):
        SyncActionNode(name,config)
    {}

    NodeStatus tick() override
    {
        const Pose2D mygoal = {1.1, 2.3, 1.54};
        setOutput("goal", mygoal);
        return NodeStatus::SUCCESS;
    }
    static const BT::PortsList& providedPorts()
    {
        static BT::PortsList ports = {{"goal", BT::PortType::OUTPUT}};
        return ports;
    }
};


int main()
{
    using namespace BT;

    BehaviorTreeFactory factory;
    factory.registerNodeType<CalculateGoalPose>("CalculateGoalPose");
    factory.registerNodeType<MoveBaseAction>("MoveBase");

    // create a Blackboard from BlackboardLocal (simple, not persistent, local storage)
    auto blackboard = Blackboard::create<BlackboardLocal>();

    // Important: when the object tree goes out of scope, all the TreeNodes are destroyed
    auto tree = buildTreeFromText(factory, xml_text, blackboard);

    NodeStatus status = NodeStatus::RUNNING;
    while (status == NodeStatus::RUNNING)
    {
        status = tree.root_node->executeTick();
        SleepMS(1);   // optional sleep to avoid "busy loops"
    }
    return 0;
}
