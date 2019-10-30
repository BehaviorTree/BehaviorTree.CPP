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

struct SimpleParallelTest : testing::Test
{
    BT::ParallelNode root;
    BT::AsyncActionTest action_1;
    BT::ConditionTestNode condition_1;

    BT::AsyncActionTest action_2;
    BT::ConditionTestNode condition_2;

    SimpleParallelTest()
      : root("root_parallel", 4)
      , action_1("action_1", milliseconds(100) )
      , condition_1("condition_1")
      , action_2("action_2", milliseconds(300))
      , condition_2("condition_2")
    {
        root.addChild(&condition_1);
        root.addChild(&action_1);
        root.addChild(&condition_2);
        root.addChild(&action_2);
    }
    ~SimpleParallelTest()
    {
        haltAllActions(&root);
    }
};

struct ComplexParallelTest : testing::Test
{
    BT::ParallelNode parallel_root;
    BT::ParallelNode parallel_left;
    BT::ParallelNode parallel_right;

    BT::AsyncActionTest action_L1;
    BT::ConditionTestNode condition_L1;

    BT::AsyncActionTest action_L2;
    BT::ConditionTestNode condition_L2;

    BT::AsyncActionTest action_R;
    BT::ConditionTestNode condition_R;

    ComplexParallelTest()
      : parallel_root("root", 2)
      , parallel_left("par1", 3)
      , parallel_right("par2", 1)
      , action_L1("action_1", milliseconds(100) )
      , condition_L1("condition_1")
      , action_L2("action_2", milliseconds(200) )
      , condition_L2("condition_2")
      , action_R("action_3", milliseconds(400) )
      , condition_R("condition_3")
    {
        parallel_root.addChild(&parallel_left);
        {
            parallel_left.addChild(&condition_L1);
            parallel_left.addChild(&action_L1);
            parallel_left.addChild(&condition_L2);
            parallel_left.addChild(&action_L2);
        }
        parallel_root.addChild(&parallel_right);
        {
            parallel_right.addChild(&condition_R);
            parallel_right.addChild(&action_R);
        }
    }
    ~ComplexParallelTest()
    {
        haltAllActions(&parallel_root);
    }
};

/****************TESTS START HERE***************************/

TEST_F(SimpleParallelTest, ConditionsTrue)
{
    BT::NodeStatus state = root.executeTick();

    ASSERT_EQ(NodeStatus::SUCCESS, condition_1.status());
    ASSERT_EQ(NodeStatus::SUCCESS, condition_2.status());
    ASSERT_EQ(NodeStatus::RUNNING, action_1.status());
    ASSERT_EQ(NodeStatus::RUNNING, action_2.status());
    ASSERT_EQ(NodeStatus::RUNNING, state);

    std::this_thread::sleep_for( milliseconds(200) );
    state = root.executeTick();

    ASSERT_EQ(NodeStatus::SUCCESS, condition_1.status());
    ASSERT_EQ(NodeStatus::SUCCESS, condition_2.status());
    ASSERT_EQ(NodeStatus::SUCCESS, action_1.status());
    ASSERT_EQ(NodeStatus::RUNNING, action_2.status());
    ASSERT_EQ(NodeStatus::RUNNING, state);

    std::this_thread::sleep_for( milliseconds(200) );
    state = root.executeTick();

    ASSERT_EQ(NodeStatus::IDLE, condition_1.status());
    ASSERT_EQ(NodeStatus::IDLE, condition_2.status());
    ASSERT_EQ(NodeStatus::IDLE, action_1.status());
    ASSERT_EQ(NodeStatus::IDLE, action_2.status());
    ASSERT_EQ(NodeStatus::SUCCESS, state);
}

TEST_F(SimpleParallelTest, Threshold_3)
{
    root.setThresholdM(3);
    action_1.setTime( milliseconds(100) );
    action_2.setTime( milliseconds(500) ); // this takes a lot of time

    BT::NodeStatus state = root.executeTick();
    // first tick, zero wait
    ASSERT_EQ(NodeStatus::SUCCESS, condition_1.status());
    ASSERT_EQ(NodeStatus::SUCCESS, condition_2.status());
    ASSERT_EQ(NodeStatus::RUNNING, action_1.status());
    ASSERT_EQ(NodeStatus::RUNNING, action_2.status());
    ASSERT_EQ(NodeStatus::RUNNING, state);

    std::this_thread::sleep_for( milliseconds(150) );
    state = root.executeTick();
    // second tick: action1 should be completed, but not action2
    // nevertheless it is sufficient because threshold is 3
    ASSERT_EQ(NodeStatus::IDLE, condition_1.status());
    ASSERT_EQ(NodeStatus::IDLE, condition_2.status());
    ASSERT_EQ(NodeStatus::IDLE, action_1.status());
    ASSERT_EQ(NodeStatus::IDLE, action_2.status());
    ASSERT_EQ(NodeStatus::SUCCESS, state);
}

TEST_F(SimpleParallelTest, Threshold_1)
{
    root.setThresholdM(2);
    BT::NodeStatus state = root.executeTick();

    ASSERT_EQ(NodeStatus::IDLE, condition_1.status());
    ASSERT_EQ(NodeStatus::IDLE, condition_2.status());
    ASSERT_EQ(NodeStatus::IDLE, action_1.status());
    ASSERT_EQ(NodeStatus::IDLE, action_2.status());
    ASSERT_EQ(NodeStatus::SUCCESS, state);
}

