/* Copyright (C) 2025 Davide Faconti -  All Rights Reserved
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

class TryCatchTest : public testing::Test
{
protected:
  BT::BehaviorTreeFactory factory;
  std::array<int, 4> counters;

  void SetUp() override
  {
    RegisterTestTick(factory, "Test", counters);
  }
};

TEST_F(TryCatchTest, AllTryChildrenSucceed)
{
  const std::string xml_text = R"(
    <root BTCPP_format="4">
       <BehaviorTree>
          <TryCatch>
            <TestA/>
            <TestB/>
            <TestC/>  <!-- catch -->
          </TryCatch>
       </BehaviorTree>
    </root>)";

  auto tree = factory.createTreeFromText(xml_text);
  auto status = tree.tickWhileRunning();

  ASSERT_EQ(status, NodeStatus::SUCCESS);
  ASSERT_EQ(counters[0], 1);  // TestA executed
  ASSERT_EQ(counters[1], 1);  // TestB executed
  ASSERT_EQ(counters[2], 0);  // TestC (catch) NOT executed
}

TEST_F(TryCatchTest, FirstChildFails_CatchExecuted)
{
  const std::string xml_text = R"(
    <root BTCPP_format="4">
       <BehaviorTree>
          <TryCatch>
            <AlwaysFailure/>
            <TestA/>
            <TestB/>  <!-- catch -->
          </TryCatch>
       </BehaviorTree>
    </root>)";

  auto tree = factory.createTreeFromText(xml_text);
  auto status = tree.tickWhileRunning();

  ASSERT_EQ(status, NodeStatus::FAILURE);
  ASSERT_EQ(counters[0], 0);  // TestA NOT executed (after failed child)
  ASSERT_EQ(counters[1], 1);  // TestB (catch) executed
}

TEST_F(TryCatchTest, SecondChildFails_CatchExecuted)
{
  const std::string xml_text = R"(
    <root BTCPP_format="4">
       <BehaviorTree>
          <TryCatch>
            <TestA/>
            <AlwaysFailure/>
            <TestB/>  <!-- catch -->
          </TryCatch>
       </BehaviorTree>
    </root>)";

  auto tree = factory.createTreeFromText(xml_text);
  auto status = tree.tickWhileRunning();

  ASSERT_EQ(status, NodeStatus::FAILURE);
  ASSERT_EQ(counters[0], 1);  // TestA executed (before failure)
  ASSERT_EQ(counters[1], 1);  // TestB (catch) executed
}

TEST_F(TryCatchTest, CatchReturnsFailure_NodeStillReturnsFAILURE)
{
  const std::string xml_text = R"(
    <root BTCPP_format="4">
       <BehaviorTree>
          <TryCatch>
            <AlwaysFailure/>  <!-- try fails -->
            <AlwaysFailure/>  <!-- catch also fails -->
          </TryCatch>
       </BehaviorTree>
    </root>)";

  auto tree = factory.createTreeFromText(xml_text);
  auto status = tree.tickWhileRunning();

  ASSERT_EQ(status, NodeStatus::FAILURE);
}

TEST_F(TryCatchTest, CatchReturnsSuccess_NodeStillReturnsFAILURE)
{
  const std::string xml_text = R"(
    <root BTCPP_format="4">
       <BehaviorTree>
          <TryCatch>
            <AlwaysFailure/>  <!-- try fails -->
            <AlwaysSuccess/>  <!-- catch succeeds -->
          </TryCatch>
       </BehaviorTree>
    </root>)";

  auto tree = factory.createTreeFromText(xml_text);
  auto status = tree.tickWhileRunning();

  // Even if catch succeeds, TryCatch returns FAILURE
  ASSERT_EQ(status, NodeStatus::FAILURE);
}

TEST_F(TryCatchTest, TryChildRunning)
{
  int tick_count = 0;
  factory.registerSimpleCondition("RunningThenSuccess", [&tick_count](BT::TreeNode&) {
    tick_count++;
    if(tick_count == 1)
    {
      return NodeStatus::RUNNING;
    }
    return NodeStatus::SUCCESS;
  });

  const std::string xml_text = R"(
    <root BTCPP_format="4">
       <BehaviorTree>
          <TryCatch>
            <RunningThenSuccess/>
            <TestA/>  <!-- catch -->
          </TryCatch>
       </BehaviorTree>
    </root>)";

  auto tree = factory.createTreeFromText(xml_text);

  auto status = tree.tickOnce();
  ASSERT_EQ(status, NodeStatus::RUNNING);

  status = tree.tickWhileRunning();
  ASSERT_EQ(status, NodeStatus::SUCCESS);
  ASSERT_EQ(counters[0], 0);  // Catch NOT executed
}

TEST_F(TryCatchTest, CatchChildRunning)
{
  int catch_tick_count = 0;
  factory.registerSimpleCondition("RunningThenFailure",
                                  [&catch_tick_count](BT::TreeNode&) {
                                    catch_tick_count++;
                                    if(catch_tick_count == 1)
                                    {
                                      return NodeStatus::RUNNING;
                                    }
                                    return NodeStatus::FAILURE;
                                  });

  const std::string xml_text = R"(
    <root BTCPP_format="4">
       <BehaviorTree>
          <TryCatch>
            <AlwaysFailure/>       <!-- try fails -->
            <RunningThenFailure/>  <!-- catch: RUNNING first, then FAILURE -->
          </TryCatch>
       </BehaviorTree>
    </root>)";

  auto tree = factory.createTreeFromText(xml_text);

  // First tick: try fails, catch starts and returns RUNNING
  auto status = tree.tickOnce();
  ASSERT_EQ(status, NodeStatus::RUNNING);

  // Second tick: catch returns FAILURE, TryCatch returns FAILURE
  status = tree.tickWhileRunning();
  ASSERT_EQ(status, NodeStatus::FAILURE);
}

TEST_F(TryCatchTest, MinimumTwoChildren_ParseTimeValidation)
{
  const std::string xml_text = R"(
    <root BTCPP_format="4">
       <BehaviorTree>
          <TryCatch>
            <AlwaysSuccess/>
          </TryCatch>
       </BehaviorTree>
    </root>)";

  // Error should be caught at parse time, not tick time
  ASSERT_THROW(factory.createTreeFromText(xml_text), BT::RuntimeError);
}

TEST_F(TryCatchTest, ReExecuteAfterSuccess)
{
  const std::string xml_text = R"(
    <root BTCPP_format="4">
       <BehaviorTree>
          <TryCatch>
            <TestA/>
            <TestB/>  <!-- catch -->
          </TryCatch>
       </BehaviorTree>
    </root>)";

  auto tree = factory.createTreeFromText(xml_text);

  auto status = tree.tickWhileRunning();
  ASSERT_EQ(status, NodeStatus::SUCCESS);
  ASSERT_EQ(counters[0], 1);

  tree.haltTree();
  status = tree.tickWhileRunning();
  ASSERT_EQ(status, NodeStatus::SUCCESS);
  ASSERT_EQ(counters[0], 2);  // TestA executed again
  ASSERT_EQ(counters[1], 0);  // Catch still never executed
}

TEST_F(TryCatchTest, ReExecuteAfterFailure)
{
  int try_tick_count = 0;
  factory.registerSimpleAction("FailThenSucceed", [&try_tick_count](BT::TreeNode&) {
    try_tick_count++;
    if(try_tick_count == 1)
    {
      return NodeStatus::FAILURE;
    }
    return NodeStatus::SUCCESS;
  });

  const std::string xml_text = R"(
    <root BTCPP_format="4">
       <BehaviorTree>
          <TryCatch>
            <FailThenSucceed/>
            <TestA/>  <!-- catch -->
          </TryCatch>
       </BehaviorTree>
    </root>)";

  auto tree = factory.createTreeFromText(xml_text);

  // First execution: try fails, catch runs
  auto status = tree.tickWhileRunning();
  ASSERT_EQ(status, NodeStatus::FAILURE);
  ASSERT_EQ(counters[0], 1);  // Catch executed

  // Second execution: try succeeds
  tree.haltTree();
  status = tree.tickWhileRunning();
  ASSERT_EQ(status, NodeStatus::SUCCESS);
  ASSERT_EQ(counters[0], 1);  // Catch not executed again
}

TEST_F(TryCatchTest, CatchOnHalt_Disabled)
{
  int catch_count = 0;
  factory.registerSimpleAction("CountCatch", [&catch_count](BT::TreeNode&) {
    catch_count++;
    return NodeStatus::SUCCESS;
  });

  int try_ticks = 0;
  factory.registerSimpleCondition("AlwaysRunning", [&try_ticks](BT::TreeNode&) {
    try_ticks++;
    return NodeStatus::RUNNING;
  });

  const std::string xml_text = R"(
    <root BTCPP_format="4">
       <BehaviorTree>
          <TryCatch>
            <AlwaysRunning/>
            <CountCatch/>  <!-- catch -->
          </TryCatch>
       </BehaviorTree>
    </root>)";

  auto tree = factory.createTreeFromText(xml_text);

  auto status = tree.tickOnce();
  ASSERT_EQ(status, NodeStatus::RUNNING);

  // Halt while try-block is RUNNING; catch_on_halt defaults to false
  tree.haltTree();
  ASSERT_EQ(catch_count, 0);  // Catch NOT executed on halt
}

TEST_F(TryCatchTest, CatchOnHalt_Enabled)
{
  int catch_count = 0;
  factory.registerSimpleAction("CountCatch", [&catch_count](BT::TreeNode&) {
    catch_count++;
    return NodeStatus::SUCCESS;
  });

  int try_ticks = 0;
  factory.registerSimpleCondition("AlwaysRunning", [&try_ticks](BT::TreeNode&) {
    try_ticks++;
    return NodeStatus::RUNNING;
  });

  const std::string xml_text = R"(
    <root BTCPP_format="4">
       <BehaviorTree>
          <TryCatch catch_on_halt="true">
            <AlwaysRunning/>
            <CountCatch/>  <!-- catch -->
          </TryCatch>
       </BehaviorTree>
    </root>)";

  auto tree = factory.createTreeFromText(xml_text);

  auto status = tree.tickOnce();
  ASSERT_EQ(status, NodeStatus::RUNNING);

  // Halt while try-block is RUNNING; catch_on_halt is true
  tree.haltTree();
  ASSERT_EQ(catch_count, 1);  // Catch executed on halt
}

TEST_F(TryCatchTest, CatchOnHalt_NotTriggeredWhenAlreadyInCatch)
{
  int catch_ticks = 0;
  factory.registerSimpleCondition("RunningCatch", [&catch_ticks](BT::TreeNode&) {
    catch_ticks++;
    return NodeStatus::RUNNING;
  });

  const std::string xml_text = R"(
    <root BTCPP_format="4">
       <BehaviorTree>
          <TryCatch catch_on_halt="true">
            <AlwaysFailure/>  <!-- try fails immediately -->
            <RunningCatch/>   <!-- catch returns RUNNING -->
          </TryCatch>
       </BehaviorTree>
    </root>)";

  auto tree = factory.createTreeFromText(xml_text);

  // First tick: try fails, enters catch, catch returns RUNNING
  auto status = tree.tickOnce();
  ASSERT_EQ(status, NodeStatus::RUNNING);
  ASSERT_EQ(catch_ticks, 1);

  // Halt while in catch mode: should NOT re-trigger catch
  tree.haltTree();
  ASSERT_EQ(catch_ticks, 1);  // Catch NOT ticked again
}

TEST_F(TryCatchTest, AsyncCatchCompletesInsideSequence)
{
  // The catch child returns RUNNING for 5 ticks, then SUCCESS.
  // Verify that the Sequence keeps ticking TryCatch, which keeps
  // ticking the catch child until it completes.
  const int kRunningTicks = 5;
  int catch_ticks = 0;
  factory.registerSimpleCondition("AsyncCleanup",
                                  [&catch_ticks, kRunningTicks](BT::TreeNode&) {
                                    catch_ticks++;
                                    if(catch_ticks <= kRunningTicks)
                                    {
                                      return NodeStatus::RUNNING;
                                    }
                                    return NodeStatus::SUCCESS;
                                  });

  const std::string xml_text = R"(
    <root BTCPP_format="4">
       <BehaviorTree>
          <Sequence>
            <TryCatch>
              <AlwaysFailure/>    <!-- try: fails immediately -->
              <AsyncCleanup/>     <!-- catch: RUNNING for 5 ticks, then SUCCESS -->
            </TryCatch>
            <TestA/>              <!-- should NOT execute: TryCatch returns FAILURE -->
          </Sequence>
       </BehaviorTree>
    </root>)";

  auto tree = factory.createTreeFromText(xml_text);

  // Tick-by-tick: the tree should stay RUNNING while catch is async
  for(int i = 0; i < kRunningTicks; i++)
  {
    auto status = tree.tickOnce();
    ASSERT_EQ(status, NodeStatus::RUNNING) << "Expected RUNNING on tick " << (i + 1);
    ASSERT_EQ(catch_ticks, i + 1);
  }

  // Next tick: catch completes → TryCatch returns FAILURE → Sequence returns FAILURE
  auto status = tree.tickOnce();
  ASSERT_EQ(status, NodeStatus::FAILURE);

  // Catch child was ticked exactly kRunningTicks + 1 times (5 RUNNING + 1 SUCCESS)
  ASSERT_EQ(catch_ticks, kRunningTicks + 1);

  // TestA was never reached because TryCatch returned FAILURE
  ASSERT_EQ(counters[0], 0);
}

TEST_F(TryCatchTest, SingleTryChild_Success)
{
  const std::string xml_text = R"(
    <root BTCPP_format="4">
       <BehaviorTree>
          <TryCatch>
            <TestA/>   <!-- single try child -->
            <TestB/>   <!-- catch -->
          </TryCatch>
       </BehaviorTree>
    </root>)";

  auto tree = factory.createTreeFromText(xml_text);
  auto status = tree.tickWhileRunning();

  ASSERT_EQ(status, NodeStatus::SUCCESS);
  ASSERT_EQ(counters[0], 1);
  ASSERT_EQ(counters[1], 0);
}

TEST_F(TryCatchTest, ManyTryChildren_ThirdFails)
{
  const std::string xml_text = R"(
    <root BTCPP_format="4">
       <BehaviorTree>
          <TryCatch>
            <TestA/>
            <TestB/>
            <AlwaysFailure/>
            <TestC/>  <!-- catch -->
          </TryCatch>
       </BehaviorTree>
    </root>)";

  auto tree = factory.createTreeFromText(xml_text);
  auto status = tree.tickWhileRunning();

  ASSERT_EQ(status, NodeStatus::FAILURE);
  ASSERT_EQ(counters[0], 1);  // TestA executed
  ASSERT_EQ(counters[1], 1);  // TestB executed
  ASSERT_EQ(counters[2], 1);  // TestC (catch) executed
}
