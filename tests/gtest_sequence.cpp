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
#include "behaviortree_cpp_v3/behavior_tree.h"

using BT::NodeStatus;
using std::chrono::milliseconds;

struct SimpleSequenceTest : testing::Test
{
    BT::SequenceNode root;
    BT::AsyncActionTest action;
    BT::ConditionTestNode condition;

    SimpleSequenceTest() :
      root("root_sequence")
      , action("action", milliseconds(100))
      , condition("condition")
    {
        root.addChild(&condition);
        root.addChild(&action);
    }
    ~SimpleSequenceTest()
    {

    }
};

struct ComplexSequenceTest : testing::Test
{
    BT::ReactiveSequence root;
    BT::AsyncActionTest action_1;
    BT::ConditionTestNode condition_1;
    BT::ConditionTestNode condition_2;

    BT::SequenceNode seq_conditions;

    ComplexSequenceTest()
      : root("root")
      , action_1("action_1", milliseconds(100))
      , condition_1("condition_1")
      , condition_2("condition_2")
      , seq_conditions("sequence_conditions")
    {
        root.addChild(&seq_conditions);
        {
            seq_conditions.addChild(&condition_1);
            seq_conditions.addChild(&condition_2);
        }
        root.addChild(&action_1);
    }
    ~ComplexSequenceTest()
    {

    }
};

struct SequenceTripleActionTest : testing::Test
{
    BT::SequenceNode root;
    BT::ConditionTestNode condition;
    BT::AsyncActionTest action_1;
    BT::SyncActionTest action_2;
    BT::AsyncActionTest action_3;

    SequenceTripleActionTest()
      : root("root_sequence")
      , condition("condition")
      , action_1("action_1", milliseconds(100))
      , action_2("action_2")
      , action_3("action_3", milliseconds(100))
    {
        root.addChild(&condition);
        root.addChild(&action_1);
        root.addChild(&action_2);
        root.addChild(&action_3);
    }
    ~SequenceTripleActionTest()
    {

    }
};

struct ComplexSequence2ActionsTest : testing::Test
{
    BT::SequenceNode root;
    BT::AsyncActionTest action_1;
    BT::AsyncActionTest action_2;
    BT::SequenceNode seq_1;
    BT::SequenceNode seq_2;

    BT::ConditionTestNode condition_1;
    BT::ConditionTestNode condition_2;

    ComplexSequence2ActionsTest()
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
    ~ComplexSequence2ActionsTest()
    {

    }
};

struct SimpleSequenceWithMemoryTest : testing::Test
{
    BT::SequenceStarNode root;
    BT::AsyncActionTest action;
    BT::ConditionTestNode condition;

    SimpleSequenceWithMemoryTest() :
      root("root_sequence")
      , action("action", milliseconds(100))
      , condition("condition")
    {
        root.addChild(&condition);
        root.addChild(&action);
    }
    ~SimpleSequenceWithMemoryTest()
    {

    }
};

struct ComplexSequenceWithMemoryTest : testing::Test
{
    BT::SequenceStarNode root;

    BT::AsyncActionTest action_1;
    BT::AsyncActionTest action_2;

    BT::ConditionTestNode condition_1;
    BT::ConditionTestNode condition_2;

    BT::SequenceStarNode seq_conditions;
    BT::SequenceStarNode seq_actions;

    ComplexSequenceWithMemoryTest()
      : root("root_sequence")
      , action_1("action_1", milliseconds(100))
      , action_2("action_2", milliseconds(100))
      , condition_1("condition_1")
      , condition_2("condition_2")
      , seq_conditions("sequence_conditions")
      , seq_actions("sequence_actions")
    {
        root.addChild(&seq_conditions);
        {
            seq_conditions.addChild(&condition_1);
            seq_conditions.addChild(&condition_2);
        }
        root.addChild(&seq_actions);
        {
            seq_actions.addChild(&action_1);
            seq_actions.addChild(&action_2);
        }
    }
    ~ComplexSequenceWithMemoryTest()
    {

    }
};

struct SimpleParallelTest : testing::Test
{
    BT::ParallelNode root;
    BT::AsyncActionTest action_1;
    BT::ConditionTestNode condition_1;

    BT::AsyncActionTest action_2;
    BT::ConditionTestNode condition_2;

    SimpleParallelTest()
      : root("root_parallel", 4)
      , action_1("action_1", milliseconds(100))
      , condition_1("condition_1")
      , action_2("action_2", milliseconds(100))
      , condition_2("condition_2")
    {
        root.addChild(&condition_1);
        root.addChild(&action_1);
        root.addChild(&condition_2);
        root.addChild(&action_2);
    }
    ~SimpleParallelTest()
    {

    }
};

/****************TESTS START HERE***************************/

