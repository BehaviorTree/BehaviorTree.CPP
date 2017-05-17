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
#include <action_test_node.h>
#include <condition_test_node.h>
#include <behavior_tree.h>



struct SimpleSequenceTest : testing::Test
{
    BT:: SequenceNode* root;
    BT::ActionTestNode* action;
    BT::ConditionTestNode* condition;
    SimpleSequenceTest()
    {
        action = new BT::ActionTestNode("action");
        condition = new BT::ConditionTestNode("condition");

        root = new BT::SequenceNode("seq1");

        root->AddChild(condition);
        root->AddChild(action);
    }
};


struct ComplexSequenceTest : testing::Test
{
    BT:: SequenceNode* root;
    BT::ActionTestNode* action_1;
    BT::ConditionTestNode* condition_1;
    BT::ConditionTestNode* condition_2;

    BT:: SequenceNode* seq_conditions;

    ComplexSequenceTest()
    {
        action_1 = new BT::ActionTestNode("action 1");

        condition_1 = new BT::ConditionTestNode("condition 1");
        condition_2 = new BT::ConditionTestNode("condition 2");
        seq_conditions = new BT::SequenceNode("sequence_conditions");

        seq_conditions->AddChild(condition_1);
        seq_conditions->AddChild(condition_2);

        root = new BT::SequenceNode("root");
        root->AddChild(seq_conditions);
        root->AddChild(action_1);
    }
};


struct ComplexSequence2ActionsTest : testing::Test
{
    BT:: SequenceNode* root;
    BT::ActionTestNode* action_1;
    BT::ActionTestNode* action_2;

    BT::ConditionTestNode* condition_1;
    BT::ConditionTestNode* condition_2;

    BT:: SequenceNode* seq_1;
    BT:: SequenceNode* seq_2;

    ComplexSequence2ActionsTest()
    {
        action_1 = new BT::ActionTestNode("action 1");
        action_2 = new BT::ActionTestNode("action 2");
        seq_1 = new BT::SequenceNode("sequence_1");
        seq_2 = new BT::SequenceNode("sequence_c2");

        condition_1 = new BT::ConditionTestNode("condition 1");
        condition_2 = new BT::ConditionTestNode("condition 2");

        seq_1->AddChild(condition_1);
        seq_1->AddChild(action_1);

        seq_2->AddChild(condition_2);
        seq_2->AddChild(action_2);


        root = new BT::SequenceNode("root");
        root->AddChild(seq_1);
        root->AddChild(seq_2);
    }
};


struct SimpleFallbackTest : testing::Test
{
    BT:: FallbackNode* root;
    BT::ActionTestNode* action;
    BT::ConditionTestNode* condition;
    SimpleFallbackTest()
    {
        action = new BT::ActionTestNode("action");
        condition = new BT::ConditionTestNode("condition");

        root = new BT::FallbackNode("seq1");

        root->AddChild(condition);
        root->AddChild(action);
    }
};


struct ComplexFallbackTest : testing::Test
{
    BT:: FallbackNode* root;
    BT::ActionTestNode* action_1;
    BT::ConditionTestNode* condition_1;
    BT::ConditionTestNode* condition_2;

    BT:: FallbackNode* sel_conditions;

    ComplexFallbackTest()
    {
        action_1 = new BT::ActionTestNode("action 1");
        condition_1 = new BT::ConditionTestNode("condition 1");
        condition_2 = new BT::ConditionTestNode("condition 2");
        sel_conditions = new BT::FallbackNode("fallback_conditions");

        sel_conditions->AddChild(condition_1);
        sel_conditions->AddChild(condition_2);

        root = new BT::FallbackNode("root");
        root->AddChild(sel_conditions);
        root->AddChild(action_1);
    }
};




struct BehaviorTreeTest : testing::Test
{
    BT:: SequenceNode* root;
    BT::ActionTestNode* action_1;
    BT::ConditionTestNode* condition_1;
    BT::ConditionTestNode* condition_2;

    BT:: FallbackNode* sel_conditions;

    BehaviorTreeTest()
    {
        action_1 = new BT::ActionTestNode("action 1");
        condition_1 = new BT::ConditionTestNode("condition 1");
        condition_2 = new BT::ConditionTestNode("condition 2");
        sel_conditions = new BT::FallbackNode("fallback_conditions");

        sel_conditions->AddChild(condition_1);
        sel_conditions->AddChild(condition_2);

        root = new BT::SequenceNode("root");
        root->AddChild(sel_conditions);
        root->AddChild(action_1);
    }
};




