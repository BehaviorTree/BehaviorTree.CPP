/* Copyright (C) 2018-2025 Davide Faconti, Eurecat -  All Rights Reserved
*
*   Permission is hereby granted, free of charge, to any person obtaining a copy of this
*   software and associated documentation files (the "Software"), to deal in the Software
*   without restriction, including without limitation the rights to use, copy, modify,
*   merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
*   permit persons to whom the Software is furnished to do so, subject to the following
*   conditions: The above copyright notice and this permission notice shall be included in
*   all copies or substantial portions of the Software.
*
*   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
*   INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
*   PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
*   HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
*   CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
*   OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "behaviortree_cpp/bt_factory.h"

#include <gtest/gtest.h>

using namespace BT;

// Test node that throws an exception
class ThrowingAction : public SyncActionNode
{
public:
  ThrowingAction(const std::string& name, const NodeConfig& config)
    : SyncActionNode(name, config)
  {}

  NodeStatus tick() override
  {
    throw std::runtime_error("Test exception from ThrowingAction");
  }

  static PortsList providedPorts()
  {
    return {};
  }
};

// Test node that succeeds
class SucceedingAction : public SyncActionNode
{
public:
  SucceedingAction(const std::string& name, const NodeConfig& config)
    : SyncActionNode(name, config)
  {}

  NodeStatus tick() override
  {
    return NodeStatus::SUCCESS;
  }

  static PortsList providedPorts()
  {
    return {};
  }
};

TEST(ExceptionTracking, BasicExceptionCapture)
{
  // Simple tree: Sequence -> ThrowingAction
  const char* xml = R"(
    <root BTCPP_format="4">
      <BehaviorTree ID="MainTree">
        <ThrowingAction name="thrower"/>
      </BehaviorTree>
    </root>
  )";

  BehaviorTreeFactory factory;
  factory.registerNodeType<ThrowingAction>("ThrowingAction");

  auto tree = factory.createTreeFromText(xml);

  try
  {
    tree.tickOnce();
    FAIL() << "Expected NodeExecutionError to be thrown";
  }
  catch(const NodeExecutionError& e)
  {
    // Verify the failed node info
    EXPECT_EQ(e.failedNode().node_name, "thrower");
    EXPECT_EQ(e.failedNode().registration_name, "ThrowingAction");
    EXPECT_EQ(e.originalMessage(), "Test exception from ThrowingAction");
  }
}

TEST(ExceptionTracking, NestedExceptionBacktrace)
{
  // Tree: Sequence -> RetryNode -> ThrowingAction
  // This tests that the backtrace shows the full path
  const char* xml = R"(
    <root BTCPP_format="4">
      <BehaviorTree ID="MainTree">
        <Sequence name="main_seq">
          <SucceedingAction name="first"/>
          <RetryUntilSuccessful num_attempts="1" name="retry">
            <ThrowingAction name="nested_thrower"/>
          </RetryUntilSuccessful>
        </Sequence>
      </BehaviorTree>
    </root>
  )";

  BehaviorTreeFactory factory;
  factory.registerNodeType<ThrowingAction>("ThrowingAction");
  factory.registerNodeType<SucceedingAction>("SucceedingAction");

  auto tree = factory.createTreeFromText(xml);

  try
  {
    tree.tickOnce();
    FAIL() << "Expected NodeExecutionError to be thrown";
  }
  catch(const NodeExecutionError& e)
  {
    // Verify the failed node is the innermost throwing node
    EXPECT_EQ(e.failedNode().node_name, "nested_thrower");

    // Check the what() message contains the failing node
    std::string what_msg = e.what();
    EXPECT_NE(what_msg.find("nested_thrower"), std::string::npos);
  }
}

TEST(ExceptionTracking, SubtreeExceptionBacktrace)
{
  // Tree with subtree: MainTree -> Subtree -> ThrowingAction
  const char* xml = R"(
    <root BTCPP_format="4" main_tree_to_execute="MainTree">
      <BehaviorTree ID="MainTree">
        <Sequence name="outer_seq">
          <SubTree ID="InnerTree" name="subtree_call"/>
        </Sequence>
      </BehaviorTree>
      <BehaviorTree ID="InnerTree">
        <Sequence name="inner_seq">
          <ThrowingAction name="subtree_thrower"/>
        </Sequence>
      </BehaviorTree>
    </root>
  )";

  BehaviorTreeFactory factory;
  factory.registerNodeType<ThrowingAction>("ThrowingAction");

  auto tree = factory.createTreeFromText(xml);

  try
  {
    tree.tickOnce();
    FAIL() << "Expected NodeExecutionError to be thrown";
  }
  catch(const NodeExecutionError& e)
  {
    // Verify the failed node is the one in the subtree
    EXPECT_EQ(e.failedNode().node_name, "subtree_thrower");

    // Verify fullPath includes the subtree hierarchy
    std::string full_path = e.failedNode().node_path;
    EXPECT_NE(full_path.find("subtree_thrower"), std::string::npos);
  }
}

TEST(ExceptionTracking, NoExceptionNoWrapping)
{
  // Verify that trees that don't throw work normally
  const char* xml = R"(
    <root BTCPP_format="4">
      <BehaviorTree ID="MainTree">
        <Sequence>
          <SucceedingAction name="a"/>
          <SucceedingAction name="b"/>
        </Sequence>
      </BehaviorTree>
    </root>
  )";

  BehaviorTreeFactory factory;
  factory.registerNodeType<SucceedingAction>("SucceedingAction");

  auto tree = factory.createTreeFromText(xml);

  // Should not throw
  EXPECT_NO_THROW({
    auto status = tree.tickOnce();
    EXPECT_EQ(status, NodeStatus::SUCCESS);
  });
}

TEST(ExceptionTracking, BacktraceEntryContents)
{
  // Test that TickBacktraceEntry contains all expected fields
  const char* xml = R"(
    <root BTCPP_format="4">
      <BehaviorTree ID="MainTree">
        <ThrowingAction name="my_action"/>
      </BehaviorTree>
    </root>
  )";

  BehaviorTreeFactory factory;
  factory.registerNodeType<ThrowingAction>("ThrowingAction");

  auto tree = factory.createTreeFromText(xml);

  try
  {
    tree.tickOnce();
    FAIL() << "Expected exception";
  }
  catch(const NodeExecutionError& e)
  {
    const auto& entry = e.failedNode();
    // Check all fields are populated
    EXPECT_FALSE(entry.node_name.empty());
    EXPECT_FALSE(entry.node_path.empty());
    EXPECT_FALSE(entry.registration_name.empty());

    EXPECT_EQ(entry.node_name, "my_action");
    EXPECT_EQ(entry.registration_name, "ThrowingAction");
  }
}
