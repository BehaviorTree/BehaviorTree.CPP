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

struct SimpleSequenceTest : testing::Test
{
    std::unique_ptr<BT::SequenceNode> root;
    std::unique_ptr<BT::ActionTestNode> action;
    std::unique_ptr<BT::ConditionTestNode> condition;
    SimpleSequenceTest()
    {
        action.reset(new BT::ActionTestNode("action"));
        condition.reset(  new BT::ConditionTestNode("condition") );

        root.reset(  new BT::SequenceNode("seq1") );

        root->addChild(condition.get());
        root->addChild(action.get());
    }
};

struct ComplexSequenceTest : testing::Test
{
    std::unique_ptr<BT::SequenceNode> root;
    std::unique_ptr<BT::ActionTestNode> action_1;
    std::unique_ptr<BT::ConditionTestNode> condition_1;
    std::unique_ptr<BT::ConditionTestNode> condition_2;

    std::unique_ptr<BT::SequenceNode> seq_conditions;

    ComplexSequenceTest()
    {
        action_1.reset(  new BT::ActionTestNode("action 1") );

        condition_1.reset(  new BT::ConditionTestNode("condition 1" ));
        condition_2.reset(  new BT::ConditionTestNode("condition 2" ));
        seq_conditions.reset(  new BT::SequenceNode("sequence_conditions") );

        seq_conditions->addChild(condition_1.get());
        seq_conditions->addChild(condition_2.get());

        root.reset(  new BT::SequenceNode("root") );
        root->addChild(seq_conditions.get());
        root->addChild(action_1.get());
    }
};

struct ComplexSequence2ActionsTest : testing::Test
{
    std::unique_ptr<BT::SequenceNode> root;
    std::unique_ptr<BT::ActionTestNode> action_1;
    std::unique_ptr<BT::ActionTestNode> action_2;

    std::unique_ptr<BT::ConditionTestNode> condition_1;
    std::unique_ptr<BT::ConditionTestNode> condition_2;

    std::unique_ptr<BT::SequenceNode> seq_1;
    std::unique_ptr<BT::SequenceNode> seq_2;

    ComplexSequence2ActionsTest()
    {
        action_1.reset(  new BT::ActionTestNode("action 1") );
        action_2.reset(  new BT::ActionTestNode("action 2") );
        seq_1.reset(  new BT::SequenceNode("sequence_1") );
        seq_2.reset(  new BT::SequenceNode("sequence_c2") );

        condition_1.reset(  new BT::ConditionTestNode("condition 1") );
        condition_2.reset(  new BT::ConditionTestNode("condition 2") );

        seq_1->addChild(condition_1.get());
        seq_1->addChild(action_1.get());

        seq_2->addChild(condition_2.get());
        seq_2->addChild(action_2.get());

        root.reset(  new BT::SequenceNode("root") );
        root->addChild(seq_1.get());
        root->addChild(seq_2.get());
    }
};

struct SimpleFallbackTest : testing::Test
{
    std::unique_ptr<BT::FallbackNode> root;
    std::unique_ptr<BT::ActionTestNode> action;
    std::unique_ptr<BT::ConditionTestNode> condition;
    SimpleFallbackTest()
    {
        action.reset(  new BT::ActionTestNode("action") );
        condition.reset(  new BT::ConditionTestNode("condition") );

        root.reset(  new BT::FallbackNode("seq1") );

        root->addChild(condition.get());
        root->addChild(action.get());
    }
};

struct ComplexFallbackTest : testing::Test
{
    std::unique_ptr<BT::FallbackNode> root;
    std::unique_ptr<BT::ActionTestNode> action_1;
    std::unique_ptr<BT::ConditionTestNode> condition_1;
    std::unique_ptr<BT::ConditionTestNode> condition_2;

    std::unique_ptr<BT::FallbackNode> sel_conditions;

