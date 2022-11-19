#include "behaviortree_cpp/loggers/bt_cout_logger.h"
#include "behaviortree_cpp/bt_factory.h"

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
<root BTCPP_format="4">

    <BehaviorTree ID="MainTree">
        <Sequence>
            <Script code=" move_goal='1;2;3' " />
            <SubTree ID="MoveRobot" target="{move_goal}" result="{move_result}" />
            <SaySomething message="{move_result}"/>
        </Sequence>
    </BehaviorTree>

    <BehaviorTree ID="MoveRobot">
        <Fallback>
            <Sequence>
                <MoveBase  goal="{target}"/>
                <Script code=" result:='goal reached' " />
            </Sequence>
            <ForceFailure>
                <Script code=" result:='error' " />
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

  factory.registerBehaviorTreeFromText(xml_text);
  auto tree = factory.createTree("MainTree");

  tree.tickWhileRunning();

  // let's visualize some information about the current state of the blackboards.
  std::cout << "\n------ First BB ------" << std::endl;
  tree.subtrees[0]->blackboard->debugMessage();
  std::cout << "\n------ Second BB------" << std::endl;
  tree.subtrees[1]->blackboard->debugMessage();

  return 0;
}

/* Expected output:

------ First BB ------
move_result (std::string)
move_goal (Pose2D)

------ Second BB------
[result] remapped to port of parent tree [move_result]
[target] remapped to port of parent tree [move_goal]

*/