struct SimpleSequenceWithMemoryTest : testing::Test
{
    BT:: SequenceNodeWithMemory* root;
    BT::ActionTestNode* action;
    BT::ConditionTestNode* condition;
    SimpleSequenceWithMemoryTest()
    {
        action = new BT::ActionTestNode("action");
        condition = new BT::ConditionTestNode("condition");

        root = new BT::SequenceNodeWithMemory("seq1");

        root->AddChild(condition);
        root->AddChild(action);
    }
};

struct ComplexSequenceWithMemoryTest : testing::Test
{
    BT:: SequenceNodeWithMemory* root;

    BT::ActionTestNode* action_1;
    BT::ActionTestNode* action_2;

    BT::ConditionTestNode* condition_1;
    BT::ConditionTestNode* condition_2;

    BT:: SequenceNodeWithMemory* seq_conditions;
    BT:: SequenceNodeWithMemory* seq_actions;

    ComplexSequenceWithMemoryTest()
    {
        action_1 = new BT::ActionTestNode("action 1");
        action_2 = new BT::ActionTestNode("action 2");


        condition_1 = new BT::ConditionTestNode("condition 1");
        condition_2 = new BT::ConditionTestNode("condition 2");

        seq_conditions = new BT::SequenceNodeWithMemory("sequence_conditions");
        seq_actions = new BT::SequenceNodeWithMemory("sequence_actions");

        seq_actions->AddChild(action_1);
        seq_actions->AddChild(action_2);

        seq_conditions->AddChild(condition_1);
        seq_conditions->AddChild(condition_2);

        root = new BT::SequenceNodeWithMemory("root");
        root->AddChild(seq_conditions);
        root->AddChild(seq_actions);
    }
};

struct SimpleFallbackWithMemoryTest : testing::Test
{
    BT::FallbackNodeWithMemory* root;
    BT::ActionTestNode* action;
    BT::ConditionTestNode* condition;
    SimpleFallbackWithMemoryTest()
    {
        action = new BT::ActionTestNode("action");
        condition = new BT::ConditionTestNode("condition");

        root = new BT::FallbackNodeWithMemory("seq1");

        root->AddChild(condition);
        root->AddChild(action);
    }
};

struct ComplexFallbackWithMemoryTest : testing::Test
{
    BT:: FallbackNodeWithMemory* root;

    BT::ActionTestNode* action_1;
    BT::ActionTestNode* action_2;

    BT::ConditionTestNode* condition_1;
    BT::ConditionTestNode* condition_2;

    BT:: FallbackNodeWithMemory* fal_conditions;
    BT:: FallbackNodeWithMemory* fal_actions;

    ComplexFallbackWithMemoryTest()
    {
        action_1 = new BT::ActionTestNode("action 1");
        action_2 = new BT::ActionTestNode("action 2");
        condition_1 = new BT::ConditionTestNode("condition 1");
        condition_2 = new BT::ConditionTestNode("condition 2");

        fal_conditions = new BT::FallbackNodeWithMemory("fallback_conditions");
        fal_actions = new BT::FallbackNodeWithMemory("fallback_actions");

        fal_actions->AddChild(action_1);
        fal_actions->AddChild(action_2);

        fal_conditions->AddChild(condition_1);
        fal_conditions->AddChild(condition_2);

        root = new BT::FallbackNodeWithMemory("root");
        root->AddChild(fal_conditions);
        root->AddChild(fal_actions);
    }
};


struct SimpleParallelTest : testing::Test
{
    BT::ParallelNode* root;
    BT::ActionTestNode* action_1;
    BT::ConditionTestNode* condition_1;

    BT::ActionTestNode* action_2;
    BT::ConditionTestNode* condition_2;

    SimpleParallelTest()
    {
        action_1 = new BT::ActionTestNode("action 1");
        condition_1 = new BT::ConditionTestNode("condition 1");


        action_2 = new BT::ActionTestNode("action 2");
        condition_2 = new BT::ConditionTestNode("condition 2");

        root = new BT::ParallelNode("par", 4);

        root->AddChild(condition_1);
        root->AddChild(action_1);
        root->AddChild(condition_2);
        root->AddChild(action_2);
    }
};