TEST_F(SimpleSequenceTest, ConditionTrue)
{
    std::cout << "Ticking the root node !" << std::endl << std::endl;
    // Ticking the root node
    BT::NodeStatus state = root.executeTick();

    ASSERT_EQ(NodeStatus::RUNNING, action.status());
    ASSERT_EQ(NodeStatus::RUNNING, state);
}

TEST_F(SimpleSequenceTest, ConditionTurnToFalse)
{
    condition.setExpectedResult(NodeStatus::FAILURE);
    BT::NodeStatus state = root.executeTick();

    state = root.executeTick();
    ASSERT_EQ(NodeStatus::FAILURE, state);
    ASSERT_EQ(NodeStatus::IDLE, condition.status());
    ASSERT_EQ(NodeStatus::IDLE, action.status());
}

TEST_F(ComplexSequenceTest, ComplexSequenceConditionsTrue)
{
    BT::NodeStatus state = root.executeTick();

    ASSERT_EQ(NodeStatus::RUNNING, state);
    ASSERT_EQ(NodeStatus::SUCCESS, seq_conditions.status());
    ASSERT_EQ(NodeStatus::IDLE, condition_1.status());
    ASSERT_EQ(NodeStatus::IDLE, condition_1.status());
    ASSERT_EQ(NodeStatus::RUNNING, action_1.status());
}

TEST_F(SequenceTripleActionTest, TripleAction)
{
    using namespace BT;
    using namespace std::chrono;
    const auto timeout = system_clock::now() + milliseconds(650);

    action_1.setTime( milliseconds(300) );
    action_3.setTime( milliseconds(300) );
    // the sequence is supposed to finish in (300 ms * 2) = 600 ms

    // first tick
    NodeStatus state = root.executeTick();

    ASSERT_EQ(NodeStatus::RUNNING, state);
    ASSERT_EQ(NodeStatus::RUNNING, action_1.status());
    ASSERT_EQ(NodeStatus::IDLE, action_2.status());
    ASSERT_EQ(NodeStatus::IDLE, action_3.status());

    // continue until succesful
    while (state != NodeStatus::SUCCESS && system_clock::now() < timeout)
    {
        std::this_thread::sleep_for(milliseconds(10));
        state = root.executeTick();
    }

    ASSERT_EQ(NodeStatus::SUCCESS, state);

    // Condition is called only once
    ASSERT_EQ(condition.tickCount(), 1);
    // all the actions are called only once
    ASSERT_EQ(action_1.tickCount(), 1);
    ASSERT_EQ(action_2.tickCount(), 1);
    ASSERT_EQ(action_3.tickCount(), 1);

    ASSERT_EQ(NodeStatus::IDLE, action_1.status());
    ASSERT_EQ(NodeStatus::IDLE, action_2.status());
    ASSERT_EQ(NodeStatus::IDLE, action_3.status());
    ASSERT_TRUE(system_clock::now() < timeout);   // no timeout should occur
}

TEST_F(ComplexSequence2ActionsTest, ConditionsTrue)
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

    state = root.executeTick();
}

TEST_F(ComplexSequenceTest, ComplexSequenceConditions1ToFalse)
{
    BT::NodeStatus state = root.executeTick();

    condition_1.setExpectedResult(NodeStatus::FAILURE);

    state = root.executeTick();

    ASSERT_EQ(NodeStatus::FAILURE, state);
    ASSERT_EQ(NodeStatus::IDLE, seq_conditions.status());
    ASSERT_EQ(NodeStatus::IDLE, condition_1.status());
    ASSERT_EQ(NodeStatus::IDLE, condition_2.status());
    ASSERT_EQ(NodeStatus::IDLE, action_1.status());
}

TEST_F(ComplexSequenceTest, ComplexSequenceConditions2ToFalse)
{
    BT::NodeStatus state = root.executeTick();

    condition_2.setExpectedResult(NodeStatus::FAILURE);

    state = root.executeTick();

    ASSERT_EQ(NodeStatus::FAILURE, state);
    ASSERT_EQ(NodeStatus::IDLE, seq_conditions.status());
    ASSERT_EQ(NodeStatus::IDLE, condition_1.status());
    ASSERT_EQ(NodeStatus::IDLE, condition_2.status());
    ASSERT_EQ(NodeStatus::IDLE, action_1.status());
}

TEST_F(SimpleSequenceWithMemoryTest, ConditionTrue)
{
    BT::NodeStatus state = root.executeTick();
    std::this_thread::sleep_for( milliseconds(50) );

    ASSERT_EQ(NodeStatus::RUNNING, state);
    ASSERT_EQ(NodeStatus::SUCCESS, condition.status());
    ASSERT_EQ(NodeStatus::RUNNING, action.status());
}

