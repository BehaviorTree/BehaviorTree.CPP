#include "behaviortree_cpp/bt_factory.h"

#include <future>

#include <gtest/gtest.h>

using namespace BT;

namespace
{

const char* json_text = R"(
{
  "TestNodeConfigs": {
    "TestA": {
      "async_delay": 2000,
      "return_status": "SUCCESS",
      "post_script": "msg ='message SUBSTITUED'"
    },
    "TestB": {
      "return_status": "FAILURE"
    }
  },

  "SubstitutionRules": {
    "actionA": "TestA",
    "actionB": "TestB",
    "actionC": "NotAConfig"
  }
}
 )";

}  // namespace

TEST(Substitution, Parser)
{
  BehaviorTreeFactory factory;

  factory.loadSubstitutionRuleFromJSON(json_text);

  const auto& rules = factory.substitutionRules();

  ASSERT_EQ(rules.size(), 3);
  ASSERT_EQ(rules.count("actionA"), 1);
  ASSERT_EQ(rules.count("actionB"), 1);
  ASSERT_EQ(rules.count("actionC"), 1);

  auto configA = std::get_if<TestNodeConfig>(&rules.at("actionA"));
  ASSERT_EQ(configA->return_status, NodeStatus::SUCCESS);
  ASSERT_EQ(configA->async_delay, std::chrono::milliseconds(2000));
  ASSERT_EQ(configA->post_script, "msg ='message SUBSTITUED'");

  auto configB = std::get_if<TestNodeConfig>(&rules.at("actionB"));
  ASSERT_EQ(configB->return_status, NodeStatus::FAILURE);
  ASSERT_EQ(configB->async_delay, std::chrono::milliseconds(0));
  ASSERT_TRUE(configB->post_script.empty());

  ASSERT_EQ(*std::get_if<std::string>(&rules.at("actionC")), "NotAConfig");
}

// Regression test for issue #934: segfault when substituting a SubTree node
TEST(Substitution, SubTreeNodeSubstitution)
{
  static const char* parent_xml = R"(
  <root BTCPP_format="4">
    <BehaviorTree ID="Parent">
      <SubTree ID="Child" name="child" />
    </BehaviorTree>
  </root>
  )";

  static const char* child_xml = R"(
  <root BTCPP_format="4">
    <BehaviorTree ID="Child">
      <AlwaysSuccess />
    </BehaviorTree>
  </root>
  )";

  BehaviorTreeFactory factory;
  factory.registerBehaviorTreeFromText(parent_xml);
  factory.registerBehaviorTreeFromText(child_xml);

  BT::TestNodeConfig config;
  config.return_status = BT::NodeStatus::SUCCESS;
  factory.addSubstitutionRule("child", config);

  // This should not crash (was a segfault before the fix)
  Tree tree;
  ASSERT_NO_THROW(tree = factory.createTree("Parent"));

  // The substituted tree should tick successfully
  auto status = tree.tickWhileRunning();
  ASSERT_EQ(status, BT::NodeStatus::SUCCESS);
}

// Test for issue #930: Mock substitution with registerSimpleAction
// should not hang when using string-based substitution from JSON.
TEST(Substitution, StringSubstitutionWithSimpleAction_Issue930)
{
  // XML tree: Sequence with a Delay, then an action to be substituted
  static const char* xml_text = R"(
  <root BTCPP_format="4">
    <BehaviorTree ID="MainTree">
      <Sequence>
        <Delay delay_msec="100">
          <AlwaysSuccess/>
        </Delay>
        <SaySomething name="action_to_replace" message="hello"/>
      </Sequence>
    </BehaviorTree>
  </root>
  )";

  BehaviorTreeFactory factory;

  // Register original action
  factory.registerSimpleAction("SaySomething",
                               [](TreeNode& /*node*/) { return NodeStatus::SUCCESS; },
                               { InputPort<std::string>("message") });

  // Register substitute action
  factory.registerSimpleAction("MyTestAction",
                               [](TreeNode&) { return NodeStatus::SUCCESS; });

  // Use string-based substitution: replace action_to_replace with
  // MyTestAction
  factory.addSubstitutionRule("action_to_replace", "MyTestAction");

  factory.registerBehaviorTreeFromText(xml_text);
  auto tree = factory.createTree("MainTree");

  // This should NOT hang. Use a future with timeout to detect hangs.
  auto future =
      std::async(std::launch::async, [&tree]() { return tree.tickWhileRunning(); });

  auto status = future.wait_for(std::chrono::seconds(5));
  ASSERT_NE(status, std::future_status::timeout) << "Tree hung! tickWhileRunning did not "
                                                    "complete within 5 seconds";
  ASSERT_EQ(future.get(), NodeStatus::SUCCESS);
}

