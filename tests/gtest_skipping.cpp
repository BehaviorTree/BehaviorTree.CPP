#include <gtest/gtest.h>
#include <string>
#include "behaviortree_cpp/basic_types.h"
#include "behaviortree_cpp/bt_factory.h"
#include "test_helper.hpp"

using namespace BT;

TEST(SkippingLogic, Sequence)
{
  BehaviorTreeFactory factory;
  std::array<int, 2> counters;
  RegisterTestTick(factory, "Test", counters);

  const std::string xml_text = R"(

    <root main_tree_to_execute = "MainTree" >
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

    <root main_tree_to_execute = "MainTree" >
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