struct ComplexParallelTest : testing::Test
{
    BT::ParallelNode* root;
    BT::ParallelNode* parallel_1;
    BT::ParallelNode* parallel_2;

    BT::ActionTestNode* action_1;
    BT::ConditionTestNode* condition_1;

    BT::ActionTestNode* action_2;
    BT::ConditionTestNode* condition_2;

    BT::ActionTestNode* action_3;
    BT::ConditionTestNode* condition_3;

    ComplexParallelTest()
    {
        action_1 = new BT::ActionTestNode("action 1");
        condition_1 = new BT::ConditionTestNode("condition 1");


        action_2 = new BT::ActionTestNode("action 2");
        condition_2 = new BT::ConditionTestNode("condition 2");


        action_3 = new BT::ActionTestNode("action 3");
        condition_3 = new BT::ConditionTestNode("condition 3");

        root = new BT::ParallelNode("root", 2);
        parallel_1 = new BT::ParallelNode("par1", 3);
        parallel_2 = new BT::ParallelNode("par2", 1);

        parallel_1->AddChild(condition_1);
        parallel_1->AddChild(action_1);
        parallel_1->AddChild(condition_2);
        parallel_1->AddChild(action_2);

        parallel_2->AddChild(condition_3);
        parallel_2->AddChild(action_3);

        root->AddChild(parallel_1);
        root->AddChild(parallel_2);
    }
};



/****************TESTS START HERE***************************/



TEST_F(SimpleSequenceTest, ConditionTrue)
{
    std::cout << "Ticking the root node !" << std::endl << std::endl;
    // Ticking the root node
    BT::ReturnStatus state = root->Tick();

    ASSERT_EQ(BT::RUNNING, action->get_status());
    ASSERT_EQ(BT::RUNNING, state);
    root->Halt();
}


TEST_F(SimpleSequenceTest, ConditionTurnToFalse)
{
    BT::ReturnStatus state = root->Tick();
    condition->set_boolean_value(false);

    state = root->Tick();
    ASSERT_EQ(BT::FAILURE, state);
    ASSERT_EQ(BT::HALTED, action->get_status());
    root->Halt();
}


TEST_F(ComplexSequenceTest, ComplexSequenceConditionsTrue)
{
    BT::ReturnStatus state = root->Tick();

    ASSERT_EQ(BT::RUNNING, action_1->get_status());
    ASSERT_EQ(BT::RUNNING, state);
    root->Halt();
}


TEST_F(ComplexSequence2ActionsTest, ConditionsTrue)
{
    BT::ReturnStatus state = root->Tick();

    state = root->Tick();

    std::this_thread::sleep_for(std::chrono::seconds(5));
    state = root->Tick();
    state = root->Tick();


    ASSERT_EQ(BT::RUNNING, state);
    ASSERT_EQ(BT::RUNNING, action_1->get_status());
    ASSERT_EQ(BT::RUNNING, seq_1->get_status());
    ASSERT_EQ(BT::HALTED, seq_2->get_status());
    ASSERT_EQ(BT::HALTED, action_2->get_status());

    root->Halt();
}

TEST_F(ComplexSequenceTest, ComplexSequenceConditions1ToFalse)
{
    BT::ReturnStatus state = root->Tick();

    condition_1->set_boolean_value(false);

    state = root->Tick();

    ASSERT_EQ(BT::FAILURE, state);
    ASSERT_EQ(BT::HALTED, action_1->get_status());
    root->Halt();
}

TEST_F(ComplexSequenceTest, ComplexSequenceConditions2ToFalse)
{
    BT::ReturnStatus state = root->Tick();

    condition_2->set_boolean_value(false);

    state = root->Tick();

    ASSERT_EQ(BT::FAILURE, state);
    ASSERT_EQ(BT::HALTED, action_1->get_status());
    root->Halt();
}



TEST_F(SimpleFallbackTest, ConditionTrue)
{
    std::cout << "Ticking the root node !" << std::endl << std::endl;
    // Ticking the root node
    condition->set_boolean_value(true);
    BT::ReturnStatus state = root->Tick();

    ASSERT_EQ(BT::IDLE, action->get_status());
    ASSERT_EQ(BT::SUCCESS, state);
    root->Halt();
}


