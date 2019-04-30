/* Copyright (C) 2015-2019 Michele Colledanchise - All Rights Reserved
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
#include "behaviortree_cpp/loggers/bt_cout_logger.h"
#include "behaviortree_cpp/bt_factory.h"

using BT::NodeStatus;
using std::chrono::milliseconds;


struct ReactiveSequenceTest : testing::Test
{
    BT::ReactiveSequence root;
    BT::ConditionTestNode condition_1;
    BT::ConditionTestNode condition_2;
    BT::AsyncActionTest action_1;

    ReactiveSequenceTest()
      : root("root_first")
      , condition_1("condition_1")
      , condition_2("condition_2")
      , action_1("action_1", milliseconds(100) )
    {
        root.addChild(&condition_1);
        root.addChild(&condition_2);
        root.addChild(&action_1);
    }
    ~ReactiveSequenceTest()
    {
        haltAllActions(&root);
    }
};

struct ReactiveSequence2ActionsTest : testing::Test
{
    BT::ReactiveSequence root;
    BT::AsyncActionTest action_1;
    BT::AsyncActionTest action_2;

    ReactiveSequence2ActionsTest()
      : root("root_sequence")
      , action_1("action_1", milliseconds(100))
      , action_2("action_2", milliseconds(100))
    {
        root.addChild(&action_1);
        root.addChild(&action_2);
    }
    ~ReactiveSequence2ActionsTest()
    {
        haltAllActions(&root);
    }
};

struct ComplexReactiveSequence2ActionsTest : testing::Test
{
    BT::ReactiveSequence root;
    BT::AsyncActionTest action_1;
    BT::AsyncActionTest action_2;
    BT::ReactiveSequence seq_1;
    BT::ReactiveSequence seq_2;

    BT::ConditionTestNode condition_1;
    BT::ConditionTestNode condition_2;

    ComplexReactiveSequence2ActionsTest()
      : root("root_sequence")
      , action_1("action_1", milliseconds(100))
      , action_2("action_2", milliseconds(100))
      , seq_1("sequence_1")
      , seq_2("sequence_2")
      , condition_1("condition_1")
      , condition_2("condition_2")
    {
        root.addChild(&seq_1);
        {
            seq_1.addChild(&condition_1);
            seq_1.addChild(&action_1);
        }
        root.addChild(&seq_2);
        {
            seq_2.addChild(&condition_2);
            seq_2.addChild(&action_2);
        }
    }
    ~ComplexReactiveSequence2ActionsTest()
    {
        haltAllActions(&root);
    }
};

/****************TESTS START HERE***************************/

TEST_F(ReactiveSequenceTest, Condition1ToFalse)
{
    condition_1.setBoolean(true);
    condition_2.setBoolean(true);

    BT::NodeStatus state = root.executeTick();

    ASSERT_EQ(NodeStatus::RUNNING, state);
    ASSERT_EQ(NodeStatus::SUCCESS, condition_1.status());
    ASSERT_EQ(NodeStatus::SUCCESS, condition_2.status());
    ASSERT_EQ(NodeStatus::RUNNING, action_1.status());

    condition_1.setBoolean(false);

    state = root.executeTick();

    ASSERT_EQ(NodeStatus::FAILURE, state);
    ASSERT_EQ(NodeStatus::FAILURE, condition_1.status());
    ASSERT_EQ(NodeStatus::IDLE, condition_2.status());
    ASSERT_EQ(NodeStatus::IDLE, action_1.status());
}

TEST_F(ReactiveSequenceTest, Condition2ToFalse)
{
    condition_1.setBoolean(true);
    condition_2.setBoolean(true);

    BT::NodeStatus state = root.executeTick();

    ASSERT_EQ(NodeStatus::RUNNING, state);
    ASSERT_EQ(NodeStatus::SUCCESS, condition_1.status());
    ASSERT_EQ(NodeStatus::SUCCESS, condition_2.status());
    ASSERT_EQ(NodeStatus::RUNNING, action_1.status());

    condition_2.setBoolean(false);

    state = root.executeTick();

    ASSERT_EQ(NodeStatus::FAILURE, state);
    ASSERT_EQ(NodeStatus::SUCCESS, condition_1.status());
    ASSERT_EQ(NodeStatus::FAILURE, condition_2.status());
    ASSERT_EQ(NodeStatus::IDLE, action_1.status());
}

TEST_F(ReactiveSequence2ActionsTest, ConditionsTrue)
{
    root.executeTick();

    ASSERT_EQ(NodeStatus::RUNNING, root.status());
    ASSERT_EQ(NodeStatus::RUNNING, action_1.status());
    ASSERT_EQ(NodeStatus::IDLE, action_2.status());

    std::this_thread::sleep_for(milliseconds(1000));

    ASSERT_EQ(NodeStatus::SUCCESS, action_1.status());

    root.executeTick();

    ASSERT_EQ(NodeStatus::RUNNING, root.status());
    ASSERT_EQ(NodeStatus::IDLE, action_1.status());
    ASSERT_EQ(NodeStatus::RUNNING, action_2.status());

    root.executeTick();

    ASSERT_EQ(NodeStatus::RUNNING, root.status());
    ASSERT_EQ(NodeStatus::RUNNING, action_1.status());
    ASSERT_EQ(NodeStatus::IDLE, action_2.status());

}

TEST_F(ComplexReactiveSequence2ActionsTest, ConditionsTrue)
{
    BT::NodeStatus state = root.executeTick();

    state = root.executeTick();

    ASSERT_EQ(NodeStatus::RUNNING, state);
    ASSERT_EQ(NodeStatus::RUNNING, seq_1.status());
    ASSERT_EQ(NodeStatus::SUCCESS, condition_1.status());
    ASSERT_EQ(NodeStatus::RUNNING, action_1.status());
    ASSERT_EQ(NodeStatus::IDLE, seq_2.status());
    ASSERT_EQ(NodeStatus::IDLE, condition_2.status());
    ASSERT_EQ(NodeStatus::IDLE, action_2.status());

    std::this_thread::sleep_for(milliseconds(300));
    state = root.executeTick();

    ASSERT_EQ(NodeStatus::RUNNING, state);
    ASSERT_EQ(NodeStatus::SUCCESS, seq_1.status());
    ASSERT_EQ(NodeStatus::IDLE, condition_1.status());
    ASSERT_EQ(NodeStatus::IDLE, action_1.status());
    ASSERT_EQ(NodeStatus::RUNNING, seq_2.status());
    ASSERT_EQ(NodeStatus::SUCCESS, condition_2.status());
    ASSERT_EQ(NodeStatus::RUNNING, action_2.status());


    std::this_thread::sleep_for(milliseconds(300));
    state = root.executeTick();

    ASSERT_EQ(NodeStatus::RUNNING, state);
    ASSERT_EQ(NodeStatus::RUNNING, seq_1.status());
    ASSERT_EQ(NodeStatus::SUCCESS, condition_1.status());
    ASSERT_EQ(NodeStatus::RUNNING, action_1.status());
    ASSERT_EQ(NodeStatus::IDLE, seq_2.status());
    ASSERT_EQ(NodeStatus::IDLE, condition_2.status());
    ASSERT_EQ(NodeStatus::IDLE, action_2.status());;
}
