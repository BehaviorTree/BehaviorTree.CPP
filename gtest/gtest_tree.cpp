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
#include "behavior_tree_core/behavior_tree.h"

using BT::NodeStatus;

struct SimpleSequenceTest : testing::Test
{
    BT::SequenceNode root;
    BT::ActionTestNode action;
    BT::ConditionTestNode condition;

    SimpleSequenceTest():
        root("root_sequence"),
        action("action"),
        condition("condition")
    {
        root.addChild(&condition);
        root.addChild(&action);
    }
};

struct ComplexSequenceTest : testing::Test
{
    BT::SequenceNode root;
    BT::ActionTestNode action_1;
    BT::ConditionTestNode condition_1;
    BT::ConditionTestNode condition_2;

    BT::SequenceNode seq_conditions;

    ComplexSequenceTest():
        root("root_sequence"),
        action_1("action_1"),
        condition_1("condition_1"),
        condition_2("condition_2"),
        seq_conditions("sequence_conditions")
    {
        root.addChild(&seq_conditions);
        {
            seq_conditions.addChild(&condition_1);
            seq_conditions.addChild(&condition_2);
        }
        root.addChild(&action_1);
    }
};

struct ComplexSequence2ActionsTest : testing::Test
{
    BT::SequenceNode root;
    BT::ActionTestNode action_1;
    BT::ActionTestNode action_2;
    BT::SequenceNode seq_1;
    BT::SequenceNode seq_2;

    BT::ConditionTestNode condition_1;
    BT::ConditionTestNode condition_2;

    ComplexSequence2ActionsTest():
        root("root_sequence"),
        action_1("action_1"),
        action_2("action_2"),
        seq_1("sequence_1"),
        seq_2("sequence_2"),
        condition_1("condition_1"),
        condition_2("condition_2")
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
};

struct SimpleFallbackTest : testing::Test
{
    BT::FallbackNode root;
    BT::ActionTestNode action;
    BT::ConditionTestNode condition;

    SimpleFallbackTest():
        root("root_fallback"),
        action("action"),
        condition("condition")
    {
        root.addChild(&condition);
        root.addChild(&action);
    }
};

struct ComplexFallbackTest : testing::Test
{
    BT::FallbackNode root;
    BT::ActionTestNode action_1;
    BT::ConditionTestNode condition_1;
    BT::ConditionTestNode condition_2;

    BT::FallbackNode sel_conditions;

    ComplexFallbackTest():
        root("root_fallback"),
        action_1("action_1"),
        condition_1("condition_1"),
        condition_2("condition_2"),
        sel_conditions("fallback_conditions")
    {
        root.addChild(&sel_conditions);
        {
            sel_conditions.addChild(&condition_1);
            sel_conditions.addChild(&condition_2);
        }
        root.addChild(&action_1);
    }
};

struct BehaviorTreeTest : testing::Test
{
    BT::SequenceNode root;
    BT::ActionTestNode action_1;
    BT::ConditionTestNode condition_1;
    BT::ConditionTestNode condition_2;

    BT::FallbackNode sel_conditions;

    BehaviorTreeTest():
        root("root_sequence"),
        action_1("action_1"),
        condition_1("condition_1"),
        condition_2("condition_2"),
        sel_conditions("fallback_conditions")
    {

        root.addChild(&sel_conditions);
        {
            sel_conditions.addChild(&condition_1);
            sel_conditions.addChild(&condition_2);
        }
        root.addChild(&action_1);
    }
};

struct SimpleSequenceWithMemoryTest : testing::Test
{
    BT::SequenceNodeWithMemory root;
    BT::ActionTestNode action;
    BT::ConditionTestNode condition;

    SimpleSequenceWithMemoryTest():
        root("root_sequence"),
        action("action"),
        condition("condition")
    {
        root.addChild(&condition);
        root.addChild(&action);
    }
};

struct ComplexSequenceWithMemoryTest : testing::Test
{
    BT::SequenceNodeWithMemory root;

