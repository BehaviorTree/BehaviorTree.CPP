#include <gtest/gtest.h>
#include <string>
#include "behaviortree_cpp/basic_types.h"
#include "behaviortree_cpp/bt_factory.h"
#include "test_helper.hpp"
#include "action_test_node.h"

using namespace BT;

TEST(SkippingLogic, Sequence)
{
  BehaviorTreeFactory factory;
  std::array<int, 2> counters;
  RegisterTestTick(factory, "Test", counters);

  const std::string xml_text = R"(

    <root BTCPP_format="4" >
        <BehaviorTree ID="MainTree">
            <Sequence>
                <Script code = "A:=1"/>
                <TestA _successIf="A==2" _failureIf="A!=1" _skipIf="A==1"/>
                <TestB/>
            </Sequence>
        </BehaviorTree>
    </root>)";

  auto tree = factory.createTreeFromText(xml_text);
  const auto status = tree.tickWhileRunning();
  ASSERT_EQ(status, NodeStatus::SUCCESS);
  ASSERT_EQ(counters[0], 0);
  ASSERT_EQ(counters[1], 1);
}

TEST(SkippingLogic, SkipAll)
{
  BehaviorTreeFactory factory;
  std::array<int, 3> counters;
  RegisterTestTick(factory, "Test", counters);

  const std::string xml_text = R"(

    <root BTCPP_format="4" >
        <BehaviorTree ID="MainTree">
            <Sequence>
                <TestA _skipIf="A==1"/>
                <TestB _skipIf="A<2"/>
                <TestC _skipIf="A>0"/>
            </Sequence>
        </BehaviorTree>
    </root>)";

  auto tree = factory.createTreeFromText(xml_text);
  tree.rootBlackboard()->set("A", 1);

  const auto status = tree.tickWhileRunning();
  ASSERT_EQ(counters[0], 0);
  ASSERT_EQ(counters[1], 0);
  ASSERT_EQ(counters[2], 0);
  ASSERT_EQ(status, NodeStatus::SKIPPED);
}

TEST(SkippingLogic, SkipSubtree)
{
  BehaviorTreeFactory factory;
  std::array<int, 3> counters;
  RegisterTestTick(factory, "Test", counters);

  const std::string xml_text = R"(

    <root BTCPP_format="4" >
        <BehaviorTree ID="main">
            <Sequence>
                <TestA/>
                <Script code=" data:=true "/>
                <SubTree ID="sub" _skipIf="data"/>
            </Sequence>
        </BehaviorTree>

        <BehaviorTree ID="sub">
            <TestB/>
        </BehaviorTree>
    </root>)";

  factory.registerBehaviorTreeFromText(xml_text);
  auto tree = factory.createTree("main");

  tree.rootBlackboard()->set("A", 1);

  const auto status = tree.tickWhileRunning();
  ASSERT_EQ(counters[0], 1);
  ASSERT_EQ(counters[1], 0);
  ASSERT_EQ(status, NodeStatus::SUCCESS);
}

TEST(SkippingLogic, ReactiveSingleChild)
{
  static const char* xml_text = R"(
    <root BTCPP_format="4">
      <BehaviorTree ID="Untitled">
        <ReactiveSequence>
          <AlwaysSuccess _skipIf="flag"/>
        </ReactiveSequence>
      </BehaviorTree>
    </root>
 )";

  BT::BehaviorTreeFactory factory;
  auto root_blackboard = BT::Blackboard::create();
  root_blackboard->set<bool>("flag", true);

  auto tree = factory.createTreeFromText(xml_text, root_blackboard);

  tree.tickWhileRunning();
}

TEST(SkippingLogic, SkippingReactiveSequence)
{
  BehaviorTreeFactory factory;
  std::array<int, 2> counters;
  RegisterTestTick(factory, "Test", counters);

  const std::string xml_text_noskip = R"(
    <root BTCPP_format="4" >
       <BehaviorTree>
          <ReactiveSequence>
            <Script code=" value:=50 "/>
            <TestA _skipIf="value < 25"/>
            <AsyncActionTest/>
          </ReactiveSequence>
       </BehaviorTree>
    </root>)";

  const std::string xml_text_skip = R"(
    <root BTCPP_format="4" >
       <BehaviorTree>
          <ReactiveSequence>
            <Script code=" value:=10 "/>
            <TestB _skipIf="value < 25"/>
            <AsyncActionTest/>
          </ReactiveSequence>
       </BehaviorTree>
    </root>)";

  factory.registerNodeType<AsyncActionTest>("AsyncActionTest");

  int expected_test_A_ticks = 0;

  for(auto const* xml_text : { &xml_text_noskip, &xml_text_skip })
  {
    auto tree = factory.createTreeFromText(*xml_text);

    for(int repeat = 0; repeat < 3; repeat++)
    {
      NodeStatus status = NodeStatus::IDLE;
      while(!isStatusCompleted(status))
      {
        status = tree.tickOnce();

        if(xml_text == &xml_text_noskip)
        {
          expected_test_A_ticks++;
        }

        tree.sleep(std::chrono::milliseconds{ 15 });
      }
      ASSERT_EQ(status, NodeStatus::SUCCESS);
    }
  }
  // counters[0] contains the number of times TestA was ticked
  ASSERT_EQ(counters[0], expected_test_A_ticks);

  // counters[1] contains the number of times TestB was ticked
  ASSERT_EQ(counters[1], 0);
}

TEST(SkippingLogic, WhileSkip)
{
  BehaviorTreeFactory factory;
  std::array<int, 2> counters;
  RegisterTestTick(factory, "Test", counters);

  const std::string xml_text_noskip = R"(
    <root BTCPP_format="4" >
       <BehaviorTree>
          <Sequence>
            <Script code=" doit:=true "/>
            <Sequence>
              <TestA _while="doit"/>
            </Sequence>
          </Sequence>
       </BehaviorTree>
    </root>)";

  const std::string xml_text_skip = R"(
    <root BTCPP_format="4" >
       <BehaviorTree>
          <Sequence>
            <Script code=" doit:=false "/>
            <Sequence>
              <TestB _while="doit"/>
            </Sequence>
          </Sequence>
       </BehaviorTree>
    </root>)";

  for(auto const* xml_text : { &xml_text_noskip, &xml_text_skip })
  {
    auto tree = factory.createTreeFromText(*xml_text);
    NodeStatus status = tree.tickWhileRunning();
    ASSERT_EQ(status, NodeStatus::SUCCESS);
  }
  // counters[0] contains the number of times TestA was ticked
  ASSERT_EQ(counters[0], 1);

  // counters[1] contains the number of times TestB was ticked
  ASSERT_EQ(counters[1], 0);
}
