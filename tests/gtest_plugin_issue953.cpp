/**
 * Test for Issue #953: convertFromString specialization in plugins not visible
 * to main application.
 *
 * This test loads a plugin (plugin_issue953.so) that defines:
 * - A custom type (Issue953Type)
 * - The convertFromString<Issue953Type> specialization (ONLY in the plugin)
 * - An action node that uses getInput<Issue953Type>()
 *
 * The key point: this test file does NOT have access to the convertFromString
 * specialization. Before the fix, getInput() would fail. After the fix, it
 * works because the StringConverter is stored in PortInfo.
 */

#include "behaviortree_cpp/bt_factory.h"

#include <filesystem>

#include <gtest/gtest.h>

using namespace BT;

// Plugin path is defined at compile time via CMake
#ifndef BT_PLUGIN_ISSUE953_PATH
#define BT_PLUGIN_ISSUE953_PATH "plugin_issue953.so"
#endif

class PluginIssue953Test : public testing::Test
{
protected:
  void SetUp() override
  {
    plugin_path_ = BT_PLUGIN_ISSUE953_PATH;

    if(!std::filesystem::exists(plugin_path_))
    {
      GTEST_SKIP() << "Plugin not found at: " << plugin_path_ << ". "
                   << "Make sure it's built before running this test.";
    }
  }

  std::string plugin_path_;
};

// Test that getInput works for a custom type defined only in the plugin
TEST_F(PluginIssue953Test, GetInputUsesStoredConverter)
{
  // This XML uses a literal string value for the input port
  const char* xml_text = R"(
    <root BTCPP_format="4">
      <BehaviorTree ID="MainTree">
        <Issue953Action input="42;hello_world;3.14159"/>
      </BehaviorTree>
    </root>
  )";

  BehaviorTreeFactory factory;

  // Load the plugin - this registers Issue953Action
  // The plugin has the convertFromString<Issue953Type> specialization,
  // but THIS file does not.
  factory.registerFromPlugin(plugin_path_);

  auto tree = factory.createTreeFromText(xml_text);

  // This should work because:
  // 1. InputPort<Issue953Type>() was called in the plugin
  // 2. GetAnyFromStringFunctor captured convertFromString at that point
  // 3. The fix makes getInput() use that stored converter
  auto status = tree.tickWhileRunning();

  ASSERT_EQ(status, NodeStatus::SUCCESS);

  // Verify the parsed values via output ports
  auto bb = tree.rootBlackboard();
  EXPECT_EQ(bb->get<int>("out_id"), 42);
  EXPECT_EQ(bb->get<std::string>("out_name"), "hello_world");
  EXPECT_DOUBLE_EQ(bb->get<double>("out_value"), 3.14159);
}

// Test with blackboard - value stored as string, then parsed on read
TEST_F(PluginIssue953Test, GetInputFromBlackboardString)
{
  const char* xml_text = R"(
    <root BTCPP_format="4">
      <BehaviorTree ID="MainTree">
        <Sequence>
          <Script code="my_data := '99;from_script;2.718'" />
          <Issue953Action input="{my_data}"/>
        </Sequence>
      </BehaviorTree>
    </root>
  )";

  BehaviorTreeFactory factory;
  factory.registerFromPlugin(plugin_path_);

  auto tree = factory.createTreeFromText(xml_text);
  auto status = tree.tickWhileRunning();

  ASSERT_EQ(status, NodeStatus::SUCCESS);

  auto bb = tree.rootBlackboard();
  EXPECT_EQ(bb->get<int>("out_id"), 99);
  EXPECT_EQ(bb->get<std::string>("out_name"), "from_script");
  EXPECT_DOUBLE_EQ(bb->get<double>("out_value"), 2.718);
}

// Test with SubTree port remapping
TEST_F(PluginIssue953Test, GetInputViaSubtreeRemapping)
{
  const char* xml_text = R"(
    <root BTCPP_format="4" main_tree_to_execute="MainTree">
      <BehaviorTree ID="MainTree">
        <SubTree ID="Issue953SubTree" data="123;subtree_test;1.5"/>
      </BehaviorTree>

      <BehaviorTree ID="Issue953SubTree">
        <Issue953Action input="{data}"/>
      </BehaviorTree>
    </root>
  )";

  BehaviorTreeFactory factory;
  factory.registerFromPlugin(plugin_path_);

  auto tree = factory.createTreeFromText(xml_text);
  auto status = tree.tickWhileRunning();

  ASSERT_EQ(status, NodeStatus::SUCCESS);

  // Get the subtree's blackboard to check output
  auto subtree_bb = tree.subtrees[1]->blackboard;
  EXPECT_EQ(subtree_bb->get<int>("out_id"), 123);
  EXPECT_EQ(subtree_bb->get<std::string>("out_name"), "subtree_test");
  EXPECT_DOUBLE_EQ(subtree_bb->get<double>("out_value"), 1.5);
}