TEST_F(SimpleFallbackTest, ConditionToFalse)
{
    condition->set_boolean_value(false);

    BT::ReturnStatus state = root->Tick();
    condition->set_boolean_value(true);


    state = root->Tick();

    ASSERT_EQ(BT::SUCCESS, state);
    ASSERT_EQ(BT::HALTED, action->get_status());
    root->Halt();
}


TEST_F(ComplexFallbackTest, Condition1ToTrue)
{
    condition_1->set_boolean_value(false);
    condition_2->set_boolean_value(false);

    BT::ReturnStatus state = root->Tick();

    condition_1->set_boolean_value(true);

    state = root->Tick();

    ASSERT_EQ(BT::SUCCESS, state);

    ASSERT_EQ(BT::HALTED, action_1->get_status());
    root->Halt();
}

TEST_F(ComplexFallbackTest, Condition2ToTrue)
{
    condition_1->set_boolean_value(false);
    condition_2->set_boolean_value(false);

    BT::ReturnStatus state = root->Tick();

    condition_2->set_boolean_value(true);

    state = root->Tick();

    ASSERT_EQ(BT::SUCCESS, state);
    ASSERT_EQ(BT::HALTED, action_1->get_status());
    root->Halt();
}



TEST_F(BehaviorTreeTest, Condition1ToFalseCondition2True)
{
    condition_1->set_boolean_value(false);
    condition_2->set_boolean_value(true);

    BT::ReturnStatus state = root->Tick();

    ASSERT_EQ(BT::RUNNING, state);
    ASSERT_EQ(BT::RUNNING, action_1->get_status());

    root->Halt();
}

TEST_F(BehaviorTreeTest, Condition2ToFalseCondition1True)
{
    condition_2->set_boolean_value(false);
    condition_1->set_boolean_value(true);

    BT::ReturnStatus state = root->Tick();

    ASSERT_EQ(BT::RUNNING, state);
    ASSERT_EQ(BT::RUNNING, action_1->get_status());

    root->Halt();
}


TEST_F(SimpleSequenceWithMemoryTest, ConditionTrue)
{
    std::cout << "Ticking the root node !" << std::endl << std::endl;
    // Ticking the root node
    BT::ReturnStatus state = root->Tick();
    std::this_thread::sleep_for(std::chrono::seconds(1));

    ASSERT_EQ(BT::RUNNING, action->get_status());
    ASSERT_EQ(BT::RUNNING, state);
    root->Halt();
}


TEST_F(SimpleSequenceWithMemoryTest, ConditionTurnToFalse)
{
    BT::ReturnStatus state = root->Tick();

    condition->set_boolean_value(false);

    state = root->Tick();

    ASSERT_EQ(BT::RUNNING, state);
    ASSERT_EQ(BT::RUNNING, action->get_status());

    root->Halt();
}


TEST_F(ComplexSequenceWithMemoryTest, ConditionsTrue)
{
    BT::ReturnStatus state = root->Tick();

    ASSERT_EQ(BT::RUNNING, action_1->get_status());
    ASSERT_EQ(BT::IDLE, action_2->get_status());
    ASSERT_EQ(BT::RUNNING, state);

    root->Halt();
}


TEST_F(ComplexSequenceWithMemoryTest, Conditions1ToFalse)
{
    BT::ReturnStatus state = root->Tick();

    condition_1->set_boolean_value(false);

    state = root->Tick();

    ASSERT_EQ(BT::RUNNING, action_1->get_status());
    ASSERT_EQ(BT::IDLE, action_2->get_status());
    ASSERT_EQ(BT::RUNNING, state);
    root->Halt();
}

TEST_F(ComplexSequenceWithMemoryTest, Conditions2ToFalse)
{
    BT::ReturnStatus state = root->Tick();

    condition_2->set_boolean_value(false);

    state = root->Tick();

    ASSERT_EQ(BT::RUNNING, action_1->get_status());
    ASSERT_EQ(BT::IDLE, action_2->get_status());
    ASSERT_EQ(BT::RUNNING, state);

    root->Halt();
}

TEST_F(ComplexSequenceWithMemoryTest, Action1Done)
{
    root->Tick();

    condition_2->set_boolean_value(false);

    root->Tick();
    std::this_thread::sleep_for(std::chrono::seconds(10));
    root->Tick();

    ASSERT_EQ(BT::RUNNING, action_2->get_status());

    root->Halt();
}