    ComplexFallbackTest()
    {
        action_1.reset(  new BT::ActionTestNode("action 1") );
        condition_1.reset(  new BT::ConditionTestNode("condition 1") );
        condition_2.reset(  new BT::ConditionTestNode("condition 2") );
        sel_conditions.reset(  new BT::FallbackNode("fallback_conditions") );

        sel_conditions->addChild(condition_1.get());
        sel_conditions->addChild(condition_2.get());

        root.reset(  new BT::FallbackNode("root") );
        root->addChild(sel_conditions.get());
        root->addChild(action_1.get());
    }
};

struct BehaviorTreeTest : testing::Test
{
    std::unique_ptr<BT::SequenceNode> root;
    std::unique_ptr<BT::ActionTestNode> action_1;
    std::unique_ptr<BT::ConditionTestNode> condition_1;
    std::unique_ptr<BT::ConditionTestNode> condition_2;

    std::unique_ptr<BT::FallbackNode> sel_conditions;

    BehaviorTreeTest()
    {
        action_1.reset(  new BT::ActionTestNode("action 1") );
        condition_1.reset(  new BT::ConditionTestNode("condition 1") );
        condition_2.reset(  new BT::ConditionTestNode("condition 2") );
        sel_conditions.reset(  new BT::FallbackNode("fallback_conditions") );

        sel_conditions->addChild(condition_1.get());
        sel_conditions->addChild(condition_2.get());

        root.reset(  new BT::SequenceNode("root") );
        root->addChild(sel_conditions.get());
        root->addChild(action_1.get());
    }
};

struct SimpleSequenceWithMemoryTest : testing::Test
{
    std::unique_ptr<BT::SequenceNodeWithMemory> root;
    std::unique_ptr<BT::ActionTestNode> action;
    std::unique_ptr<BT::ConditionTestNode> condition;
    SimpleSequenceWithMemoryTest()
    {
        action.reset(  new BT::ActionTestNode("action") );
        condition.reset(  new BT::ConditionTestNode("condition") );

        root.reset(  new BT::SequenceNodeWithMemory("seq1") );

        root->addChild(condition.get());
        root->addChild(action.get());
    }
};

struct ComplexSequenceWithMemoryTest : testing::Test
{
    std::unique_ptr<BT::SequenceNodeWithMemory> root;

    std::unique_ptr<BT::ActionTestNode> action_1;
    std::unique_ptr<BT::ActionTestNode> action_2;

    std::unique_ptr<BT::ConditionTestNode> condition_1;
    std::unique_ptr<BT::ConditionTestNode> condition_2;

    std::unique_ptr<BT::SequenceNodeWithMemory> seq_conditions;
    std::unique_ptr<BT::SequenceNodeWithMemory> seq_actions;

    ComplexSequenceWithMemoryTest()
    {
        action_1.reset(  new BT::ActionTestNode("action 1") );
        action_2.reset(  new BT::ActionTestNode("action 2") );

        condition_1.reset(  new BT::ConditionTestNode("condition 1") );
        condition_2.reset(  new BT::ConditionTestNode("condition 2") );

        seq_conditions.reset(  new BT::SequenceNodeWithMemory("sequence_conditions") );
        seq_actions.reset(  new BT::SequenceNodeWithMemory("sequence_actions") );

        seq_actions->addChild(action_1.get());
        seq_actions->addChild(action_2.get());

        seq_conditions->addChild(condition_1.get());
        seq_conditions->addChild(condition_2.get());

        root.reset(  new BT::SequenceNodeWithMemory("root") );
        root->addChild(seq_conditions.get());
        root->addChild(seq_actions.get());
    }
};