TEST_F(SimpleSequenceWithMemoryTest, ConditionTurnToFalse)
{
    BT::NodeStatus state = root.executeTick();

    ASSERT_EQ(NodeStatus::RUNNING, state);
    ASSERT_EQ(NodeStatus::SUCCESS, condition.status());
    ASSERT_EQ(NodeStatus::RUNNING, action.status());

    condition.setExpectedResult(NodeStatus::FAILURE);
    state = root.executeTick();

    ASSERT_EQ(NodeStatus::RUNNING, state);
    ASSERT_EQ(NodeStatus::SUCCESS, condition.status());
    ASSERT_EQ(NodeStatus::RUNNING, action.status());
}

TEST_F(ComplexSequenceWithMemoryTest, ConditionsTrue)
{
    BT::NodeStatus state = root.executeTick();

    ASSERT_EQ(NodeStatus::RUNNING, state);
    ASSERT_EQ(NodeStatus::SUCCESS, seq_conditions.status());
    ASSERT_EQ(NodeStatus::IDLE, condition_1.status());
    ASSERT_EQ(NodeStatus::IDLE, condition_2.status());
    ASSERT_EQ(NodeStatus::RUNNING, seq_actions.status());
    ASSERT_EQ(NodeStatus::RUNNING, action_1.status());
    ASSERT_EQ(NodeStatus::IDLE, action_2.status());
}

TEST_F(ComplexSequenceWithMemoryTest, Conditions1ToFalse)
{
    BT::NodeStatus state = root.executeTick();

    condition_1.setExpectedResult(NodeStatus::FAILURE);
    state = root.executeTick();
    // change in condition_1 does not affect the state of the tree,
    // since the seq_conditions was executed already
    ASSERT_EQ(NodeStatus::RUNNING, state);
    ASSERT_EQ(NodeStatus::SUCCESS, seq_conditions.status());
    ASSERT_EQ(NodeStatus::IDLE, condition_1.status());
    ASSERT_EQ(NodeStatus::IDLE, condition_2.status());
    ASSERT_EQ(NodeStatus::RUNNING, seq_actions.status());
    ASSERT_EQ(NodeStatus::RUNNING, action_1.status());
    ASSERT_EQ(NodeStatus::IDLE, action_2.status());
}

TEST_F(ComplexSequenceWithMemoryTest, Conditions2ToFalse)
{
    BT::NodeStatus state = root.executeTick();

    condition_2.setExpectedResult(NodeStatus::FAILURE);
    state = root.executeTick();
    // change in condition_2 does not affect the state of the tree,
    // since the seq_conditions was executed already
    ASSERT_EQ(NodeStatus::RUNNING, state);
    ASSERT_EQ(NodeStatus::SUCCESS, seq_conditions.status());
    ASSERT_EQ(NodeStatus::IDLE, condition_1.status());
    ASSERT_EQ(NodeStatus::IDLE, condition_2.status());
    ASSERT_EQ(NodeStatus::RUNNING, seq_actions.status());
    ASSERT_EQ(NodeStatus::RUNNING, action_1.status());
    ASSERT_EQ(NodeStatus::IDLE, action_2.status());
}

TEST_F(ComplexSequenceWithMemoryTest, Action1DoneSeq)
{
    root.executeTick();

    condition_2.setExpectedResult(NodeStatus::FAILURE);
    root.executeTick();

    // change in condition_2 does not affect the state of the tree,
    // since the seq_conditions was executed already
    ASSERT_EQ(NodeStatus::SUCCESS, seq_conditions.status());
    ASSERT_EQ(NodeStatus::IDLE, condition_1.status());
    ASSERT_EQ(NodeStatus::IDLE, condition_2.status());
    ASSERT_EQ(NodeStatus::RUNNING, seq_actions.status());
    ASSERT_EQ(NodeStatus::RUNNING, action_1.status());
    ASSERT_EQ(NodeStatus::IDLE, action_2.status());

    std::this_thread::sleep_for(milliseconds(150));
    root.executeTick();

    ASSERT_EQ(NodeStatus::SUCCESS, seq_conditions.status());
    ASSERT_EQ(NodeStatus::IDLE, condition_1.status());
    ASSERT_EQ(NodeStatus::IDLE, condition_2.status());
    ASSERT_EQ(NodeStatus::RUNNING, seq_actions.status());
    ASSERT_EQ(NodeStatus::SUCCESS, action_1.status());
    ASSERT_EQ(NodeStatus::RUNNING, action_2.status());

    std::this_thread::sleep_for(milliseconds(150));
    root.executeTick();

    ASSERT_EQ(NodeStatus::SUCCESS, root.status());
    ASSERT_EQ(NodeStatus::IDLE, seq_conditions.status());
    ASSERT_EQ(NodeStatus::IDLE, condition_1.status());
    ASSERT_EQ(NodeStatus::IDLE, condition_2.status());
    ASSERT_EQ(NodeStatus::IDLE, seq_actions.status());
    ASSERT_EQ(NodeStatus::IDLE, action_1.status());
    ASSERT_EQ(NodeStatus::IDLE, action_2.status());
}

