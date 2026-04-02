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
      <Fallback name="report_mock_message">
        <Sequence>
          <Script name="set_message" code="msg:= 'the original message' "/>
          <SaySomething message="{msg}"/>
        </Sequence>
        <SaySomething message="{msg}"/>
      </Fallback>

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
 * @brief In this example we will see how we can substitute some nodes
 * in the Tree above with mocks.
 *
 * This variant is optimized for observability: even when the substituted
 * TestNode resolves to FAILURE, a Fallback logs the final message from inside
 * the tree.
 *
 * See t15_nodes_mocking_strict_failure.cpp for the companion example that
 * preserves strict failure propagation instead.
 * @param argc
 * @param argv
 * @return
 */

int main(int /*argc*/, char** /*argv*/)
{
  using namespace DummyNodes;
  BT::BehaviorTreeFactory factory;
  factory.registerNodeType<SaySomething>("SaySomething");
  factory.registerBehaviorTreeFromText(xml_text);

  // let's check what the "original" tree should return
  {
    auto tree = factory.createTree("MainTree");

    std::cout << "----- Nodes fullPath() -------\n";
    // as a reminder, let's print the full names of all the nodes
    tree.applyVisitor(
        [](BT::TreeNode* node) { std::cout << node->fullPath() << std::endl; });

    std::cout << "\n------ Output (original) ------\n";
    tree.tickWhileRunning();
  }

  // We have three mechanisms to create Nodes to be used as "mocks".
  // We will see later how to use them.

  //---------------------------------------------------------------
  // Mock type 1: register a specific "dummy" Node into the factory
  // You can use any registration method, but to keep this short,
  // we use registerSimpleAction()

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

  //---------------------------------------------------------------
  // Mock type 2: Use our configurable BT::TestNode

  // This is the configuration passed to the TestNode
  BT::TestNodeConfig test_config;
  // the returned status can also be computed from a script.
    // Change mock_should_fail to true in the XML above to see the failure path.
  test_config.return_status.reset();
  test_config.return_status_script =
      "(mock_should_fail == true) ? FAILURE : SUCCESS";
  // Convert the node in asynchronous and wait 2000 ms
  test_config.async_delay = std::chrono::milliseconds(2000);
  // Execute a different script depending on the resolved return status.
  test_config.success_script = "msg := 'message SUBSTITUTED' ";
  test_config.failure_script = "msg := 'message FAILURE branch' ";

  // this will be synchronous (async_delay is 0)
  BT::TestNodeConfig counting_config;
  counting_config.return_status = BT::NodeStatus::SUCCESS;

  //---------------------------------------------------------------
  // Next, we want to substitute one or more of out Nodes with this mocks
  // The simplest way is to use a JSON file, otherwise we can do it manually.
  bool const USE_JSON = true;

  if(!USE_JSON)  // manually add substitution rules
  {
    // Substitute nodes which match the wildcard pattern "mysub/action_*"
    // with DummyAction
    factory.addSubstitutionRule("mysub/action_*", "DummyAction");

    // Substitute the node with name "talk" with DummySaySomething
    factory.addSubstitutionRule("talk", "DummySaySomething");

    // Substitute the node with name "set_message" with
    // the a BT::TestNode with the give configuration
    factory.addSubstitutionRule("set_message", test_config);

    // we can also substitute entire branches, for instance the Sequence "counting"
    factory.addSubstitutionRule("counting", counting_config);
  }
  else  // use a JSON file to apply substitution rules programmatically
  {
    // this JSON is equivalent to the code we wrote above
    const char* json_text = R"(
    {
      "TestNodeConfigs": {
        "NewMessage": {
          "async_delay": 2000,
          "return_status_script": "(mock_should_fail == true) ? FAILURE : SUCCESS",
          "success_script": "msg ='message SUBSTITUTED'",
          "failure_script": "msg ='message FAILURE branch'"
        },
        "NoCounting": {
          "return_status": "SUCCESS"
        }
      },

      "SubstitutionRules": {
        "mysub/action_*": "DummyAction",
        "talk": "DummySaySomething",
        "set_message": "NewMessage",
        "counting": "NoCounting"
      }
    })";

    factory.loadSubstitutionRuleFromJSON(json_text);
  }
  //---------------------------------------------------------------
  // IMPORTANT: all substitutions must be done BEFORE creating the tree
  // During the construction phase of the tree, the substitution
  // rules will be used to instantiate the test nodes, instead of the
  // original ones.
  auto tree = factory.createTree("MainTree");
  std::cout << "\n------ Output (substituted) ------\n";
  tree.tickWhileRunning();

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
report_mock_message
Sequence::9
set_message
SaySomething::11
SaySomething::12
counting
SaySomething::14
SaySomething::15
SaySomething::16

------ Output (original) ------
Robot says: hello world
Robot says: the original message
Robot says: 1
Robot says: 2
Robot says: 3

------ Output (substituted) ------
DummySaySomething: hello world
DummyAction substituting node with fullPath(): mysub/action_subA
DummyAction substituting node with fullPath(): mysub/action_subB
Robot says: message SUBSTITUTED

*/