struct SimpleFallbackWithMemoryTest : testing::Test
{
    std::unique_ptr<BT::FallbackNodeWithMemory> root;
    std::unique_ptr<BT::ActionTestNode> action;
    std::unique_ptr<BT::ConditionTestNode> condition;
    SimpleFallbackWithMemoryTest()
    {
        action.reset(  new BT::ActionTestNode("action") );
        condition.reset(  new BT::ConditionTestNode("condition") );

        root.reset(  new BT::FallbackNodeWithMemory("seq1") );

        root->addChild(condition.get());
        root->addChild(action.get());
    }
};

struct ComplexFallbackWithMemoryTest : testing::Test
{
    std::unique_ptr<BT::FallbackNodeWithMemory> root;

    std::unique_ptr<BT::ActionTestNode> action_1;
    std::unique_ptr<BT::ActionTestNode> action_2;

    std::unique_ptr<BT::ConditionTestNode> condition_1;
    std::unique_ptr<BT::ConditionTestNode> condition_2;

    std::unique_ptr<BT::FallbackNodeWithMemory> fal_conditions;
    std::unique_ptr<BT::FallbackNodeWithMemory> fal_actions;

    ComplexFallbackWithMemoryTest()
    {
        action_1.reset(  new BT::ActionTestNode("action 1") );
        action_2.reset(  new BT::ActionTestNode("action 2") );
        condition_1.reset(  new BT::ConditionTestNode("condition 1") );
        condition_2.reset(  new BT::ConditionTestNode("condition 2") );

        fal_conditions.reset(  new BT::FallbackNodeWithMemory("fallback_conditions") );
        fal_actions.reset(  new BT::FallbackNodeWithMemory("fallback_actions") );

        fal_actions->addChild(action_1.get());
        fal_actions->addChild(action_2.get());

        fal_conditions->addChild(condition_1.get());
        fal_conditions->addChild(condition_2.get());

        root.reset(  new BT::FallbackNodeWithMemory("root") );
        root->addChild(fal_conditions.get());
        root->addChild(fal_actions.get());
    }
};

struct SimpleParallelTest : testing::Test
{
    std::unique_ptr<BT::ParallelNode> root;
    std::unique_ptr<BT::ActionTestNode> action_1;
    std::unique_ptr<BT::ConditionTestNode> condition_1;

    std::unique_ptr<BT::ActionTestNode> action_2;
    std::unique_ptr<BT::ConditionTestNode> condition_2;

    SimpleParallelTest()
    {
        action_1.reset(  new BT::ActionTestNode("action 1") );
        condition_1.reset(  new BT::ConditionTestNode("condition 1") );

        action_2.reset(  new BT::ActionTestNode("action 2") );
        condition_2.reset(  new BT::ConditionTestNode("condition 2") );

        root.reset(  new BT::ParallelNode("par", 4) );

        root->addChild(condition_1.get());
        root->addChild(action_1.get());
        root->addChild(condition_2.get());
        root->addChild(action_2.get());
    }
};

struct ComplexParallelTest : testing::Test
{
    std::unique_ptr<BT::ParallelNode> root;
    std::unique_ptr<BT::ParallelNode> parallel_1;
    std::unique_ptr<BT::ParallelNode> parallel_2;

    std::unique_ptr<BT::ActionTestNode> action_1;
    std::unique_ptr<BT::ConditionTestNode> condition_1;

    std::unique_ptr<BT::ActionTestNode> action_2;
    std::unique_ptr<BT::ConditionTestNode> condition_2;

    std::unique_ptr<BT::ActionTestNode> action_3;
    std::unique_ptr<BT::ConditionTestNode> condition_3;

