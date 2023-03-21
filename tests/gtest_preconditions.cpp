#include <gtest/gtest.h>
#include <string>
#include "behaviortree_cpp/basic_types.h"
#include "behaviortree_cpp/bt_factory.h"
#include "test_helper.hpp"

using namespace BT;

TEST(PreconditionsDecorator, Integers)
{
  BehaviorTreeFactory factory;
  std::array<int, 3> counters;
  RegisterTestTick(factory, "Test", counters);

  const std::string xml_text = R"(

    <root BTCPP_format="4" >
        <BehaviorTree ID="MainTree">
            <Sequence>
                <Script code = "A:=1; B:=1; C:=3" />
                <Precondition if="A==B" else="FAILURE">
                    <TestA/>
                </Precondition>
                <Precondition if="A==C" else="SUCCESS">
                    <TestB/>
                </Precondition>
                <Precondition if="A!=C" else="FAILURE">
                    <TestC/>
                </Precondition>
            </Sequence>
        </BehaviorTree>
    </root>)";

  auto tree = factory.createTreeFromText(xml_text);
  const auto status = tree.tickWhileRunning();
  ASSERT_EQ(status, NodeStatus::SUCCESS);
  ASSERT_EQ(counters[0], 1);
  ASSERT_EQ(counters[1], 0);
  ASSERT_EQ(counters[2], 1);
}

TEST(PreconditionsDecorator, DoubleEquals)
{
  BehaviorTreeFactory factory;
  std::array<int, 3> counters;
  RegisterTestTick(factory, "Test", counters);

  const std::string xml_text = R"(

    <root BTCPP_format="4" >
        <BehaviorTree ID="MainTree">
            <Sequence>
                <Script code = " A:=1.1; B:=(1.0+0.1); C:= 2.0 " />

                <Precondition if="A==B" else="FAILURE">
                    <TestA/>
                </Precondition>

                <Precondition if="A==C" else="SUCCESS">
                    <TestB/>
                </Precondition>

                <Precondition if="A!=C" else="FAILURE">
                    <TestC/>
                </Precondition>
            </Sequence>
        </BehaviorTree>
    </root>)";

  auto tree = factory.createTreeFromText(xml_text);
  const auto status = tree.tickWhileRunning();
  ASSERT_EQ(status, NodeStatus::SUCCESS);
  ASSERT_EQ(counters[0], 1);
  ASSERT_EQ(counters[1], 0);
  ASSERT_EQ(counters[2], 1);
}

TEST(PreconditionsDecorator, StringEquals)
{
  BehaviorTreeFactory factory;
  std::array<int, 2> counters;
  RegisterTestTick(factory, "Test", counters);

  const std::string xml_text = R"(

    <root BTCPP_format="4" >
        <BehaviorTree ID="MainTree">
            <Sequence>
                <Script code = "A:='hello'" />
                <Script code = "B:='world'" />
                <Script code = "C:='world'" />

                <Precondition if=" A==B " else="SUCCESS">
                    <TestA/>
                </Precondition>
                <Precondition if=" B==C " else="FAILURE">
                    <TestB/>
                </Precondition>
            </Sequence>
        </BehaviorTree>
    </root>)";

  auto tree = factory.createTreeFromText(xml_text);
  const auto status = tree.tickWhileRunning();
  ASSERT_EQ(status, NodeStatus::SUCCESS);
  ASSERT_EQ(counters[0], 0);
  ASSERT_EQ(counters[1], 1);
}

TEST(Preconditions, Basic)
{
  BehaviorTreeFactory factory;
  std::array<int, 4> counters;
  RegisterTestTick(factory, "Test", counters);

  const std::string xml_text = R"(

    <root BTCPP_format="4" >
        <BehaviorTree ID="MainTree">
            <Sequence>
                <Script code = "A:=1" />
                <TestA _successIf= "A==1"/>
                <TestB _successIf= "A==2"/>
                <Fallback>
                    <TestC _failureIf= "A==1"/>
                    <TestD _failureIf= "A!=1"/>
                </Fallback>
            </Sequence>
        </BehaviorTree>
    </root>)";

  auto tree = factory.createTreeFromText(xml_text);
  const auto status = tree.tickWhileRunning();
  ASSERT_EQ(status, NodeStatus::SUCCESS);
  ASSERT_EQ(counters[0], 0);   // skipped
  ASSERT_EQ(counters[1], 1);   // executed
  ASSERT_EQ(counters[2], 0);   // skipped
  ASSERT_EQ(counters[3], 1);   // executed
}


TEST(Preconditions, Issue533)
{
  BehaviorTreeFactory factory;
  std::array<int, 3> counters;
  RegisterTestTick(factory, "Test", counters);

  const std::string xml_text = R"(

    <root BTCPP_format="4" >
        <BehaviorTree ID="MainTree">
            <Sequence>
                <TestA _skipIf="A!=1" />
                <TestB _skipIf="A!=2" _onSuccess="A=1"/>
                <TestC _skipIf="A!=3" _onSuccess="A=2"/>
            </Sequence>
        </BehaviorTree>
    </root>)";

  auto tree = factory.createTreeFromText(xml_text);
  tree.subtrees.front()->blackboard->set("A", 3);

  tree.tickOnce();
  ASSERT_EQ(counters[0], 0);
  ASSERT_EQ(counters[1], 0);
  ASSERT_EQ(counters[2], 1);

  tree.tickOnce();
  ASSERT_EQ(counters[0], 0);
  ASSERT_EQ(counters[1], 1);
  ASSERT_EQ(counters[2], 1);

  tree.tickOnce();
  ASSERT_EQ(counters[0], 1);
  ASSERT_EQ(counters[1], 1);
  ASSERT_EQ(counters[2], 1);
}
