/* Copyright (C) 2015-2017 Michele Colledanchise -  All Rights Reserved
 * Copyright (C) 2018-2019 Davide Faconti, Eurecat -  All Rights Reserved
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

#include "action_test_node.h"
#include <string>

BT::AsyncActionTest::AsyncActionTest(const std::string& name, BT::Duration deadline_ms)
  : ThreadedAction(name, {}), success_count_(0), failure_count_(0)
{
  expected_result_ = NodeStatus::SUCCESS;
  time_ = deadline_ms;
  tick_count_ = 0;
}

BT::NodeStatus BT::AsyncActionTest::tick()
{
  using std::chrono::high_resolution_clock;
  tick_count_++;

  auto initial_time = high_resolution_clock::now();

  // we simulate an asynchronous action that takes an amount of time equal to time_
  while(!isHaltRequested() && high_resolution_clock::now() < initial_time + time_)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }

  // check if we exited the while() loop because of the flag stop_loop_
  if(isHaltRequested())
  {
    return NodeStatus::IDLE;
  }

  if(expected_result_ == NodeStatus::SUCCESS)
  {
    success_count_++;
  }
  else if(expected_result_ == NodeStatus::FAILURE)
  {
    failure_count_++;
  }

  return expected_result_;
}

void BT::AsyncActionTest::halt()
{
  // do more cleanup here if necessary
  ThreadedAction::halt();
}

void BT::AsyncActionTest::setTime(BT::Duration time)
{
  time_ = time;
}

void BT::AsyncActionTest::setExpectedResult(BT::NodeStatus res)
{
  expected_result_ = res;
}

//----------------------------------------------

BT::SyncActionTest::SyncActionTest(const std::string& name) : SyncActionNode(name, {})
{
  tick_count_ = 0;
  expected_result_ = NodeStatus::SUCCESS;
}

BT::NodeStatus BT::SyncActionTest::tick()
{
  tick_count_++;
  return expected_result_;
}

void BT::SyncActionTest::setExpectedResult(NodeStatus res)
{
  expected_result_ = res;
}