    ComplexParallelTest()
    {
        action_1.reset(  new BT::ActionTestNode("action 1") );
        condition_1.reset(  new BT::ConditionTestNode("condition 1") );

        action_2.reset(  new BT::ActionTestNode("action 2") );
        condition_2.reset(  new BT::ConditionTestNode("condition 2") );

        action_3.reset(  new BT::ActionTestNode("action 3") );
        condition_3.reset(  new BT::ConditionTestNode("condition 3") );

        root.reset(  new BT::ParallelNode("root", 2) );
        parallel_1.reset(  new BT::ParallelNode("par1", 3) );
        parallel_2.reset(  new BT::ParallelNode("par2", 1) );

        parallel_1->addChild(condition_1.get());
        parallel_1->addChild(action_1.get());
        parallel_1->addChild(condition_2.get());
        parallel_1->addChild(action_2.get());

        parallel_2->addChild(condition_3.get());
        parallel_2->addChild(action_3.get());

        root->addChild(parallel_1.get());
        root->addChild(parallel_2.get());
    }
};

/****************TESTS START HERE***************************/

TEST_F(SimpleSequenceTest, ConditionTrue)
{
    std::cout << "Ticking the root node !" << std::endl << std::endl;
    // Ticking the root node
    BT::NodeStatus state = root->tick();

    ASSERT_EQ(BT::RUNNING, action->status());
    ASSERT_EQ(BT::RUNNING, state);
    root->halt();
}

TEST_F(SimpleSequenceTest, ConditionTurnToFalse)
{
    BT::NodeStatus state = root->tick();
    condition->set_boolean_value(false);

    state = root->tick();
    ASSERT_EQ(BT::FAILURE, state);
    ASSERT_EQ(BT::HALTED, action->status());
    root->halt();
}

TEST_F(ComplexSequenceTest, ComplexSequenceConditionsTrue)
{
    BT::NodeStatus state = root->tick();

    ASSERT_EQ(BT::RUNNING, action_1->status());
    ASSERT_EQ(BT::RUNNING, state);
    root->halt();
}

TEST_F(ComplexSequence2ActionsTest, ConditionsTrue)
{
    BT::NodeStatus state = root->tick();

    state = root->tick();

    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    state = root->tick();
    state = root->tick();

    ASSERT_EQ(BT::RUNNING, state);
    ASSERT_EQ(BT::RUNNING, action_1->status());
    ASSERT_EQ(BT::RUNNING, seq_1->status());
    ASSERT_EQ(BT::HALTED, seq_2->status());
    ASSERT_EQ(BT::HALTED, action_2->status());

    root->halt();
}

TEST_F(ComplexSequenceTest, ComplexSequenceConditions1ToFalse)
{
    BT::NodeStatus state = root->tick();

    condition_1->set_boolean_value(false);

    state = root->tick();

    ASSERT_EQ(BT::FAILURE, state);
    ASSERT_EQ(BT::HALTED, action_1->status());
    root->halt();
}

TEST_F(ComplexSequenceTest, ComplexSequenceConditions2ToFalse)
{
    BT::NodeStatus state = root->tick();

    condition_2->set_boolean_value(false);

    state = root->tick();

    ASSERT_EQ(BT::FAILURE, state);
    ASSERT_EQ(BT::HALTED, action_1->status());
    root->halt();
}

TEST_F(SimpleFallbackTest, ConditionTrue)
{
    std::cout << "Ticking the root node !" << std::endl << std::endl;
    // Ticking the root node
    condition->set_boolean_value(true);
    BT::NodeStatus state = root->tick();

    ASSERT_EQ(BT::IDLE, action->status());
    ASSERT_EQ(BT::SUCCESS, state);
    root->halt();
}

TEST_F(SimpleFallbackTest, ConditionToFalse)
{
    condition->set_boolean_value(false);

    BT::NodeStatus state = root->tick();
    condition->set_boolean_value(true);

    state = root->tick();

    ASSERT_EQ(BT::SUCCESS, state);
    ASSERT_EQ(BT::HALTED, action->status());
    root->halt();
}

TEST_F(ComplexFallbackTest, Condition1ToTrue)
{
    condition_1->set_boolean_value(false);
    condition_2->set_boolean_value(false);

    BT::NodeStatus state = root->tick();

    condition_1->set_boolean_value(true);

    state = root->tick();

    ASSERT_EQ(BT::SUCCESS, state);

    ASSERT_EQ(BT::HALTED, action_1->status());
    root->halt();
}

