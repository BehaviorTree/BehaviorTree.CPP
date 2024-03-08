#include "dummy_nodes.h"
#include "behaviortree_cpp/bt_factory.h"

/** This example show how it is possible to:
 * - load BehaviorTrees from multiple files manually (without the <include> tag)
 * - instantiate a specific tree, instead of the one specified by [main_tree_to_execute]
 */

// clang-format off

static const char* xml_text_main = R"(
<root BTCPP_format="4">
    <BehaviorTree ID="MainTree">
        <Sequence>
            <SaySomething message="starting MainTree" />
            <SubTree ID="SubA"/>
            <SubTree ID="SubB"/>
        </Sequence>
    </BehaviorTree>
</root>  )";

static const char* xml_text_subA = R"(
<root BTCPP_format="4">
    <BehaviorTree ID="SubA">
        <SaySomething message="Executing SubA" />
    </BehaviorTree>
</root>  )";

static const char* xml_text_subB = R"(
<root BTCPP_format="4">
    <BehaviorTree ID="SubB">
        <SaySomething message="Executing SubB" />
    </BehaviorTree>
</root>  )";

// clang-format on

using namespace BT;

int main()
{
  BT::BehaviorTreeFactory factory;
  factory.registerNodeType<DummyNodes::SaySomething>("SaySomething");

  // Register the behavior tree definitions, but do not instantiate them yet.
  // Order is not important.
  factory.registerBehaviorTreeFromText(xml_text_subA);
  factory.registerBehaviorTreeFromText(xml_text_subB);
  factory.registerBehaviorTreeFromText(xml_text_main);

  //Check that the BTs have been registered correctly
  std::cout << "Registered BehaviorTrees:" << std::endl;
  for(const std::string& bt_name : factory.registeredBehaviorTrees())
  {
    std::cout << " - " << bt_name << std::endl;
  }

  // You can create the MainTree and the subtrees will be added automatically.
  std::cout << "----- MainTree tick ----" << std::endl;
  auto main_tree = factory.createTree("MainTree");
  main_tree.tickWhileRunning();

  // ... or you can create only one of the subtree
  std::cout << "----- SubA tick ----" << std::endl;
  auto subA_tree = factory.createTree("SubA");
  subA_tree.tickWhileRunning();

  return 0;
}
/* Expected output:

Registered BehaviorTrees:
 - MainTree
 - SubA
 - SubB
----- MainTree tick ----
Robot says: starting MainTree
Robot says: Executing SubA
Robot says: Executing SubB
----- SubA tick ----
Robot says: Executing SubA

*/