TEST_F(ComplexSequenceWithMemoryTest, Action2FailureSeq)
{
    root.executeTick();
    std::this_thread::sleep_for(milliseconds(150));
    root.executeTick();

    ASSERT_EQ(NodeStatus::SUCCESS, seq_conditions.status());
    ASSERT_EQ(NodeStatus::IDLE, condition_1.status());
    ASSERT_EQ(NodeStatus::IDLE, condition_2.status());
    ASSERT_EQ(NodeStatus::RUNNING, seq_actions.status());
    ASSERT_EQ(NodeStatus::SUCCESS, action_1.status());
    ASSERT_EQ(NodeStatus::RUNNING, action_2.status());

    action_2.setExpectedResult(NodeStatus::FAILURE);
    std::this_thread::sleep_for(milliseconds(150));
    root.executeTick();

    // failure in action_2 does not affect the state of already
    // executed nodes (seq_conditions and action_1)
    ASSERT_EQ(NodeStatus::FAILURE, root.status());
    ASSERT_EQ(NodeStatus::SUCCESS, seq_conditions.status());
    ASSERT_EQ(NodeStatus::IDLE, condition_1.status());
    ASSERT_EQ(NodeStatus::IDLE, condition_2.status());
    ASSERT_EQ(NodeStatus::IDLE, seq_actions.status());
    ASSERT_EQ(NodeStatus::SUCCESS, action_1.status());
    ASSERT_EQ(NodeStatus::IDLE, action_2.status());

    action_2.setExpectedResult(NodeStatus::SUCCESS);
    root.executeTick();

    ASSERT_EQ(NodeStatus::SUCCESS, seq_conditions.status());
    ASSERT_EQ(NodeStatus::IDLE, condition_1.status());
    ASSERT_EQ(NodeStatus::IDLE, condition_2.status());
    ASSERT_EQ(NodeStatus::RUNNING, seq_actions.status());
    ASSERT_EQ(NodeStatus::SUCCESS, action_1.status());
    ASSERT_EQ(NodeStatus::RUNNING, action_2.status());

    std::this_thread::sleep_for(milliseconds(150));
    root.executeTick();

    ASSERT_EQ(NodeStatus::SUCCESS, root.status());
    ASSERT_EQ(NodeStatus::IDLE, seq_conditions.status());
    ASSERT_EQ(NodeStatus::IDLE, condition_1.status());
    ASSERT_EQ(NodeStatus::IDLE, condition_2.status());
    ASSERT_EQ(NodeStatus::IDLE, seq_actions.status());
    ASSERT_EQ(NodeStatus::IDLE, action_1.status());
    ASSERT_EQ(NodeStatus::IDLE, action_2.status());
}

TEST_F(ComplexSequenceWithMemoryTest, Action2HaltSeq)
{
    root.executeTick();
    std::this_thread::sleep_for(milliseconds(150));
    root.executeTick();

    root.halt();

    ASSERT_EQ(NodeStatus::IDLE, seq_conditions.status());
    ASSERT_EQ(NodeStatus::IDLE, condition_1.status());
    ASSERT_EQ(NodeStatus::IDLE, condition_2.status());
    ASSERT_EQ(NodeStatus::IDLE, seq_actions.status());
    ASSERT_EQ(NodeStatus::IDLE, action_1.status());
    ASSERT_EQ(NodeStatus::IDLE, action_2.status());

    root.executeTick();

    // tree retakes execution from action_2
    ASSERT_EQ(NodeStatus::IDLE, seq_conditions.status());
    ASSERT_EQ(NodeStatus::IDLE, condition_1.status());
    ASSERT_EQ(NodeStatus::IDLE, condition_2.status());
    ASSERT_EQ(NodeStatus::RUNNING, seq_actions.status());
    ASSERT_EQ(NodeStatus::IDLE, action_1.status());
    ASSERT_EQ(NodeStatus::RUNNING, action_2.status());

    std::this_thread::sleep_for(milliseconds(150));
    root.executeTick();

    ASSERT_EQ(NodeStatus::SUCCESS, root.status());
    ASSERT_EQ(NodeStatus::IDLE, seq_conditions.status());
    ASSERT_EQ(NodeStatus::IDLE, condition_1.status());
    ASSERT_EQ(NodeStatus::IDLE, condition_2.status());
    ASSERT_EQ(NodeStatus::IDLE, seq_actions.status());
    ASSERT_EQ(NodeStatus::IDLE, action_1.status());
    ASSERT_EQ(NodeStatus::IDLE, action_2.status());
}