// Test for issue #930: TestNodeConfig-based substitution with
// async_delay should also not hang on single-threaded executor.
TEST(Substitution, TestNodeConfigAsyncSubstitution_Issue930)
{
  static const char* xml_text = R"(
  <root BTCPP_format="4">
    <BehaviorTree ID="MainTree">
      <Sequence>
        <AlwaysSuccess name="action_A"/>
        <AlwaysSuccess name="action_B"/>
      </Sequence>
    </BehaviorTree>
  </root>
  )";

  BehaviorTreeFactory factory;
  factory.registerBehaviorTreeFromText(xml_text);

  // Substitute action_B with a TestNodeConfig that has async_delay
  TestNodeConfig test_config;
  test_config.return_status = NodeStatus::SUCCESS;
  test_config.async_delay = std::chrono::milliseconds(100);
  factory.addSubstitutionRule("action_B", test_config);

  auto tree = factory.createTree("MainTree");

  // This should NOT hang -- the TestNode should complete after the
  // async_delay and emit a wake-up signal.
  auto future =
      std::async(std::launch::async, [&tree]() { return tree.tickWhileRunning(); });

  auto status = future.wait_for(std::chrono::seconds(5));
  ASSERT_NE(status, std::future_status::timeout) << "Tree hung! tickWhileRunning did not "
                                                    "complete within 5 seconds";
  ASSERT_EQ(future.get(), NodeStatus::SUCCESS);
}

// Test for issue #930: JSON-based substitution mapping to a
// registered SimpleAction (string rule) should work correctly.
TEST(Substitution, JsonStringSubstitution_Issue930)
{
  static const char* xml_text = R"(
  <root BTCPP_format="4">
    <BehaviorTree ID="MainTree">
      <Sequence>
        <AlwaysSuccess name="action_A"/>
        <AlwaysSuccess name="action_B"/>
      </Sequence>
    </BehaviorTree>
  </root>
  )";

  // JSON that maps action_B to a registered SimpleAction
  // (not a TestNodeConfig name)
  static const char* json_rules = R"(
  {
    "TestNodeConfigs": {},
    "SubstitutionRules": {
      "action_B": "MyReplacement"
    }
  }
  )";

  BehaviorTreeFactory factory;

  // Register the replacement action
  factory.registerSimpleAction("MyReplacement",
                               [](TreeNode&) { return NodeStatus::SUCCESS; });

  factory.loadSubstitutionRuleFromJSON(json_rules);
  factory.registerBehaviorTreeFromText(xml_text);
  auto tree = factory.createTree("MainTree");

  // This should NOT hang
  auto future =
      std::async(std::launch::async, [&tree]() { return tree.tickWhileRunning(); });

  auto status = future.wait_for(std::chrono::seconds(5));
  ASSERT_NE(status, std::future_status::timeout) << "Tree hung! tickWhileRunning did not "
                                                    "complete within 5 seconds";
  ASSERT_EQ(future.get(), NodeStatus::SUCCESS);
}