TEST_F(ComplexFallbackTest, Condition2ToTrue)
{
    condition_1->set_boolean_value(false);
    condition_2->set_boolean_value(false);

    BT::NodeStatus state = root->tick();

    condition_2->set_boolean_value(true);

    state = root->tick();

    ASSERT_EQ(BT::SUCCESS, state);
    ASSERT_EQ(BT::HALTED, action_1->status());
    root->halt();
}

TEST_F(BehaviorTreeTest, Condition1ToFalseCondition2True)
{
    condition_1->set_boolean_value(false);
    condition_2->set_boolean_value(true);

    BT::NodeStatus state = root->tick();

    ASSERT_EQ(BT::RUNNING, state);
    ASSERT_EQ(BT::RUNNING, action_1->status());

    root->halt();
}

TEST_F(BehaviorTreeTest, Condition2ToFalseCondition1True)
{
    condition_2->set_boolean_value(false);
    condition_1->set_boolean_value(true);

    BT::NodeStatus state = root->tick();

    ASSERT_EQ(BT::RUNNING, state);
    ASSERT_EQ(BT::RUNNING, action_1->status());

    root->halt();
}

TEST_F(SimpleSequenceWithMemoryTest, ConditionTrue)
{
    std::cout << "Ticking the root node !" << std::endl << std::endl;
    // Ticking the root node
    BT::NodeStatus state = root->tick();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    ASSERT_EQ(BT::RUNNING, action->status());
    ASSERT_EQ(BT::RUNNING, state);
    root->halt();
}

TEST_F(SimpleSequenceWithMemoryTest, ConditionTurnToFalse)
{
    BT::NodeStatus state = root->tick();

    condition->set_boolean_value(false);

    state = root->tick();

    ASSERT_EQ(BT::RUNNING, state);
    ASSERT_EQ(BT::RUNNING, action->status());

    root->halt();
}

TEST_F(ComplexSequenceWithMemoryTest, ConditionsTrue)
{
    BT::NodeStatus state = root->tick();

    ASSERT_EQ(BT::RUNNING, action_1->status());
    ASSERT_EQ(BT::IDLE, action_2->status());
    ASSERT_EQ(BT::RUNNING, state);

    root->halt();
}

TEST_F(ComplexSequenceWithMemoryTest, Conditions1ToFalse)
{
    BT::NodeStatus state = root->tick();

    condition_1->set_boolean_value(false);

    state = root->tick();

    ASSERT_EQ(BT::RUNNING, action_1->status());
    ASSERT_EQ(BT::IDLE, action_2->status());
    ASSERT_EQ(BT::RUNNING, state);
    root->halt();
}

TEST_F(ComplexSequenceWithMemoryTest, Conditions2ToFalse)
{
    BT::NodeStatus state = root->tick();

    condition_2->set_boolean_value(false);

    state = root->tick();

    ASSERT_EQ(BT::RUNNING, action_1->status());
    ASSERT_EQ(BT::IDLE, action_2->status());
    ASSERT_EQ(BT::RUNNING, state);

    root->halt();
}

TEST_F(ComplexSequenceWithMemoryTest, Action1Done)
{
    root->tick();

    condition_2->set_boolean_value(false);

    root->tick();
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    root->tick();

    ASSERT_EQ(BT::RUNNING, action_2->status());

    root->halt();
}

TEST_F(SimpleFallbackWithMemoryTest, ConditionFalse)
{
    std::cout << "Ticking the root node !" << std::endl << std::endl;
    // Ticking the root node
    condition->set_boolean_value(false);
    BT::NodeStatus state = root->tick();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    ASSERT_EQ(BT::RUNNING, action->status());
    ASSERT_EQ(BT::RUNNING, state);

    root->halt();
}

