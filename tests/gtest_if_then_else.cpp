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

class IfThenElseTest : public testing::Test
{
protected:
  BT::BehaviorTreeFactory factory;
  std::array<int, 4> counters;

  void SetUp() override
  {
    RegisterTestTick(factory, "Test", counters);
  }
};

TEST_F(IfThenElseTest, ConditionTrue_ThenBranch)
{
  // When condition is true, execute the "then" branch
  const std::string xml_text = R"(
    <root BTCPP_format="4">
       <BehaviorTree>
          <IfThenElse>
            <AlwaysSuccess/>  <!-- condition -->
            <TestA/>          <!-- then -->
            <TestB/>          <!-- else -->
          </IfThenElse>
       </BehaviorTree>
    </root>)";

  auto tree = factory.createTreeFromText(xml_text);
  auto status = tree.tickWhileRunning();

  ASSERT_EQ(status, NodeStatus::SUCCESS);
  ASSERT_EQ(counters[0], 1);  // TestA executed
  ASSERT_EQ(counters[1], 0);  // TestB not executed
}

TEST_F(IfThenElseTest, ConditionFalse_ElseBranch)
{
  // When condition is false, execute the "else" branch
  const std::string xml_text = R"(
    <root BTCPP_format="4">
       <BehaviorTree>
          <IfThenElse>
            <AlwaysFailure/>  <!-- condition -->
            <TestA/>          <!-- then -->
            <TestB/>          <!-- else -->
          </IfThenElse>
       </BehaviorTree>
    </root>)";

  auto tree = factory.createTreeFromText(xml_text);
  auto status = tree.tickWhileRunning();

  ASSERT_EQ(status, NodeStatus::SUCCESS);
  ASSERT_EQ(counters[0], 0);  // TestA not executed
  ASSERT_EQ(counters[1], 1);  // TestB executed
}

TEST_F(IfThenElseTest, ConditionFalse_TwoChildren_ReturnsFailure)
{
  // With only 2 children and condition false, return FAILURE
  const std::string xml_text = R"(
    <root BTCPP_format="4">
       <BehaviorTree>
          <IfThenElse>
            <AlwaysFailure/>  <!-- condition -->
            <TestA/>          <!-- then -->
          </IfThenElse>
       </BehaviorTree>
    </root>)";

  auto tree = factory.createTreeFromText(xml_text);
  auto status = tree.tickWhileRunning();

  ASSERT_EQ(status, NodeStatus::FAILURE);
  ASSERT_EQ(counters[0], 0);  // TestA not executed
}

TEST_F(IfThenElseTest, ThenBranchFails)
{
  // When then-branch fails, IfThenElse returns FAILURE
  const std::string xml_text = R"(
    <root BTCPP_format="4">
       <BehaviorTree>
          <IfThenElse>
            <AlwaysSuccess/>  <!-- condition -->
            <AlwaysFailure/>  <!-- then -->
            <TestA/>          <!-- else -->
          </IfThenElse>
       </BehaviorTree>
    </root>)";

  auto tree = factory.createTreeFromText(xml_text);
  auto status = tree.tickWhileRunning();

  ASSERT_EQ(status, NodeStatus::FAILURE);
  ASSERT_EQ(counters[0], 0);  // TestA (else) not executed
}

TEST_F(IfThenElseTest, ElseBranchFails)
{
  // When else-branch fails, IfThenElse returns FAILURE
  const std::string xml_text = R"(
    <root BTCPP_format="4">
       <BehaviorTree>
          <IfThenElse>
            <AlwaysFailure/>  <!-- condition -->
            <TestA/>          <!-- then -->
            <AlwaysFailure/>  <!-- else -->
          </IfThenElse>
       </BehaviorTree>
    </root>)";

  auto tree = factory.createTreeFromText(xml_text);
  auto status = tree.tickWhileRunning();

  ASSERT_EQ(status, NodeStatus::FAILURE);
  ASSERT_EQ(counters[0], 0);  // TestA (then) not executed
}

TEST_F(IfThenElseTest, ConditionRunning)
{
  // Test condition that returns RUNNING first, then SUCCESS
  int condition_ticks = 0;
  factory.registerSimpleCondition("RunningThenSuccess",
                                  [&condition_ticks](BT::TreeNode&) {
                                    condition_ticks++;
                                    if(condition_ticks == 1)
                                    {
                                      return NodeStatus::RUNNING;
                                    }
                                    return NodeStatus::SUCCESS;
                                  });

  const std::string xml_text = R"(
    <root BTCPP_format="4">
       <BehaviorTree>
          <IfThenElse>
            <RunningThenSuccess/>
            <TestA/>
            <TestB/>
          </IfThenElse>
       </BehaviorTree>
    </root>)";

  auto tree = factory.createTreeFromText(xml_text);

  // First tick - condition returns RUNNING
  auto status = tree.tickOnce();
  ASSERT_EQ(status, NodeStatus::RUNNING);
  ASSERT_EQ(counters[0], 0);  // TestA not executed yet

  // Second tick - condition returns SUCCESS, then-branch executes
  status = tree.tickWhileRunning();
  ASSERT_EQ(status, NodeStatus::SUCCESS);
  ASSERT_EQ(counters[0], 1);  // TestA executed
}

TEST_F(IfThenElseTest, HaltBehavior)
{
  // Test that halt resets the node properly
  const std::string xml_text = R"(
    <root BTCPP_format="4">
       <BehaviorTree>
          <IfThenElse>
            <AlwaysSuccess/>
            <TestA/>
            <TestB/>
          </IfThenElse>
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

TEST_F(IfThenElseTest, InvalidChildCount_One)
{
  // IfThenElse with only 1 child should throw
  const std::string xml_text = R"(
    <root BTCPP_format="4">
       <BehaviorTree>
          <IfThenElse>
            <AlwaysSuccess/>
          </IfThenElse>
       </BehaviorTree>
    </root>)";

  auto tree = factory.createTreeFromText(xml_text);
  // Throws LogicError, wrapped in NodeExecutionError with backtrace
  ASSERT_THROW(tree.tickWhileRunning(), BT::BehaviorTreeException);
}

TEST_F(IfThenElseTest, InvalidChildCount_Four)
{
  // IfThenElse with 4 children should throw
  const std::string xml_text = R"(
    <root BTCPP_format="4">
       <BehaviorTree>
          <IfThenElse>
            <AlwaysSuccess/>
            <TestA/>
            <TestB/>
            <TestC/>
          </IfThenElse>
       </BehaviorTree>
    </root>)";

  auto tree = factory.createTreeFromText(xml_text);
  // Throws LogicError, wrapped in NodeExecutionError with backtrace
  ASSERT_THROW(tree.tickWhileRunning(), BT::BehaviorTreeException);
}