TEST_F(SimpleFallbackWithMemoryTest, ConditionFalse)
{
    std::cout << "Ticking the root node !" << std::endl << std::endl;
    // Ticking the root node
    condition->set_boolean_value(false);
    BT::ReturnStatus state = root->Tick();
    std::this_thread::sleep_for(std::chrono::seconds(1));

    ASSERT_EQ(BT::RUNNING, action->get_status());
    ASSERT_EQ(BT::RUNNING, state);

    root->Halt();
}


TEST_F(SimpleFallbackWithMemoryTest, ConditionTurnToTrue)
{
    condition->set_boolean_value(false);

    BT::ReturnStatus state = root->Tick();
    condition->set_boolean_value(true);

    state = root->Tick();

    ASSERT_EQ(BT::RUNNING, state);
    ASSERT_EQ(BT::RUNNING, action->get_status());

    root->Halt();
}



TEST_F(ComplexFallbackWithMemoryTest, ConditionsTrue)
{
    BT::ReturnStatus state = root->Tick();

    ASSERT_EQ(BT::IDLE, action_1->get_status());
    ASSERT_EQ(BT::IDLE, action_2->get_status());
    ASSERT_EQ(BT::SUCCESS, state);

    root->Halt();
}

TEST_F(ComplexFallbackWithMemoryTest, Condition1False)
{
    condition_1->set_boolean_value(false);
    BT::ReturnStatus state = root->Tick();

    ASSERT_EQ(BT::IDLE, action_1->get_status());
    ASSERT_EQ(BT::IDLE, action_2->get_status());
    ASSERT_EQ(BT::SUCCESS, state);

    root->Halt();
}


TEST_F(ComplexFallbackWithMemoryTest, ConditionsFalse)
{
    condition_1->set_boolean_value(false);
    condition_2->set_boolean_value(false);
    BT::ReturnStatus state = root->Tick();

    ASSERT_EQ(BT::RUNNING, action_1->get_status());
    ASSERT_EQ(BT::IDLE, action_2->get_status());
    ASSERT_EQ(BT::RUNNING, state);

    root->Halt();
}




TEST_F(ComplexFallbackWithMemoryTest, Conditions1ToTrue)
{
    condition_1->set_boolean_value(false);
    condition_2->set_boolean_value(false);
    BT::ReturnStatus state = root->Tick();
    condition_1->set_boolean_value(true);

    state = root->Tick();

    ASSERT_EQ(BT::RUNNING, action_1->get_status());
    ASSERT_EQ(BT::IDLE, action_2->get_status());
    ASSERT_EQ(BT::RUNNING, state);

    root->Halt();
}

TEST_F(ComplexFallbackWithMemoryTest, Conditions2ToTrue)
{
    condition_1->set_boolean_value(false);

    condition_2->set_boolean_value(false);

    BT::ReturnStatus state = root->Tick();

    condition_2->set_boolean_value(true);

    state = root->Tick();

    ASSERT_EQ(BT::RUNNING, action_1->get_status());
    ASSERT_EQ(BT::IDLE, action_2->get_status());
    ASSERT_EQ(BT::RUNNING, state);

    root->Halt();
}

TEST_F(ComplexFallbackWithMemoryTest, Action1Failed)
{
    action_1->set_boolean_value(false);
    condition_1->set_boolean_value(false);
    condition_2->set_boolean_value(false);

    BT::ReturnStatus state = root->Tick();

    state = root->Tick();
    std::this_thread::sleep_for(std::chrono::seconds(5));
    state = root->Tick();

    ASSERT_EQ(BT::IDLE, action_1->get_status());
    ASSERT_EQ(BT::RUNNING, action_2->get_status());
    ASSERT_EQ(BT::RUNNING, state);

    root->Halt();
}


TEST_F(SimpleParallelTest, ConditionsTrue)
{
    BT::ReturnStatus state = root->Tick();

    ASSERT_EQ(BT::IDLE, condition_1->get_status());
    ASSERT_EQ(BT::IDLE, condition_2->get_status());
    ASSERT_EQ(BT::RUNNING, action_1->get_status());
    ASSERT_EQ(BT::RUNNING, action_2->get_status());
    ASSERT_EQ(BT::RUNNING, state);

    root->Halt();
}