    BT::ActionTestNode action_1;
    BT::ActionTestNode action_2;

    BT::ConditionTestNode condition_1;
    BT::ConditionTestNode condition_2;

    BT::SequenceNodeWithMemory seq_conditions;
    BT::SequenceNodeWithMemory seq_actions;

    ComplexSequenceWithMemoryTest():
        root("root_sequence"),
        action_1("action_1"),
        action_2("action_2"),
        condition_1("condition_1"),
        condition_2("condition_2"),
        seq_conditions("sequence_conditions"),
        seq_actions("sequence_actions")
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
};

struct SimpleFallbackWithMemoryTest : testing::Test
{
    BT::FallbackNodeWithMemory root;
    BT::ActionTestNode action;
    BT::ConditionTestNode condition;

    SimpleFallbackWithMemoryTest():
        root("root_sequence"),
        action("action"),
        condition("condition")
    {
        root.addChild(&condition);
        root.addChild(&action);
    }
};

struct ComplexFallbackWithMemoryTest : testing::Test
{
    BT::FallbackNodeWithMemory root;

    BT::ActionTestNode action_1;
    BT::ActionTestNode action_2;

    BT::ConditionTestNode condition_1;
    BT::ConditionTestNode condition_2;

    BT::FallbackNodeWithMemory fal_conditions;
    BT::FallbackNodeWithMemory fal_actions;

    ComplexFallbackWithMemoryTest():
        root("root_fallback"),
        action_1("action_1"),
        action_2("action_2"),
        condition_1("condition_1"),
        condition_2("condition_2"),
        fal_conditions("fallback_conditions"),
        fal_actions("fallback_actions")
    {
        root.addChild(&fal_conditions);
        {
            fal_conditions.addChild(&condition_1);
            fal_conditions.addChild(&condition_2);
        }
        root.addChild(&fal_actions);
        {
            fal_actions.addChild(&action_1);
            fal_actions.addChild(&action_2);
        }
    }
};

struct SimpleParallelTest : testing::Test
{
    BT::ParallelNode root;
    BT::ActionTestNode action_1;
    BT::ConditionTestNode condition_1;

    BT::ActionTestNode action_2;
    BT::ConditionTestNode condition_2;

    SimpleParallelTest():
        root("root_parallel",4),
        action_1("action_1"),
        condition_1("condition_1"),
        action_2("action_2"),
        condition_2("condition_2")
    {
        root.addChild(&condition_1);
        root.addChild(&action_1);
        root.addChild(&condition_2);
        root.addChild(&action_2);
    }
};

struct ComplexParallelTest : testing::Test
{
    BT::ParallelNode root;
    BT::ParallelNode parallel_1;
    BT::ParallelNode parallel_2;

    BT::ActionTestNode action_1;
    BT::ConditionTestNode condition_1;

    BT::ActionTestNode action_2;
    BT::ConditionTestNode condition_2;

    BT::ActionTestNode action_3;
    BT::ConditionTestNode condition_3;