// Test for issue #930: loadSubstitutionRuleFromJSON should work
// when TestNodeConfigs is empty (only string rules).
TEST(Substitution, JsonWithEmptyTestNodeConfigs_Issue930)
{
  static const char* json_rules = R"(
  {
    "TestNodeConfigs": {},
    "SubstitutionRules": {
      "node_A": "ReplacementNode"
    }
  }
  )";

  BehaviorTreeFactory factory;
  factory.registerSimpleAction("ReplacementNode",
                               [](TreeNode&) { return NodeStatus::SUCCESS; });

  // This should not throw
  ASSERT_NO_THROW(factory.loadSubstitutionRuleFromJSON(json_rules));

  const auto& rules = factory.substitutionRules();
  ASSERT_EQ(rules.size(), 1);
  ASSERT_EQ(rules.count("node_A"), 1);
  auto* rule_str = std::get_if<std::string>(&rules.at("node_A"));
  ASSERT_NE(rule_str, nullptr);
  ASSERT_EQ(*rule_str, "ReplacementNode");
}

// Test for issue #930: loadSubstitutionRuleFromJSON should handle
// missing TestNodeConfigs gracefully.
TEST(Substitution, JsonWithoutTestNodeConfigs_Issue930)
{
  static const char* json_rules = R"(
  {
    "SubstitutionRules": {
      "node_A": "ReplacementNode"
    }
  }
  )";

  BehaviorTreeFactory factory;
  factory.registerSimpleAction("ReplacementNode",
                               [](TreeNode&) { return NodeStatus::SUCCESS; });

  // TestNodeConfigs is optional: string-only substitution rules
  // don't need TestNodeConfigs.
  ASSERT_NO_THROW(factory.loadSubstitutionRuleFromJSON(json_rules));

  const auto& rules = factory.substitutionRules();
  ASSERT_EQ(rules.size(), 1);
  auto* rule_str = std::get_if<std::string>(&rules.at("node_A"));
  ASSERT_NE(rule_str, nullptr);
  ASSERT_EQ(*rule_str, "ReplacementNode");
}

// Test for issue #930: End-to-end test combining JSON-based
// string substitution with tree execution involving async nodes.
// This closely matches the issue reporter's scenario.
TEST(Substitution, JsonStringSubstitutionWithDelay_Issue930)
{
  static const char* xml_text = R"(
  <root BTCPP_format="4">
    <BehaviorTree ID="MainTree">
      <Sequence>
        <Delay delay_msec="50">
          <AlwaysSuccess/>
        </Delay>
        <Script name="script_2" code=" val:=1 "/>
      </Sequence>
    </BehaviorTree>
  </root>
  )";

  static const char* json_rules = R"(
  {
    "SubstitutionRules": {
      "script_2": "MyTest"
    }
  }
  )";

  BehaviorTreeFactory factory;

  bool action_executed = false;
  factory.registerSimpleAction("MyTest", [&](TreeNode&) {
    action_executed = true;
    return NodeStatus::SUCCESS;
  });

  factory.loadSubstitutionRuleFromJSON(json_rules);
  factory.registerBehaviorTreeFromText(xml_text);
  auto tree = factory.createTree("MainTree");

  auto future =
      std::async(std::launch::async, [&tree]() { return tree.tickWhileRunning(); });

  auto status = future.wait_for(std::chrono::seconds(5));
  ASSERT_NE(status, std::future_status::timeout) << "Tree hung! tickWhileRunning did not "
                                                    "complete within "
                                                    "5 seconds";
  ASSERT_EQ(future.get(), NodeStatus::SUCCESS);
  ASSERT_TRUE(action_executed);
}

// Test for issue #930: Verify that substituted node's registration
// ID is preserved correctly for string-based substitution.
TEST(Substitution, StringSubstitutionRegistrationID_Issue930)
{
  static const char* xml_text = R"(
  <root BTCPP_format="4">
    <BehaviorTree ID="MainTree">
      <AlwaysSuccess name="target_node"/>
    </BehaviorTree>
  </root>
  )";

  BehaviorTreeFactory factory;

  factory.registerSimpleAction("MyReplacement",
                               [](TreeNode&) { return NodeStatus::SUCCESS; });

  factory.addSubstitutionRule("target_node", "MyReplacement");
  factory.registerBehaviorTreeFromText(xml_text);
  auto tree = factory.createTree("MainTree");

  // The substituted node should still work correctly
  ASSERT_EQ(tree.tickWhileRunning(), NodeStatus::SUCCESS);
}
