#include "dummy_nodes.h"

#include "behaviortree_cpp/bt_factory.h"

// clang-format off
namespace
{
const char* xml_text = R"(
<root BTCPP_format="4">

  <BehaviorTree ID="MainTree">
    <Sequence>
      <SaySomething name="talk" message="hello world"/>

      <SubTree ID="MySub" name="mysub"/>

      <Script name="set_mock_flag" code="mock_should_fail:= false"/>
      <Script name="set_message" code="msg:= 'the original message' "/>
      <SaySomething message="{msg}"/>

      <Sequence name="counting">
        <SaySomething message="1"/>
        <SaySomething message="2"/>
        <SaySomething message="3"/>
      </Sequence>
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
}  // namespace
// clang-format on

/**
 * @brief Companion to tutorial 15 that preserves strict failure propagation.
 *
 * If the substituted TestNode resolves to FAILURE, the enclosing Sequence stops
 * immediately. The final blackboard message is still printed by this executable,
 * but the tree itself does not log the failure branch through a fallback.
 */

int main(int /*argc*/, char** /*argv*/)
{
  using namespace DummyNodes;
  BT::BehaviorTreeFactory factory;
  factory.registerNodeType<SaySomething>("SaySomething");
  factory.registerBehaviorTreeFromText(xml_text);

  {
    auto tree = factory.createTree("MainTree");

    std::cout << "----- Nodes fullPath() -------\n";
    tree.applyVisitor(
        [](BT::TreeNode* node) { std::cout << node->fullPath() << std::endl; });

    std::cout << "\n------ Output (original) ------\n";
    auto status = tree.tickWhileRunning();
    std::cout << "Original status: " << BT::toStr(status, false) << std::endl;
  }

  factory.registerSimpleAction("DummyAction", [](BT::TreeNode& self) {
    std::cout << "DummyAction substituting node with fullPath(): " << self.fullPath()
              << std::endl;
    return BT::NodeStatus::SUCCESS;
  });

  factory.registerSimpleAction("DummySaySomething", [](BT::TreeNode& self) {
    auto msg = self.getInput<std::string>("message");
    std::cout << "DummySaySomething: " << msg.value() << std::endl;
    return BT::NodeStatus::SUCCESS;
  });

  BT::TestNodeConfig test_config;
  test_config.return_status_script = "(mock_should_fail == true) ? FAILURE : SUCCESS";
  test_config.async_delay = std::chrono::milliseconds(2000);
  test_config.success_script = "msg := 'message SUBSTITUTED' ";
  test_config.failure_script = "msg := 'message FAILURE branch' ";

  BT::TestNodeConfig counting_config;
  counting_config.return_status = BT::NodeStatus::SUCCESS;

  factory.addSubstitutionRule("mysub/action_*", "DummyAction");
  factory.addSubstitutionRule("talk", "DummySaySomething");
  factory.addSubstitutionRule("set_message", test_config);
  factory.addSubstitutionRule("counting", counting_config);

  auto blackboard = BT::Blackboard::create();
  auto tree = factory.createTree("MainTree", blackboard);
  std::cout << "\n------ Output (substituted, strict failure) ------\n";
  auto status = tree.tickWhileRunning();
  std::cout << "Substituted status: " << BT::toStr(status, false) << std::endl;
  if(blackboard->getEntry("msg"))
  {
    std::cout << "Substituted final msg: " << blackboard->get<std::string>("msg")
              << std::endl;
  }

  return 0;
}

/* Expected output:

----- Nodes fullPath() -------
Sequence::1
talk
mysub
mysub/Sequence::4
mysub/action_subA
mysub/action_subB
set_mock_flag
set_message
SaySomething::9
counting
SaySomething::11
SaySomething::12
SaySomething::13

------ Output (original) ------
Robot says: hello world
Robot says: the original message
Robot says: 1
Robot says: 2
Robot says: 3
Original status: SUCCESS

------ Output (substituted, strict failure) ------
DummySaySomething: hello world
DummyAction substituting node with fullPath(): mysub/action_subA
DummyAction substituting node with fullPath(): mysub/action_subB
Substituted status: SUCCESS
Substituted final msg: message SUBSTITUTED

If you change mock_should_fail to true in the XML above, the substituted status
becomes FAILURE and the final msg becomes "message FAILURE branch".

*/
