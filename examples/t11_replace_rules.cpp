#include "behaviortree_cpp/bt_factory.h"
#include "dummy_nodes.h"

// clang-format off

static const char* xml_text = R"(
<root BTCPP_format="4">

    <BehaviorTree ID="MainTree">
        <Sequence>
            <SaySomething name="talk" message="hello world"/>
            <Fallback>
                <AlwaysFailure name="failing_action"/>
                <SubTree ID="MySub" name="mysub"/>
            </Fallback>
            <AlwaysSuccess name="last_action"/>
        </Sequence>
    </BehaviorTree>

    <BehaviorTree ID="MySub">
      <Sequence>
        <AlwaysSuccess name="action_subA"/>
        <AlwaysSuccess name="action_subB"/>
      </Sequence>
    </BehaviorTree>

</root>
 )";

// clang-format on

int main()
{
  using namespace DummyNodes;
  BT::BehaviorTreeFactory factory;

  factory.registerNodeType<SaySomething>("SaySomething");

  // Here we use lambdas and registerSimpleAction, to create the "dummy" or
  // "test" nodes.
  // You can't use the usual registerNodeType approach.

  // Simple node that just print its name and return SUCCESS
  factory.registerSimpleAction("TestAction", [](BT::TreeNode& self){
    std::cout << "TestAction substituting: "<< self.name() << std::endl;
    return BT::NodeStatus::SUCCESS;
  });

  // Action that is means to substitute SaySomething.
  // It will try to use the input port "message"
  factory.registerSimpleAction("TestSaySomething", [](BT::TreeNode& self){
    auto msg = self.getInput<std::string>("message");
    if (!msg)
    {
      throw BT::RuntimeError( "missing required input [message]: ", msg.error() );
    }
    std::cout << "TestSaySomething: " << msg.value() << std::endl;
    return BT::NodeStatus::SUCCESS;
  });

  // Here we specify (using wildcard pattern, optionally) which
  // Nodes should be sustityted and with which other node.

  factory.addSubstitutionRule("mysub/action_*", "TestAction");
  factory.addSubstitutionRule("last_action", "TestAction");
  factory.addSubstitutionRule("talk", "TestSaySomething");

  factory.registerBehaviorTreeFromText(xml_text);

  // During the construction phase of the tree, the substitution
  // rules will be used to instantiate the test nodes, instead of the
  // original ones.
  auto tree = factory.createTree("MainTree");

  BT::printTreeRecursively(tree.rootNode());

  tree.tickWhileRunning();
  return 0;
}
