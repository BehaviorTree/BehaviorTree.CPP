/* Copyright (C) 2020-2025 Davide Faconti -  All Rights Reserved
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

#include "test_helper.hpp"

#include "behaviortree_cpp/bt_factory.h"

#include <gtest/gtest.h>

using BT::NodeStatus;

class WhileDoElseTest : public testing::Test
{
protected:
  BT::BehaviorTreeFactory factory;
  std::array<int, 4> counters;

  void SetUp() override
  {
    RegisterTestTick(factory, "Test", counters);
  }
};

TEST_F(WhileDoElseTest, ConditionTrue_DoBranch)
{
  // When condition is true, execute the "do" branch
  const std::string xml_text = R"(
    <root BTCPP_format="4">
       <BehaviorTree>
          <WhileDoElse>
            <AlwaysSuccess/>  <!-- condition -->
            <TestA/>          <!-- do -->
            <TestB/>          <!-- else -->
          </WhileDoElse>
       </BehaviorTree>
    </root>)";

  auto tree = factory.createTreeFromText(xml_text);
  auto status = tree.tickWhileRunning();

  ASSERT_EQ(status, NodeStatus::SUCCESS);
  ASSERT_EQ(counters[0], 1);  // TestA executed
  ASSERT_EQ(counters[1], 0);  // TestB not executed
}

TEST_F(WhileDoElseTest, ConditionFalse_ElseBranch)
{
  // When condition is false, execute the "else" branch
  const std::string xml_text = R"(
    <root BTCPP_format="4">
       <BehaviorTree>
          <WhileDoElse>
            <AlwaysFailure/>  <!-- condition -->
            <TestA/>          <!-- do -->
            <TestB/>          <!-- else -->
          </WhileDoElse>
       </BehaviorTree>
    </root>)";

  auto tree = factory.createTreeFromText(xml_text);
  auto status = tree.tickWhileRunning();

  ASSERT_EQ(status, NodeStatus::SUCCESS);
  ASSERT_EQ(counters[0], 0);  // TestA not executed
  ASSERT_EQ(counters[1], 1);  // TestB executed
}

TEST_F(WhileDoElseTest, ConditionFalse_TwoChildren_ReturnsFailure)
{
  // With only 2 children and condition false, return FAILURE
  const std::string xml_text = R"(
    <root BTCPP_format="4">
       <BehaviorTree>
          <WhileDoElse>
            <AlwaysFailure/>  <!-- condition -->
            <TestA/>          <!-- do -->
          </WhileDoElse>
       </BehaviorTree>
    </root>)";

  auto tree = factory.createTreeFromText(xml_text);
  auto status = tree.tickWhileRunning();

  ASSERT_EQ(status, NodeStatus::FAILURE);
  ASSERT_EQ(counters[0], 0);  // TestA not executed
}

TEST_F(WhileDoElseTest, DoBranchFails)
{
  // When do-branch fails, WhileDoElse returns FAILURE
  const std::string xml_text = R"(
    <root BTCPP_format="4">
       <BehaviorTree>
          <WhileDoElse>
            <AlwaysSuccess/>  <!-- condition -->
            <AlwaysFailure/>  <!-- do -->
            <TestA/>          <!-- else -->
          </WhileDoElse>
       </BehaviorTree>
    </root>)";

  auto tree = factory.createTreeFromText(xml_text);
  auto status = tree.tickWhileRunning();

  ASSERT_EQ(status, NodeStatus::FAILURE);
  ASSERT_EQ(counters[0], 0);  // TestA (else) not executed
}

TEST_F(WhileDoElseTest, ElseBranchFails)
{
  // When else-branch fails, WhileDoElse returns FAILURE
  const std::string xml_text = R"(
    <root BTCPP_format="4">
       <BehaviorTree>
          <WhileDoElse>
            <AlwaysFailure/>  <!-- condition -->
            <TestA/>          <!-- do -->
            <AlwaysFailure/>  <!-- else -->
          </WhileDoElse>
       </BehaviorTree>
    </root>)";

  auto tree = factory.createTreeFromText(xml_text);
  auto status = tree.tickWhileRunning();

  ASSERT_EQ(status, NodeStatus::FAILURE);
  ASSERT_EQ(counters[0], 0);  // TestA (do) not executed
}

TEST_F(WhileDoElseTest, ConditionChanges_HaltsElse)
{
  // When condition changes from false to true, else branch should be halted
  int condition_counter = 0;
  factory.registerSimpleCondition("ToggleCondition", [&condition_counter](BT::TreeNode&) {
    return (condition_counter++ == 0) ? NodeStatus::FAILURE : NodeStatus::SUCCESS;
  });

  const std::string xml_text = R"(
    <root BTCPP_format="4">
       <BehaviorTree>
          <WhileDoElse>
            <ToggleCondition/>
            <TestA/>
            <TestB/>
          </WhileDoElse>
       </BehaviorTree>
    </root>)";

  auto tree = factory.createTreeFromText(xml_text);

  // First tick - condition false, executes else (TestB)
  auto status = tree.tickWhileRunning();
  ASSERT_EQ(status, NodeStatus::SUCCESS);
  ASSERT_EQ(counters[0], 0);  // TestA not executed
  ASSERT_EQ(counters[1], 1);  // TestB executed
}

TEST_F(WhileDoElseTest, ConditionChanges_HaltsDo)
{
  // When condition changes from true to false, do branch should be halted
  int condition_counter = 0;
  factory.registerSimpleCondition(
      "ToggleCondition2", [&condition_counter](BT::TreeNode&) {
        return (condition_counter++ == 0) ? NodeStatus::SUCCESS : NodeStatus::FAILURE;
      });

  const std::string xml_text = R"(
    <root BTCPP_format="4">
       <BehaviorTree>
          <WhileDoElse>
            <ToggleCondition2/>
            <TestA/>
            <TestB/>
          </WhileDoElse>
       </BehaviorTree>
    </root>)";

  auto tree = factory.createTreeFromText(xml_text);

  // First tick - condition true, executes do (TestA)
  auto status = tree.tickWhileRunning();
  ASSERT_EQ(status, NodeStatus::SUCCESS);
  ASSERT_EQ(counters[0], 1);  // TestA executed
  ASSERT_EQ(counters[1], 0);  // TestB not executed
}

TEST_F(WhileDoElseTest, HaltBehavior)
{
  // Test that halt resets the node properly
  const std::string xml_text = R"(
    <root BTCPP_format="4">
       <BehaviorTree>
          <WhileDoElse>
            <AlwaysSuccess/>
            <TestA/>
            <TestB/>
          </WhileDoElse>
       </BehaviorTree>
    </root>)";

  auto tree = factory.createTreeFromText(xml_text);

  // First execution
  auto status = tree.tickWhileRunning();
  ASSERT_EQ(status, NodeStatus::SUCCESS);
  ASSERT_EQ(counters[0], 1);

  // Halt and re-execute
  tree.haltTree();
  status = tree.tickWhileRunning();
  ASSERT_EQ(status, NodeStatus::SUCCESS);
  ASSERT_EQ(counters[0], 2);  // TestA executed again
}

TEST_F(WhileDoElseTest, InvalidChildCount_One)
{
  // WhileDoElse with only 1 child should throw
  const std::string xml_text = R"(
    <root BTCPP_format="4">
       <BehaviorTree>
          <WhileDoElse>
            <AlwaysSuccess/>
          </WhileDoElse>
       </BehaviorTree>
    </root>)";

  auto tree = factory.createTreeFromText(xml_text);
  // Throws LogicError, wrapped in NodeExecutionError with backtrace
  ASSERT_THROW(tree.tickWhileRunning(), BT::BehaviorTreeException);
}

TEST_F(WhileDoElseTest, InvalidChildCount_Four)
{
  // WhileDoElse with 4 children should throw
  const std::string xml_text = R"(
    <root BTCPP_format="4">
       <BehaviorTree>
          <WhileDoElse>
            <AlwaysSuccess/>
            <TestA/>
            <TestB/>
            <TestC/>
          </WhileDoElse>
       </BehaviorTree>
    </root>)";

  auto tree = factory.createTreeFromText(xml_text);
  // Throws LogicError, wrapped in NodeExecutionError with backtrace
  ASSERT_THROW(tree.tickWhileRunning(), BT::BehaviorTreeException);
}

TEST_F(WhileDoElseTest, ConditionRunning)
{
  // Test behavior when condition returns RUNNING
  bool first_tick = true;
  factory.registerSimpleCondition("RunningThenSuccess", [&first_tick](BT::TreeNode&) {
    if(first_tick)
    {
      first_tick = false;
      return NodeStatus::RUNNING;
    }
    return NodeStatus::SUCCESS;
  });

  const std::string xml_text = R"(
    <root BTCPP_format="4">
       <BehaviorTree>
          <WhileDoElse>
            <RunningThenSuccess/>
            <TestA/>
            <TestB/>
          </WhileDoElse>
       </BehaviorTree>
    </root>)";

  auto tree = factory.createTreeFromText(xml_text);

  // First tick - condition returns RUNNING
  auto status = tree.tickOnce();
  ASSERT_EQ(status, NodeStatus::RUNNING);
  ASSERT_EQ(counters[0], 0);  // TestA not executed yet
  ASSERT_EQ(counters[1], 0);  // TestB not executed yet

  // Second tick - condition returns SUCCESS, executes do branch
  status = tree.tickWhileRunning();
  ASSERT_EQ(status, NodeStatus::SUCCESS);
  ASSERT_EQ(counters[0], 1);  // TestA executed
}
