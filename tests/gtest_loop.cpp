/* Copyright (C) 2018-2025 Davide Faconti -  All Rights Reserved
*
*   Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"),
*   to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
*   and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
*   The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
*
*   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
*   WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "behaviortree_cpp/bt_factory.h"
#include "behaviortree_cpp/decorators/loop_node.h"

#include <gtest/gtest.h>

using namespace BT;

// ============ LoopNode with static queue (string parsed) ============

TEST(LoopNode, StaticIntQueue)
{
  BehaviorTreeFactory factory;

  std::vector<int> received_values;
  PortsList ports = { InputPort<int>("value") };
  factory.registerSimpleAction(
      "RecordIntValue",
      [&received_values](TreeNode& node) {
        auto val = node.getInput<int>("value");
        if(val)
        {
          received_values.push_back(val.value());
        }
        return NodeStatus::SUCCESS;
      },
      ports);

  const std::string xml_text = R"(
    <root BTCPP_format="4">
       <BehaviorTree>
          <LoopInt queue="1;2;3;4;5" value="{val}">
            <RecordIntValue value="{val}"/>
          </LoopInt>
       </BehaviorTree>
    </root>)";

  auto tree = factory.createTreeFromText(xml_text);
  auto status = tree.tickWhileRunning();

  ASSERT_EQ(status, NodeStatus::SUCCESS);
  ASSERT_EQ(received_values.size(), 5u);
  EXPECT_EQ(received_values[0], 1);
  EXPECT_EQ(received_values[1], 2);
  EXPECT_EQ(received_values[2], 3);
  EXPECT_EQ(received_values[3], 4);
  EXPECT_EQ(received_values[4], 5);
}

TEST(LoopNode, StaticDoubleQueue)
{
  BehaviorTreeFactory factory;

  std::vector<double> received_values;
  PortsList ports = { InputPort<double>("value") };
  factory.registerSimpleAction(
      "RecordDoubleValue",
      [&received_values](TreeNode& node) {
        auto val = node.getInput<double>("value");
        if(val)
        {
          received_values.push_back(val.value());
        }
        return NodeStatus::SUCCESS;
      },
      ports);

  const std::string xml_text = R"(
    <root BTCPP_format="4">
       <BehaviorTree>
          <LoopDouble queue="1.5;2.5;3.5" value="{val}">
            <RecordDoubleValue value="{val}"/>
          </LoopDouble>
       </BehaviorTree>
    </root>)";

  auto tree = factory.createTreeFromText(xml_text);
  auto status = tree.tickWhileRunning();

  ASSERT_EQ(status, NodeStatus::SUCCESS);
  ASSERT_EQ(received_values.size(), 3u);
  EXPECT_DOUBLE_EQ(received_values[0], 1.5);
  EXPECT_DOUBLE_EQ(received_values[1], 2.5);
  EXPECT_DOUBLE_EQ(received_values[2], 3.5);
}

TEST(LoopNode, StaticStringQueue)
{
  BehaviorTreeFactory factory;

  std::vector<std::string> received_values;
  PortsList ports = { InputPort<std::string>("value") };
  factory.registerSimpleAction(
      "RecordStringValue",
      [&received_values](TreeNode& node) {
        auto val = node.getInput<std::string>("value");
        if(val)
        {
          received_values.push_back(val.value());
        }
        return NodeStatus::SUCCESS;
      },
      ports);

  const std::string xml_text = R"(
    <root BTCPP_format="4">
       <BehaviorTree>
          <LoopString queue="hello;world;test" value="{val}">
            <RecordStringValue value="{val}"/>
          </LoopString>
       </BehaviorTree>
    </root>)";

  auto tree = factory.createTreeFromText(xml_text);
  auto status = tree.tickWhileRunning();

  ASSERT_EQ(status, NodeStatus::SUCCESS);
  ASSERT_EQ(received_values.size(), 3u);
  EXPECT_EQ(received_values[0], "hello");
  EXPECT_EQ(received_values[1], "world");
  EXPECT_EQ(received_values[2], "test");
}

// ============ LoopNode with empty queue ============

TEST(LoopNode, EmptyQueue_ReturnsSuccess)
{
  BehaviorTreeFactory factory;

  int tick_count = 0;
  factory.registerSimpleAction("CountTicks", [&tick_count](TreeNode&) {
    tick_count++;
    return NodeStatus::SUCCESS;
  });

  const std::string xml_text = R"(
    <root BTCPP_format="4">
       <BehaviorTree>
          <LoopInt queue="" if_empty="SUCCESS" value="{val}">
            <CountTicks/>
          </LoopInt>
       </BehaviorTree>
    </root>)";

  auto tree = factory.createTreeFromText(xml_text);
  auto status = tree.tickWhileRunning();

  ASSERT_EQ(status, NodeStatus::SUCCESS);
  ASSERT_EQ(tick_count, 0);  // Child should never be ticked
}

TEST(LoopNode, EmptyQueue_ReturnsFailure)
{
  BehaviorTreeFactory factory;

  int tick_count = 0;
  factory.registerSimpleAction("CountTicks", [&tick_count](TreeNode&) {
    tick_count++;
    return NodeStatus::SUCCESS;
  });

  const std::string xml_text = R"(
    <root BTCPP_format="4">
       <BehaviorTree>
          <LoopInt queue="" if_empty="FAILURE" value="{val}">
            <CountTicks/>
          </LoopInt>
       </BehaviorTree>
    </root>)";

  auto tree = factory.createTreeFromText(xml_text);
  auto status = tree.tickWhileRunning();

  ASSERT_EQ(status, NodeStatus::FAILURE);
  ASSERT_EQ(tick_count, 0);
}

TEST(LoopNode, EmptyQueue_ReturnsSkipped)
{
  BehaviorTreeFactory factory;

  int tick_count = 0;
  factory.registerSimpleAction("CountTicks", [&tick_count](TreeNode&) {
    tick_count++;
    return NodeStatus::SUCCESS;
  });

  const std::string xml_text = R"(
    <root BTCPP_format="4">
       <BehaviorTree>
          <LoopInt queue="" if_empty="SKIPPED" value="{val}">
            <CountTicks/>
          </LoopInt>
       </BehaviorTree>
    </root>)";

  auto tree = factory.createTreeFromText(xml_text);
  auto status = tree.tickWhileRunning();

  ASSERT_EQ(status, NodeStatus::SKIPPED);
  ASSERT_EQ(tick_count, 0);
}

// ============ LoopNode with child failure ============

TEST(LoopNode, ChildFailure_StopsLoop)
{
  BehaviorTreeFactory factory;

  int tick_count = 0;
  factory.registerSimpleAction("FailOnThird", [&tick_count](TreeNode&) {
    tick_count++;
    if(tick_count == 3)
    {
      return NodeStatus::FAILURE;
    }
    return NodeStatus::SUCCESS;
  });

  const std::string xml_text = R"(
    <root BTCPP_format="4">
       <BehaviorTree>
          <LoopInt queue="1;2;3;4;5" value="{val}">
            <FailOnThird/>
          </LoopInt>
       </BehaviorTree>
    </root>)";

  auto tree = factory.createTreeFromText(xml_text);
  auto status = tree.tickWhileRunning();

  ASSERT_EQ(status, NodeStatus::FAILURE);
  ASSERT_EQ(tick_count, 3);  // Loop should stop at third iteration
}

// ============ LoopNode with dynamic queue from blackboard ============

TEST(LoopNode, DynamicQueueFromBlackboard)
{
  BehaviorTreeFactory factory;

  std::vector<int> received_values;
  PortsList ports = { InputPort<int>("value") };
  factory.registerSimpleAction(
      "RecordIntValue",
      [&received_values](TreeNode& node) {
        auto val = node.getInput<int>("value");
        if(val)
        {
          received_values.push_back(val.value());
        }
        return NodeStatus::SUCCESS;
      },
      ports);

  const std::string xml_text = R"(
    <root BTCPP_format="4">
       <BehaviorTree>
          <LoopInt queue="{my_queue}" value="{val}">
            <RecordIntValue value="{val}"/>
          </LoopInt>
       </BehaviorTree>
    </root>)";

  auto tree = factory.createTreeFromText(xml_text);

  // Set up queue in blackboard
  auto queue = std::make_shared<std::deque<int>>();
  queue->push_back(10);
  queue->push_back(20);
  queue->push_back(30);
  tree.rootBlackboard()->set("my_queue", queue);

  auto status = tree.tickWhileRunning();

  ASSERT_EQ(status, NodeStatus::SUCCESS);
  ASSERT_EQ(received_values.size(), 3u);
  EXPECT_EQ(received_values[0], 10);
  EXPECT_EQ(received_values[1], 20);
  EXPECT_EQ(received_values[2], 30);
}

// ============ LoopNode with vector input (Issue #969) ============

TEST(LoopNode, VectorInput_Issue969)
{
  BehaviorTreeFactory factory;

  std::vector<int> received_values;
  PortsList ports = { InputPort<int>("value") };
  factory.registerSimpleAction(
      "RecordIntValue",
      [&received_values](TreeNode& node) {
        auto val = node.getInput<int>("value");
        if(val)
        {
          received_values.push_back(val.value());
        }
        return NodeStatus::SUCCESS;
      },
      ports);

  const std::string xml_text = R"(
    <root BTCPP_format="4">
       <BehaviorTree>
          <LoopInt queue="{my_vector}" value="{val}">
            <RecordIntValue value="{val}"/>
          </LoopInt>
       </BehaviorTree>
    </root>)";

  auto tree = factory.createTreeFromText(xml_text);

  // Set up vector in blackboard (should be converted to deque)
  std::vector<int> vec = { 100, 200, 300 };
  tree.rootBlackboard()->set("my_vector", vec);

  auto status = tree.tickWhileRunning();

  ASSERT_EQ(status, NodeStatus::SUCCESS);
  ASSERT_EQ(received_values.size(), 3u);
  EXPECT_EQ(received_values[0], 100);
  EXPECT_EQ(received_values[1], 200);
  EXPECT_EQ(received_values[2], 300);
}

// ============ LoopNode with bool queue ============

TEST(LoopNode, BoolQueue)
{
  BehaviorTreeFactory factory;

  std::vector<bool> received_values;
  PortsList ports = { InputPort<bool>("value") };
  factory.registerSimpleAction(
      "RecordBoolValue",
      [&received_values](TreeNode& node) {
        auto val = node.getInput<bool>("value");
        if(val)
        {
          received_values.push_back(val.value());
        }
        return NodeStatus::SUCCESS;
      },
      ports);

  const std::string xml_text = R"(
    <root BTCPP_format="4">
       <BehaviorTree>
          <LoopBool queue="true;false;true" value="{val}">
            <RecordBoolValue value="{val}"/>
          </LoopBool>
       </BehaviorTree>
    </root>)";

  auto tree = factory.createTreeFromText(xml_text);
  auto status = tree.tickWhileRunning();

  ASSERT_EQ(status, NodeStatus::SUCCESS);
  ASSERT_EQ(received_values.size(), 3u);
  EXPECT_TRUE(received_values[0]);
  EXPECT_FALSE(received_values[1]);
  EXPECT_TRUE(received_values[2]);
}

// ============ LoopNode restart behavior ============

TEST(LoopNode, RestartAfterCompletion)
{
  BehaviorTreeFactory factory;

  int tick_count = 0;
  factory.registerSimpleAction("CountTicks", [&tick_count](TreeNode&) {
    tick_count++;
    return NodeStatus::SUCCESS;
  });

  const std::string xml_text = R"(
    <root BTCPP_format="4">
       <BehaviorTree>
          <LoopInt queue="1;2;3" value="{val}">
            <CountTicks/>
          </LoopInt>
       </BehaviorTree>
    </root>)";

  auto tree = factory.createTreeFromText(xml_text);

  // First execution
  auto status = tree.tickWhileRunning();
  ASSERT_EQ(status, NodeStatus::SUCCESS);
  ASSERT_EQ(tick_count, 3);

  // Reset and execute again
  tree.haltTree();
  tick_count = 0;
  status = tree.tickWhileRunning();
  ASSERT_EQ(status, NodeStatus::SUCCESS);
  ASSERT_EQ(tick_count, 3);  // Should iterate over queue again
}

// ============ convertFromString tests for SharedQueue ============

TEST(LoopNode, ConvertFromString_Int)
{
  auto queue = convertFromString<SharedQueue<int>>("1;2;3;4;5");
  ASSERT_NE(queue, nullptr);
  ASSERT_EQ(queue->size(), 5u);
  EXPECT_EQ(queue->at(0), 1);
  EXPECT_EQ(queue->at(4), 5);
}

TEST(LoopNode, ConvertFromString_Double)
{
  auto queue = convertFromString<SharedQueue<double>>("1.1;2.2;3.3");
  ASSERT_NE(queue, nullptr);
  ASSERT_EQ(queue->size(), 3u);
  EXPECT_DOUBLE_EQ(queue->at(0), 1.1);
  EXPECT_DOUBLE_EQ(queue->at(2), 3.3);
}

TEST(LoopNode, ConvertFromString_Bool)
{
  auto queue = convertFromString<SharedQueue<bool>>("true;false;true;false");
  ASSERT_NE(queue, nullptr);
  ASSERT_EQ(queue->size(), 4u);
  EXPECT_TRUE(queue->at(0));
  EXPECT_FALSE(queue->at(1));
}

TEST(LoopNode, ConvertFromString_String)
{
  auto queue = convertFromString<SharedQueue<std::string>>("foo;bar;baz");
  ASSERT_NE(queue, nullptr);
  ASSERT_EQ(queue->size(), 3u);
  EXPECT_EQ(queue->at(0), "foo");
  EXPECT_EQ(queue->at(1), "bar");
  EXPECT_EQ(queue->at(2), "baz");
}