TEST_F(SimpleFallbackWithMemoryTest, ConditionTurnToTrue)
{
    condition->set_boolean_value(false);

    BT::NodeStatus state = root->tick();
    condition->set_boolean_value(true);

    state = root->tick();

    ASSERT_EQ(BT::RUNNING, state);
    ASSERT_EQ(BT::RUNNING, action->status());

    root->halt();
}

TEST_F(ComplexFallbackWithMemoryTest, ConditionsTrue)
{
    BT::NodeStatus state = root->tick();

    ASSERT_EQ(BT::IDLE, action_1->status());
    ASSERT_EQ(BT::IDLE, action_2->status());
    ASSERT_EQ(BT::SUCCESS, state);

    root->halt();
}

TEST_F(ComplexFallbackWithMemoryTest, Condition1False)
{
    condition_1->set_boolean_value(false);
    BT::NodeStatus state = root->tick();

    ASSERT_EQ(BT::IDLE, action_1->status());
    ASSERT_EQ(BT::IDLE, action_2->status());
    ASSERT_EQ(BT::SUCCESS, state);

    root->halt();
}

TEST_F(ComplexFallbackWithMemoryTest, ConditionsFalse)
{
    condition_1->set_boolean_value(false);
    condition_2->set_boolean_value(false);
    BT::NodeStatus state = root->tick();

    ASSERT_EQ(BT::RUNNING, action_1->status());
    ASSERT_EQ(BT::IDLE, action_2->status());
    ASSERT_EQ(BT::RUNNING, state);

    root->halt();
}

TEST_F(ComplexFallbackWithMemoryTest, Conditions1ToTrue)
{
    condition_1->set_boolean_value(false);
    condition_2->set_boolean_value(false);
    BT::NodeStatus state = root->tick();
    condition_1->set_boolean_value(true);

    state = root->tick();

    ASSERT_EQ(BT::RUNNING, action_1->status());
    ASSERT_EQ(BT::IDLE, action_2->status());
    ASSERT_EQ(BT::RUNNING, state);

    root->halt();
}

TEST_F(ComplexFallbackWithMemoryTest, Conditions2ToTrue)
{
    condition_1->set_boolean_value(false);

    condition_2->set_boolean_value(false);

    BT::NodeStatus state = root->tick();

    condition_2->set_boolean_value(true);

    state = root->tick();

    ASSERT_EQ(BT::RUNNING, action_1->status());
    ASSERT_EQ(BT::IDLE, action_2->status());
    ASSERT_EQ(BT::RUNNING, state);

    root->halt();
}

TEST_F(ComplexFallbackWithMemoryTest, Action1Failed)
{
    action_1->set_boolean_value(false);
    condition_1->set_boolean_value(false);
    condition_2->set_boolean_value(false);

    BT::NodeStatus state = root->tick();

    state = root->tick();
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    state = root->tick();

    ASSERT_EQ(BT::IDLE, action_1->status());
    ASSERT_EQ(BT::RUNNING, action_2->status());
    ASSERT_EQ(BT::RUNNING, state);

    root->halt();
}

TEST_F(SimpleParallelTest, ConditionsTrue)
{
    BT::NodeStatus state = root->tick();

    ASSERT_EQ(BT::IDLE, condition_1->status());
    ASSERT_EQ(BT::IDLE, condition_2->status());
    ASSERT_EQ(BT::RUNNING, action_1->status());
    ASSERT_EQ(BT::RUNNING, action_2->status());
    ASSERT_EQ(BT::RUNNING, state);

    root->halt();
}

TEST_F(SimpleParallelTest, Threshold_3)
{
    root->setThresholdM(3);
    action_2->set_time(200);
    root->tick();
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    BT::NodeStatus state = root->tick();

    ASSERT_EQ(BT::IDLE, condition_1->status());
    ASSERT_EQ(BT::IDLE, condition_2->status());
    ASSERT_EQ(BT::IDLE, action_1->status());
    ASSERT_EQ(BT::HALTED, action_2->status());
    ASSERT_EQ(BT::SUCCESS, state);

    root->halt();
}