    ComplexParallelTest():
        root("root",2),
        parallel_1("par1",3),
        parallel_2("par2",1),
        action_1("action_1"),
        condition_1("condition_1"),
        action_2("action_2"),
        condition_2("condition_2"),
        action_3("action_3"),
        condition_3("condition_3")
    {
        root.addChild(&parallel_1);
        {
            parallel_1.addChild(&condition_1);
            parallel_1.addChild(&action_1);
            parallel_1.addChild(&condition_2);
            parallel_1.addChild(&action_2);
        }
        root.addChild(&parallel_2);
        {
            parallel_2.addChild(&condition_3);
            parallel_2.addChild(&action_3);
        }
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
    root.halt();
}

TEST_F(SimpleSequenceTest, ConditionTurnToFalse)
{
    BT::NodeStatus state = root.executeTick();
    condition.set_boolean_value(false);

    state = root.executeTick();
    ASSERT_EQ(NodeStatus::FAILURE, state);
    ASSERT_EQ(NodeStatus::HALTED, action.status());
    root.halt();
}

TEST_F(ComplexSequenceTest, ComplexSequenceConditionsTrue)
{
    BT::NodeStatus state = root.executeTick();

    ASSERT_EQ(NodeStatus::RUNNING, action_1.status());
    ASSERT_EQ(NodeStatus::RUNNING, state);
    root.halt();
}

TEST_F(ComplexSequence2ActionsTest, ConditionsTrue)
{
    BT::NodeStatus state = root.executeTick();

    state = root.executeTick();

    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    state = root.executeTick();
    state = root.executeTick();

    ASSERT_EQ(NodeStatus::RUNNING, state);
    ASSERT_EQ(NodeStatus::RUNNING, action_1.status());
    ASSERT_EQ(NodeStatus::RUNNING, seq_1.status());
    ASSERT_EQ(NodeStatus::HALTED, seq_2.status());
    ASSERT_EQ(NodeStatus::HALTED, action_2.status());

    root.halt();
}

TEST_F(ComplexSequenceTest, ComplexSequenceConditions1ToFalse)
{
    BT::NodeStatus state = root.executeTick();

    condition_1.set_boolean_value(false);

    state = root.executeTick();

    ASSERT_EQ(NodeStatus::FAILURE, state);
    ASSERT_EQ(NodeStatus::HALTED, action_1.status());
    root.halt();
}

TEST_F(ComplexSequenceTest, ComplexSequenceConditions2ToFalse)
{
    BT::NodeStatus state = root.executeTick();

    condition_2.set_boolean_value(false);

    state = root.executeTick();

    ASSERT_EQ(NodeStatus::FAILURE, state);
    ASSERT_EQ(NodeStatus::HALTED, action_1.status());
    root.halt();
}

TEST_F(SimpleFallbackTest, ConditionTrue)
{
    std::cout << "Ticking the root node !" << std::endl << std::endl;
    // Ticking the root node
    condition.set_boolean_value(true);
    BT::NodeStatus state = root.executeTick();

    ASSERT_EQ(NodeStatus::IDLE, action.status());
    ASSERT_EQ(NodeStatus::SUCCESS, state);
    root.halt();
}

TEST_F(SimpleFallbackTest, ConditionToFalse)
{
    condition.set_boolean_value(false);

    BT::NodeStatus state = root.executeTick();
    condition.set_boolean_value(true);

    state = root.executeTick();

    ASSERT_EQ(NodeStatus::SUCCESS, state);
    ASSERT_EQ(NodeStatus::HALTED, action.status());
    root.halt();
}

TEST_F(ComplexFallbackTest, Condition1ToTrue)
{
    condition_1.set_boolean_value(false);
    condition_2.set_boolean_value(false);

    BT::NodeStatus state = root.executeTick();

    condition_1.set_boolean_value(true);

    state = root.executeTick();

    ASSERT_EQ(NodeStatus::SUCCESS, state);

    ASSERT_EQ(NodeStatus::HALTED, action_1.status());
    root.halt();
}

TEST_F(ComplexFallbackTest, Condition2ToTrue)
{
    condition_1.set_boolean_value(false);
    condition_2.set_boolean_value(false);

    BT::NodeStatus state = root.executeTick();

    condition_2.set_boolean_value(true);

    state = root.executeTick();

    ASSERT_EQ(NodeStatus::SUCCESS, state);
    ASSERT_EQ(NodeStatus::HALTED, action_1.status());
    root.halt();
}

TEST_F(BehaviorTreeTest, Condition1ToFalseCondition2True)
{
    condition_1.set_boolean_value(false);
    condition_2.set_boolean_value(true);

    BT::NodeStatus state = root.executeTick();

    ASSERT_EQ(NodeStatus::RUNNING, state);
    ASSERT_EQ(NodeStatus::RUNNING, action_1.status());

    root.halt();
}

TEST_F(BehaviorTreeTest, Condition2ToFalseCondition1True)
{
    condition_2.set_boolean_value(false);
    condition_1.set_boolean_value(true);

    BT::NodeStatus state = root.executeTick();

    ASSERT_EQ(NodeStatus::RUNNING, state);
    ASSERT_EQ(NodeStatus::RUNNING, action_1.status());

    root.halt();
}

TEST_F(SimpleSequenceWithMemoryTest, ConditionTrue)
{
    std::cout << "Ticking the root node !" << std::endl << std::endl;
    // Ticking the root node
    BT::NodeStatus state = root.executeTick();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    ASSERT_EQ(NodeStatus::RUNNING, action.status());
    ASSERT_EQ(NodeStatus::RUNNING, state);
    root.halt();
}

TEST_F(SimpleSequenceWithMemoryTest, ConditionTurnToFalse)
{
    BT::NodeStatus state = root.executeTick();

    condition.set_boolean_value(false);

    state = root.executeTick();

    ASSERT_EQ(NodeStatus::RUNNING, state);
    ASSERT_EQ(NodeStatus::RUNNING, action.status());

    root.halt();
}

TEST_F(ComplexSequenceWithMemoryTest, ConditionsTrue)
{
    BT::NodeStatus state = root.executeTick();

    ASSERT_EQ(NodeStatus::RUNNING, action_1.status());
    ASSERT_EQ(NodeStatus::IDLE, action_2.status());
    ASSERT_EQ(NodeStatus::RUNNING, state);

    root.halt();
}

TEST_F(ComplexSequenceWithMemoryTest, Conditions1ToFalse)
{
    BT::NodeStatus state = root.executeTick();

    condition_1.set_boolean_value(false);

    state = root.executeTick();

    ASSERT_EQ(NodeStatus::RUNNING, action_1.status());
    ASSERT_EQ(NodeStatus::IDLE, action_2.status());
    ASSERT_EQ(NodeStatus::RUNNING, state);
    root.halt();
}

TEST_F(ComplexSequenceWithMemoryTest, Conditions2ToFalse)
{
    BT::NodeStatus state = root.executeTick();

    condition_2.set_boolean_value(false);

    state = root.executeTick();

    ASSERT_EQ(NodeStatus::RUNNING, action_1.status());
    ASSERT_EQ(NodeStatus::IDLE, action_2.status());
    ASSERT_EQ(NodeStatus::RUNNING, state);

    root.halt();
}

TEST_F(ComplexSequenceWithMemoryTest, Action1Done)
{
    root.executeTick();

    condition_2.set_boolean_value(false);

    root.executeTick();
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    root.executeTick();

    ASSERT_EQ(NodeStatus::RUNNING, action_2.status());

    root.halt();
}

TEST_F(SimpleFallbackWithMemoryTest, ConditionFalse)
{
    std::cout << "Ticking the root node !" << std::endl << std::endl;
    // Ticking the root node
    condition.set_boolean_value(false);
    BT::NodeStatus state = root.executeTick();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    ASSERT_EQ(NodeStatus::RUNNING, action.status());
    ASSERT_EQ(NodeStatus::RUNNING, state);

    root.halt();
}

TEST_F(SimpleFallbackWithMemoryTest, ConditionTurnToTrue)
{
    condition.set_boolean_value(false);

    BT::NodeStatus state = root.executeTick();
    condition.set_boolean_value(true);

    state = root.executeTick();

    ASSERT_EQ(NodeStatus::RUNNING, state);
    ASSERT_EQ(NodeStatus::RUNNING, action.status());

    root.halt();
}

TEST_F(ComplexFallbackWithMemoryTest, ConditionsTrue)
{
    BT::NodeStatus state = root.executeTick();

    ASSERT_EQ(NodeStatus::IDLE, action_1.status());
    ASSERT_EQ(NodeStatus::IDLE, action_2.status());
    ASSERT_EQ(NodeStatus::SUCCESS, state);

    root.halt();
}

TEST_F(ComplexFallbackWithMemoryTest, Condition1False)
{
    condition_1.set_boolean_value(false);
    BT::NodeStatus state = root.executeTick();

    ASSERT_EQ(NodeStatus::IDLE, action_1.status());
    ASSERT_EQ(NodeStatus::IDLE, action_2.status());
    ASSERT_EQ(NodeStatus::SUCCESS, state);

    root.halt();
}

TEST_F(ComplexFallbackWithMemoryTest, ConditionsFalse)
{
    condition_1.set_boolean_value(false);
    condition_2.set_boolean_value(false);
    BT::NodeStatus state = root.executeTick();

    ASSERT_EQ(NodeStatus::RUNNING, action_1.status());
    ASSERT_EQ(NodeStatus::IDLE, action_2.status());
    ASSERT_EQ(NodeStatus::RUNNING, state);

    root.halt();
}

TEST_F(ComplexFallbackWithMemoryTest, Conditions1ToTrue)
{
    condition_1.set_boolean_value(false);
    condition_2.set_boolean_value(false);
    BT::NodeStatus state = root.executeTick();
    condition_1.set_boolean_value(true);

    state = root.executeTick();

    ASSERT_EQ(NodeStatus::RUNNING, action_1.status());
    ASSERT_EQ(NodeStatus::IDLE, action_2.status());
    ASSERT_EQ(NodeStatus::RUNNING, state);

    root.halt();
}

TEST_F(ComplexFallbackWithMemoryTest, Conditions2ToTrue)
{
    condition_1.set_boolean_value(false);

    condition_2.set_boolean_value(false);

    BT::NodeStatus state = root.executeTick();

    condition_2.set_boolean_value(true);

    state = root.executeTick();

    ASSERT_EQ(NodeStatus::RUNNING, action_1.status());
    ASSERT_EQ(NodeStatus::IDLE, action_2.status());
    ASSERT_EQ(NodeStatus::RUNNING, state);

    root.halt();
}

TEST_F(ComplexFallbackWithMemoryTest, Action1Failed)
{
    action_1.set_boolean_value(false);
    condition_1.set_boolean_value(false);
    condition_2.set_boolean_value(false);

    BT::NodeStatus state = root.executeTick();

    state = root.executeTick();
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    state = root.executeTick();

    ASSERT_EQ(NodeStatus::IDLE, action_1.status());
    ASSERT_EQ(NodeStatus::RUNNING, action_2.status());
    ASSERT_EQ(NodeStatus::RUNNING, state);

    root.halt();
}

TEST_F(SimpleParallelTest, ConditionsTrue)
{
    BT::NodeStatus state = root.executeTick();

    ASSERT_EQ(NodeStatus::IDLE, condition_1.status());
    ASSERT_EQ(NodeStatus::IDLE, condition_2.status());
    ASSERT_EQ(NodeStatus::RUNNING, action_1.status());
    ASSERT_EQ(NodeStatus::RUNNING, action_2.status());
    ASSERT_EQ(NodeStatus::RUNNING, state);

    root.halt();
}

TEST_F(SimpleParallelTest, Threshold_3)
{
    root.setThresholdM(3);
    action_2.set_time(200);
    root.executeTick();
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    BT::NodeStatus state = root.executeTick();

    ASSERT_EQ(NodeStatus::IDLE, condition_1.status());
    ASSERT_EQ(NodeStatus::IDLE, condition_2.status());
    ASSERT_EQ(NodeStatus::IDLE, action_1.status());
    ASSERT_EQ(NodeStatus::HALTED, action_2.status());
    ASSERT_EQ(NodeStatus::SUCCESS, state);

    root.halt();
}

TEST_F(SimpleParallelTest, Threshold_1)
{
    root.setThresholdM(1);
    BT::NodeStatus state = root.executeTick();

    ASSERT_EQ(NodeStatus::IDLE, condition_1.status());
    ASSERT_EQ(NodeStatus::IDLE, condition_2.status());
    ASSERT_EQ(NodeStatus::IDLE, action_1.status());
    ASSERT_EQ(NodeStatus::IDLE, action_2.status());
    ASSERT_EQ(NodeStatus::SUCCESS, state);

    root.halt();
}
TEST_F(ComplexParallelTest, ConditionsTrue)
{
    BT::NodeStatus state = root.executeTick();

    ASSERT_EQ(NodeStatus::IDLE, condition_1.status());
    ASSERT_EQ(NodeStatus::IDLE, condition_2.status());
    ASSERT_EQ(NodeStatus::IDLE, condition_3.status());
    ASSERT_EQ(NodeStatus::RUNNING, action_1.status());
    ASSERT_EQ(NodeStatus::RUNNING, action_2.status());
    ASSERT_EQ(NodeStatus::IDLE, action_3.status());
    ASSERT_EQ(NodeStatus::RUNNING, parallel_1.status());
    ASSERT_EQ(NodeStatus::IDLE, parallel_2.status());
    ASSERT_EQ(NodeStatus::RUNNING, state);

    root.halt();
}

TEST_F(ComplexParallelTest, Condition3False)
{
    condition_3.set_boolean_value(false);
    BT::NodeStatus state = root.executeTick();

    ASSERT_EQ(NodeStatus::IDLE, condition_1.status());
    ASSERT_EQ(NodeStatus::IDLE, condition_2.status());
    ASSERT_EQ(NodeStatus::IDLE, condition_3.status());
    ASSERT_EQ(NodeStatus::RUNNING, action_1.status());
    ASSERT_EQ(NodeStatus::RUNNING, action_2.status());
    ASSERT_EQ(NodeStatus::RUNNING, action_3.status());
    ASSERT_EQ(NodeStatus::RUNNING, parallel_1.status());
    ASSERT_EQ(NodeStatus::RUNNING, parallel_2.status());
    ASSERT_EQ(NodeStatus::RUNNING, state);

    root.halt();
}

TEST_F(ComplexParallelTest, Condition3FalseAction1Done)
{
    action_2.set_time(10);
    action_3.set_time(10);

    condition_3.set_boolean_value(false);
    BT::NodeStatus state = root.executeTick();
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    ASSERT_EQ(NodeStatus::IDLE, condition_1.status());
    ASSERT_EQ(NodeStatus::IDLE, condition_2.status());
    ASSERT_EQ(NodeStatus::IDLE, condition_3.status());
    ASSERT_EQ(NodeStatus::SUCCESS, action_1.status());     // success not read yet by the node parallel_1
    ASSERT_EQ(NodeStatus::RUNNING, parallel_1.status());   // parallel_1 hasn't realize (yet) that action_1 has succeeded

    state = root.executeTick();

    ASSERT_EQ(NodeStatus::IDLE, action_1.status());
    ASSERT_EQ(NodeStatus::IDLE, parallel_1.status());
    ASSERT_EQ(NodeStatus::HALTED, action_2.status());
    ASSERT_EQ(NodeStatus::RUNNING, action_3.status());
    ASSERT_EQ(NodeStatus::RUNNING, parallel_2.status());
    ASSERT_EQ(NodeStatus::RUNNING, state);

    state = root.executeTick();
    std::this_thread::sleep_for(std::chrono::milliseconds(1500));
    state = root.executeTick();

    ASSERT_EQ(NodeStatus::IDLE, parallel_2.status());
    ASSERT_EQ(NodeStatus::IDLE, action_1.status());
    ASSERT_EQ(NodeStatus::IDLE, parallel_1.status());
    ASSERT_EQ(NodeStatus::IDLE, action_3.status());
    ASSERT_EQ(NodeStatus::IDLE, parallel_2.status());
    ASSERT_EQ(NodeStatus::SUCCESS, state);

    root.halt();
}

int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