TEST_F(ComplexParallelTest, ConditionsTrue)
{
    BT::NodeStatus state = parallel_root.executeTick();

    ASSERT_EQ(NodeStatus::RUNNING, parallel_left.status());
    ASSERT_EQ(NodeStatus::SUCCESS, condition_L1.status());
    ASSERT_EQ(NodeStatus::SUCCESS, condition_L2.status());
    ASSERT_EQ(NodeStatus::RUNNING, action_L1.status());
    ASSERT_EQ(NodeStatus::RUNNING, action_L2.status());

    ASSERT_EQ(NodeStatus::SUCCESS, parallel_right.status());
    ASSERT_EQ(NodeStatus::IDLE, condition_R.status());
    ASSERT_EQ(NodeStatus::IDLE, action_R.status());

    ASSERT_EQ(NodeStatus::RUNNING, state);
    //----------------------------------------
    std::this_thread::sleep_for(milliseconds(200));
    state = parallel_root.executeTick();

    ASSERT_EQ(NodeStatus::IDLE, parallel_left.status());
    ASSERT_EQ(NodeStatus::IDLE, condition_L1.status());
    ASSERT_EQ(NodeStatus::IDLE, condition_L2.status());
    ASSERT_EQ(NodeStatus::IDLE, action_L1.status());
    ASSERT_EQ(NodeStatus::IDLE, action_L2.status());

    ASSERT_EQ(NodeStatus::IDLE, parallel_right.status());
    ASSERT_EQ(NodeStatus::IDLE, condition_R.status());
    ASSERT_EQ(NodeStatus::IDLE, action_R.status());

    ASSERT_EQ(NodeStatus::SUCCESS, state);
}

TEST_F(ComplexParallelTest, ConditionRightFalse)
{
    condition_R.setBoolean(false);
    BT::NodeStatus state = parallel_root.executeTick();

    // All the actions are running

    ASSERT_EQ(NodeStatus::RUNNING, parallel_left.status());
    ASSERT_EQ(NodeStatus::SUCCESS, condition_L1.status());
    ASSERT_EQ(NodeStatus::SUCCESS, condition_L2.status());
    ASSERT_EQ(NodeStatus::RUNNING, action_L1.status());
    ASSERT_EQ(NodeStatus::RUNNING, action_L2.status());

    ASSERT_EQ(NodeStatus::RUNNING, parallel_right.status());
    ASSERT_EQ(NodeStatus::FAILURE, condition_R.status());
    ASSERT_EQ(NodeStatus::RUNNING, action_R.status());

    ASSERT_EQ(NodeStatus::RUNNING, state);

    //----------------------------------------
    std::this_thread::sleep_for(milliseconds(500));
    state = parallel_root.executeTick();

    ASSERT_EQ(NodeStatus::IDLE, parallel_left.status());
    ASSERT_EQ(NodeStatus::IDLE, condition_L1.status());
    ASSERT_EQ(NodeStatus::IDLE, condition_L2.status());
    ASSERT_EQ(NodeStatus::IDLE, action_L1.status());
    ASSERT_EQ(NodeStatus::IDLE, action_L2.status());

    ASSERT_EQ(NodeStatus::IDLE, parallel_right.status());
    ASSERT_EQ(NodeStatus::IDLE, condition_R.status());
    ASSERT_EQ(NodeStatus::IDLE, action_R.status());

    ASSERT_EQ(NodeStatus::SUCCESS, state);
}

TEST_F(ComplexParallelTest, ConditionRightFalseAction1Done)
{
    condition_R.setBoolean(false);

    parallel_left.setThresholdM(4);

    BT::NodeStatus state = parallel_root.executeTick();
    std::this_thread::sleep_for(milliseconds(300));

    // parallel_1 hasn't realize (yet) that action_1 has succeeded
    ASSERT_EQ(NodeStatus::RUNNING, parallel_left.status());
    ASSERT_EQ(NodeStatus::SUCCESS, condition_L1.status());
    ASSERT_EQ(NodeStatus::SUCCESS, condition_L2.status());
    ASSERT_EQ(NodeStatus::SUCCESS, action_L1.status());
    ASSERT_EQ(NodeStatus::SUCCESS, action_L2.status());

    ASSERT_EQ(NodeStatus::RUNNING, parallel_right.status());

    //------------------------
    state = parallel_root.executeTick();

    ASSERT_EQ(NodeStatus::SUCCESS, parallel_left.status());
    ASSERT_EQ(NodeStatus::IDLE, condition_L1.status());
    ASSERT_EQ(NodeStatus::IDLE, condition_L2.status());
    ASSERT_EQ(NodeStatus::IDLE, action_L1.status());
    ASSERT_EQ(NodeStatus::IDLE, action_L2.status());

    ASSERT_EQ(NodeStatus::RUNNING, parallel_right.status());
    ASSERT_EQ(NodeStatus::RUNNING, action_R.status());

    ASSERT_EQ(NodeStatus::RUNNING, state);

    //----------------------------------
    std::this_thread::sleep_for(milliseconds(300));
    state = parallel_root.executeTick();

    ASSERT_EQ(NodeStatus::IDLE, parallel_left.status());
    ASSERT_EQ(NodeStatus::IDLE, action_L1.status());

    ASSERT_EQ(NodeStatus::IDLE, parallel_right.status());
    ASSERT_EQ(NodeStatus::IDLE, action_R.status());

    ASSERT_EQ(NodeStatus::SUCCESS, state);
}