TEST_F(SimpleParallelTest, Threshold_1)
{
    root->setThresholdM(1);
    BT::NodeStatus state = root->tick();

    ASSERT_EQ(BT::IDLE, condition_1->status());
    ASSERT_EQ(BT::IDLE, condition_2->status());
    ASSERT_EQ(BT::IDLE, action_1->status());
    ASSERT_EQ(BT::IDLE, action_2->status());
    ASSERT_EQ(BT::SUCCESS, state);

    root->halt();
}
TEST_F(ComplexParallelTest, ConditionsTrue)
{
    BT::NodeStatus state = root->tick();

    ASSERT_EQ(BT::IDLE, condition_1->status());
    ASSERT_EQ(BT::IDLE, condition_2->status());
    ASSERT_EQ(BT::IDLE, condition_3->status());
    ASSERT_EQ(BT::RUNNING, action_1->status());
    ASSERT_EQ(BT::RUNNING, action_2->status());
    ASSERT_EQ(BT::IDLE, action_3->status());
    ASSERT_EQ(BT::RUNNING, parallel_1->status());
    ASSERT_EQ(BT::IDLE, parallel_2->status());
    ASSERT_EQ(BT::RUNNING, state);

    root->halt();
}

TEST_F(ComplexParallelTest, Condition3False)
{
    condition_3->set_boolean_value(false);
    BT::NodeStatus state = root->tick();

    ASSERT_EQ(BT::IDLE, condition_1->status());
    ASSERT_EQ(BT::IDLE, condition_2->status());
    ASSERT_EQ(BT::IDLE, condition_3->status());
    ASSERT_EQ(BT::RUNNING, action_1->status());
    ASSERT_EQ(BT::RUNNING, action_2->status());
    ASSERT_EQ(BT::RUNNING, action_3->status());
    ASSERT_EQ(BT::RUNNING, parallel_1->status());
    ASSERT_EQ(BT::RUNNING, parallel_2->status());
    ASSERT_EQ(BT::RUNNING, state);

    root->halt();
}

TEST_F(ComplexParallelTest, Condition3FalseAction1Done)
{
    action_2->set_time(10);
    action_3->set_time(10);

    condition_3->set_boolean_value(false);
    BT::NodeStatus state = root->tick();
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    ASSERT_EQ(BT::IDLE, condition_1->status());
    ASSERT_EQ(BT::IDLE, condition_2->status());
    ASSERT_EQ(BT::IDLE, condition_3->status());
    ASSERT_EQ(BT::SUCCESS, action_1->status());     // success not read yet by the node parallel_1
    ASSERT_EQ(BT::RUNNING, parallel_1->status());   // parallel_1 hasn't realize (yet) that action_1 has succeeded

    state = root->tick();

    ASSERT_EQ(BT::IDLE, action_1->status());
    ASSERT_EQ(BT::IDLE, parallel_1->status());
    ASSERT_EQ(BT::HALTED, action_2->status());
    ASSERT_EQ(BT::RUNNING, action_3->status());
    ASSERT_EQ(BT::RUNNING, parallel_2->status());
    ASSERT_EQ(BT::RUNNING, state);

    state = root->tick();
    std::this_thread::sleep_for(std::chrono::milliseconds(1500));
    state = root->tick();

    ASSERT_EQ(BT::IDLE, parallel_2->status());
    ASSERT_EQ(BT::IDLE, action_1->status());
    ASSERT_EQ(BT::IDLE, parallel_1->status());
    ASSERT_EQ(BT::IDLE, action_3->status());
    ASSERT_EQ(BT::IDLE, parallel_2->status());
    ASSERT_EQ(BT::SUCCESS, state);

    root->halt();
}

int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
