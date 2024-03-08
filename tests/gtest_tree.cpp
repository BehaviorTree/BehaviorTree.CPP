/* Copyright (C) 2015-2017 Michele Colledanchise - All Rights Reserved
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

#include <gtest/gtest.h>
#include "action_test_node.h"
#include "condition_test_node.h"
#include "behaviortree_cpp/behavior_tree.h"

#include <sstream>
#include <string>

using BT::NodeStatus;
using std::chrono::milliseconds;

struct BehaviorTreeTest : testing::Test
{
  BT::SequenceNode root;
  BT::AsyncActionTest action_1;
  BT::ConditionTestNode condition_1;
  BT::ConditionTestNode condition_2;

  BT::FallbackNode fal_conditions;

  BehaviorTreeTest()
    : root("root_sequence")
    , action_1("action_1", milliseconds(100))
    , condition_1("condition_1")
    , condition_2("condition_2")
    , fal_conditions("fallback_conditions")
  {
    root.addChild(&fal_conditions);
    {
      fal_conditions.addChild(&condition_1);
      fal_conditions.addChild(&condition_2);
    }
    root.addChild(&action_1);
  }
  ~BehaviorTreeTest()
  {}
};

/****************TESTS START HERE***************************/

TEST_F(BehaviorTreeTest, Condition1ToFalseCondition2True)
{
  condition_1.setExpectedResult(NodeStatus::FAILURE);
  condition_2.setExpectedResult(NodeStatus::SUCCESS);

  BT::NodeStatus state = root.executeTick();

  ASSERT_EQ(NodeStatus::RUNNING, state);
  ASSERT_EQ(NodeStatus::SUCCESS, fal_conditions.status());
  ASSERT_EQ(NodeStatus::IDLE, condition_1.status());
  ASSERT_EQ(NodeStatus::IDLE, condition_2.status());
  ASSERT_EQ(NodeStatus::RUNNING, action_1.status());
}

TEST_F(BehaviorTreeTest, Condition2ToFalseCondition1True)
{
  condition_2.setExpectedResult(NodeStatus::FAILURE);
  condition_1.setExpectedResult(NodeStatus::SUCCESS);

  BT::NodeStatus state = root.executeTick();

  ASSERT_EQ(NodeStatus::RUNNING, state);
  ASSERT_EQ(NodeStatus::SUCCESS, fal_conditions.status());
  ASSERT_EQ(NodeStatus::IDLE, condition_1.status());
  ASSERT_EQ(NodeStatus::IDLE, condition_2.status());
  ASSERT_EQ(NodeStatus::RUNNING, action_1.status());
}

TEST_F(BehaviorTreeTest, PrintWithStream)
{
  // no stream parameter should go to default stream (std::cout)
  BT::printTreeRecursively(&root);

  // verify value for with custom stream parameter
  std::stringstream stream;
  BT::printTreeRecursively(&root, stream);
  const auto string = stream.str();
  std::string line;

  // first line is all dashes
  ASSERT_FALSE(std::getline(stream, line, '\n').fail());
  ASSERT_STREQ("----------------", line.c_str());

  // each line is the name of the node, indented by depth * 3 spaces
  ASSERT_FALSE(std::getline(stream, line, '\n').fail());
  ASSERT_STREQ(root.name().c_str(), line.c_str());

  ASSERT_FALSE(std::getline(stream, line, '\n').fail());
  ASSERT_STREQ(("   " + fal_conditions.name()).c_str(), line.c_str());

  ASSERT_FALSE(std::getline(stream, line, '\n').fail());
  ASSERT_STREQ(("      " + condition_1.name()).c_str(), line.c_str());

  ASSERT_FALSE(std::getline(stream, line, '\n').fail());
  ASSERT_STREQ(("      " + condition_2.name()).c_str(), line.c_str());

  ASSERT_FALSE(std::getline(stream, line, '\n').fail());
  ASSERT_STREQ(("   " + action_1.name()).c_str(), line.c_str());

  // last line is all dashes
  ASSERT_FALSE(std::getline(stream, line, '\n').fail());
  ASSERT_STREQ("----------------", line.c_str());

  // no more lines
  ASSERT_TRUE(std::getline(stream, line, '\n').fail());
}

int main(int argc, char** argv)
{
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
