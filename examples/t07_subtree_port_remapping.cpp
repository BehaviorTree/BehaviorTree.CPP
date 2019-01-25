#include "behaviortree_cpp/loggers/bt_cout_logger.h"
#include "behaviortree_cpp/bt_factory.h"

#include "movebase_node.h"
#include "dummy_nodes.h"

/** In the CrossDoor example we did not exchange any information
 * between the Maintree and the DoorClosed subtree.
 *
 * If we tried to do that we would have noticed that it can be done, because
 * each of the tree/subtree has its own Blackboard to avoid the problem of name
 * clashing in very large trees.
 *
 * But a SubTree can have input/output ports itself.
 * In practice, these ports are nothing more than "soft links" between the
 * ports inside the SubTree (called "internal") and those in the parent
 * Tree (called "external").
 *
 */


// clang-format off

const std::string xml_text = R"(
<root main_tree_to_execute = "MainTree">
    <!-- .................................. -->
    <BehaviorTree ID="MainTree">

        <Sequence name="main_sequence">
            <SetBlackboard output_key="move_goal" value="1;2;3" />
            <SubTree ID="MoveRobot">
                <remap internal="target" external="move_goal"/>
                <remap internal="output" external="move_result"/>
            </SubTree>
            <SaySomething message="{move_result}"/>
        </Sequence>

    </BehaviorTree>
    <!-- .................................. -->
    <BehaviorTree ID="MoveRobot">
        <Fallback name="move_robot_main">
            <SequenceStar>
                <MoveBase       goal="{target}"/>
                <SetBlackboard output_key="output" value="mission accomplished" />
            </SequenceStar>
            <ForceFailure>
                <SetBlackboard output_key="output" value="mission failed" />
            </ForceFailure>
        </Fallback>
    </BehaviorTree>
    <!---------------------------------------> 
</root>
 )";

// clang-format on

/** using the <remap> tag we where able to connect the ports as follows:
 *
 *   MoveRobot->target  is connected to MainTree->move_goal
 *   MoveRobot->output  is connected to MainTree->move_result
 *
 */

using namespace BT;

int main()
{
    BT::BehaviorTreeFactory factory;

    DummyNodes::RegisterNodes(factory);
    factory.registerNodeType<MoveBaseAction>("MoveBase");

    // Important: when the object tree goes out of scope, all the TreeNodes are destroyed
    auto tree = factory.createTreeFromText(xml_text);


    {
        NodeStatus status = NodeStatus::RUNNING;
        // Keep on ticking until you get either a SUCCESS or FAILURE state
        while( status == NodeStatus::RUNNING)
        {
            status = tree.root_node->executeTick();
            SleepMS(1);   // optional sleep to avoid "busy loops"
        }
    }

    // let's visualize some information about the current state of the blackboards.
    std::cout << "--------------" << std::endl;
    tree.blackboard_stack[0]->debugMessage();
    std::cout << "--------------" << std::endl;
    tree.blackboard_stack[1]->debugMessage();
    std::cout << "--------------" << std::endl;

    return 0;
}

/* Expected output:

    [ MoveBase: STARTED ]. goal: x=1 y=2.0 theta=3.00
    [ MoveBase: FINISHED ]
    Robot says: mission accomplished
    --------------
    move_result (std::string) -> full
    move_goal (Pose2D) -> full
    --------------
    output (std::string) -> remapped to parent [move_result]
    target (Pose2D) -> remapped to parent [move_goal]
    --------------

*/
