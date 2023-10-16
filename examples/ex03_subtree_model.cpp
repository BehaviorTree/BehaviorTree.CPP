#include "behaviortree_cpp/bt_factory.h"
#include "behaviortree_cpp/decorators/loop_node.h"
#include "behaviortree_cpp/loggers/bt_cout_logger.h"
#include <list>

using namespace BT;

/*
 * Demonstrate how to use a SubTree Model (since version 4.4)
 */

/**
 * You can optinally add a model to a SubTrees, in this case "MySub".
 * We are telling to the factory that the callee should remap
 * two mandatory inputs, called:
 *
 * - in_value (that has the default value 42)
 * - in_name (no default)
 *
 * Similarly, there are two output values:
 *
 * - out_value (default remapping to port {output})
 * - out_state (no default)
 *
 * The callee MUST specify, at least, those remapping that have
 * no default value.
 */

// clang-format off
static const char* xml_subtree = R"(
<root BTCPP_format="4">

  <TreeNodesModel>
    <SubTree ID="MySub">
      <input_port name="in_value" default="42"/>
      <input_port name="in_name"/>
      <output_port name="out_result" default="{output}"/>
      <output_port name="out_state"/>
    </SubTree>
  </TreeNodesModel>

  <BehaviorTree ID="MySub">
    <Sequence>
      <ScriptCondition code="in_name=='john' && in_value==42" />
      <Script code="out_result:=69; out_state:='ACTIVE'" />
    </Sequence>
  </BehaviorTree>
</root>
 )";

/**
 * Here, when calling "MySub", only in_name and out_state are explicitly
 * remapped. Will we use the default values for the other two.
 */

static const char* xml_maintree = R"(
<root BTCPP_format="4">

  <BehaviorTree ID="MainTree">
    <Sequence>
      <Script code="name_arg:= 'john' "/>
      <SubTree ID="MySub" in_name="{name_arg}"  out_state="{state}"/>
      <ScriptCondition code=" output==69 && state=='ACTIVE' " />
    </Sequence>
  </BehaviorTree>

</root>
 )";

// clang-format on

int main()
{
  BehaviorTreeFactory factory;
  factory.registerBehaviorTreeFromText(xml_subtree);
  factory.registerBehaviorTreeFromText(xml_maintree);

  auto tree = factory.createTree("MainTree");
  StdCoutLogger logger(tree);
  tree.tickWhileRunning();

  // We expect the sequence to be succesful.

  // The full remapping was:
  //
  // - in_name <-> {name_arg}   // specified by callee
  // - in_value <-> 42          // default
  // - out_result <-> {output}  // default
  // - out_state <-> {state}    // specified by callee

  return 0;
}
