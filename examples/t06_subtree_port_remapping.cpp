#include "behaviortree_cpp_v3/loggers/bt_cout_logger.h"
#include "behaviortree_cpp_v3/bt_factory.h"

#include "movebase_node.h"
#include "dummy_nodes.h"

/** In the CrossDoor example we did not exchange any information
 * between the Maintree and the DoorClosed subtree.
 *
 * If we tried to do that, we would have noticed that it can't be done, because
 * each of the tree/subtree has its own Blackboard, to avoid the problem of name
 * clashing in very large trees.
 *
 * But a SubTree can have its own input/output ports.
 * In practice, these ports are nothing more than "soft links" between the
 * ports inside the SubTree (called "internal") and those in the parent
 * Tree (called "external").
 *
 */

// clang-format off

static const char* xml_text = R"(
<root main_tree_to_execute = "MainTree">

    <BehaviorTree ID="MainTree">

        <Sequence name="main_sequence">
            <SetBlackboard output_key="move_goal" value="1;2;3" />
            <SubTree ID="MoveRobot" target="move_goal" output="move_result" />
            <SaySomething message="{move_result}"/>
        </Sequence>

    </BehaviorTree>

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

</root>
 )";

// clang-format on

using namespace BT;
using namespace DummyNodes;

int main()
{
  BehaviorTreeFactory factory;

  factory.registerNodeType<SaySomething>("SaySomething");
  factory.registerNodeType<MoveBaseAction>("MoveBase");

  auto tree = factory.createTreeFromText(xml_text);

  tree.tickRootWhileRunning();

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