TEST_F(SimpleParallelTest, Threshold_3)
{
    root->set_threshold_M(3);
    action_2->set_time(200);
    root->Tick();
    std::this_thread::sleep_for(std::chrono::seconds(5));
    BT::ReturnStatus state = root->Tick();

    ASSERT_EQ(BT::IDLE, condition_1->get_status());
    ASSERT_EQ(BT::IDLE, condition_2->get_status());
    ASSERT_EQ(BT::IDLE, action_1->get_status());
    ASSERT_EQ(BT::HALTED, action_2->get_status());
    ASSERT_EQ(BT::SUCCESS, state);

    root->Halt();
}


TEST_F(SimpleParallelTest, Threshold_1)
{
    root->set_threshold_M(1);
    BT::ReturnStatus state = root->Tick();

    ASSERT_EQ(BT::IDLE, condition_1->get_status());
    ASSERT_EQ(BT::IDLE, condition_2->get_status());
    ASSERT_EQ(BT::IDLE, action_1->get_status());
    ASSERT_EQ(BT::IDLE, action_2->get_status());
    ASSERT_EQ(BT::SUCCESS, state);

    root->Halt();
}
TEST_F(ComplexParallelTest, ConditionsTrue)
{
    BT::ReturnStatus state = root->Tick();

    ASSERT_EQ(BT::IDLE, condition_1->get_status());
    ASSERT_EQ(BT::IDLE, condition_2->get_status());
    ASSERT_EQ(BT::IDLE, condition_3->get_status());
    ASSERT_EQ(BT::RUNNING, action_1->get_status());
    ASSERT_EQ(BT::RUNNING, action_2->get_status());
    ASSERT_EQ(BT::IDLE, action_3->get_status());
    ASSERT_EQ(BT::RUNNING, parallel_1->get_status());
    ASSERT_EQ(BT::IDLE, parallel_2->get_status());
    ASSERT_EQ(BT::RUNNING, state);

    root->Halt();
}



TEST_F(ComplexParallelTest, Condition3False)
{
    condition_3->set_boolean_value(false);
    BT::ReturnStatus state = root->Tick();

    ASSERT_EQ(BT::IDLE, condition_1->get_status());
    ASSERT_EQ(BT::IDLE, condition_2->get_status());
    ASSERT_EQ(BT::IDLE, condition_3->get_status());
    ASSERT_EQ(BT::RUNNING, action_1->get_status());
    ASSERT_EQ(BT::RUNNING, action_2->get_status());
    ASSERT_EQ(BT::RUNNING, action_3->get_status());
    ASSERT_EQ(BT::RUNNING, parallel_1->get_status());
    ASSERT_EQ(BT::RUNNING, parallel_2->get_status());
    ASSERT_EQ(BT::RUNNING, state);

    root->Halt();
}


TEST_F(ComplexParallelTest, Condition3FalseAction1Done)
{
    action_2->set_time(10);
    action_3->set_time(10);

    condition_3->set_boolean_value(false);
    BT::ReturnStatus state = root->Tick();
    std::this_thread::sleep_for(std::chrono::seconds(5));


    ASSERT_EQ(BT::IDLE, condition_1->get_status());
    ASSERT_EQ(BT::IDLE, condition_2->get_status());
    ASSERT_EQ(BT::IDLE, condition_3->get_status());
    ASSERT_EQ(BT::SUCCESS, action_1->get_status());  // success not read yet by the node parallel_1
    ASSERT_EQ(BT::RUNNING, parallel_1->get_status());  // parallel_1 hasn't realize (yet) that action_1 has succeeded

    state = root->Tick();

    ASSERT_EQ(BT::IDLE, action_1->get_status());
    ASSERT_EQ(BT::IDLE, parallel_1->get_status());
    ASSERT_EQ(BT::HALTED, action_2->get_status());
    ASSERT_EQ(BT::RUNNING, action_3->get_status());
    ASSERT_EQ(BT::RUNNING, parallel_2->get_status());
    ASSERT_EQ(BT::RUNNING, state);


    state = root->Tick();
    std::this_thread::sleep_for(std::chrono::seconds(15));
    state = root->Tick();

    ASSERT_EQ(BT::IDLE, parallel_2->get_status());
    ASSERT_EQ(BT::IDLE, action_1->get_status());
    ASSERT_EQ(BT::IDLE, parallel_1->get_status());
    ASSERT_EQ(BT::IDLE, action_3->get_status());
    ASSERT_EQ(BT::IDLE, parallel_2->get_status());
    ASSERT_EQ(BT::SUCCESS, state);

    root->Halt();
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}


