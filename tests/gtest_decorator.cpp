/* Copyright (C) 2018-2019 Davide Faconti, Eurecat - All Rights Reserved
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

struct DeadlineTest : testing::Test
{
    BT::TimeoutNode root;
    BT::AsyncActionTest action;

    DeadlineTest() : root("deadline", 300)
      , action("action", milliseconds(500) )
    {
        root.setChild(&action);
    }
    ~DeadlineTest()
    {
        haltAllActions(&root);
    }
};

struct RepeatTest : testing::Test
{
    BT::RepeatNode root;
    BT::SyncActionTest action;

    RepeatTest() : root("repeat", 3), action("action")
    {
        root.setChild(&action);
    }
    ~RepeatTest()
    {
        haltAllActions(&root);
    }
};

struct RetryTest : testing::Test
{
    BT::RetryNode root;
    BT::SyncActionTest action;

    RetryTest() : root("retry", 3), action("action")
    {
        root.setChild(&action);
    }
    ~RetryTest()
    {
        haltAllActions(&root);
    }
};

struct TimeoutAndRetry : testing::Test
{
    BT::TimeoutNode timeout_root;
    BT::RetryNode retry;
    BT::SyncActionTest action;

    TimeoutAndRetry() :
      timeout_root("deadline", 9)
      , retry("retry", 1000)
      , action("action")
    {
        timeout_root.setChild(&retry);
        retry.setChild(&action);
    }
    ~TimeoutAndRetry()
    {
        haltAllActions(&timeout_root);
    }
};

/****************TESTS START HERE***************************/

TEST_F(DeadlineTest, DeadlineTriggeredTest)
{
    BT::NodeStatus state = root.executeTick();
    // deadline in 300 ms, action requires 500 ms

    ASSERT_EQ(NodeStatus::RUNNING, action.status());
    ASSERT_EQ(NodeStatus::RUNNING, state);

    std::this_thread::sleep_for(std::chrono::milliseconds(400));
    state = root.executeTick();
    ASSERT_EQ(NodeStatus::FAILURE, state);
    ASSERT_EQ(NodeStatus::IDLE, action.status());
}

TEST_F(DeadlineTest, DeadlineNotTriggeredTest)
{
    action.setTime( milliseconds(200) );
    // deadline in 300 ms

    BT::NodeStatus state = root.executeTick();

    ASSERT_EQ(NodeStatus::RUNNING, action.status());
    ASSERT_EQ(NodeStatus::RUNNING, state);

    std::this_thread::sleep_for(std::chrono::milliseconds(400));
    state = root.executeTick();
    ASSERT_EQ(NodeStatus::IDLE, action.status());
    ASSERT_EQ(NodeStatus::SUCCESS, state);
}

TEST_F(RetryTest, RetryTestA)
{
    action.setBoolean(false);

    root.executeTick();
    ASSERT_EQ(NodeStatus::FAILURE, root.status());
    ASSERT_EQ(3, action.tickCount() );

    action.setBoolean(true);
    action.resetTicks();

    root.executeTick();
    ASSERT_EQ(NodeStatus::SUCCESS, root.status());
    ASSERT_EQ(1, action.tickCount() );
}

TEST_F(RepeatTest, RepeatTestA)
{
    action.setBoolean(false);

    root.executeTick();
    ASSERT_EQ(NodeStatus::FAILURE, root.status());
    ASSERT_EQ(1, action.tickCount() );

    //-------------------
    action.resetTicks();
    action.setBoolean(true);

    root.executeTick();
    ASSERT_EQ(NodeStatus::SUCCESS, root.status());
    ASSERT_EQ(3, action.tickCount() );
}

// https://github.com/BehaviorTree/BehaviorTree.CPP/issues/57
TEST_F(TimeoutAndRetry, Issue57)
{
    action.setBoolean( false );

    auto t1 = std::chrono::high_resolution_clock::now();

    while( std::chrono::high_resolution_clock::now() < t1 + std::chrono::seconds(2) )
    {
        ASSERT_NE( timeout_root.executeTick(), BT::NodeStatus::IDLE );
        std::this_thread::sleep_for( std::chrono::microseconds(50) );
    }
}
