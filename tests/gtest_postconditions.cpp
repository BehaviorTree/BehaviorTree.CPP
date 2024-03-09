#include <gtest/gtest.h>
#include <string>
#include "behaviortree_cpp/basic_types.h"
#include "behaviortree_cpp/bt_factory.h"

using namespace BT;

TEST(PostConditions, BasicTest)
{
  BehaviorTreeFactory factory;

  const std::string xml_text = R"(

    <root BTCPP_format="4" >
        <BehaviorTree ID="MainTree">
            <Sequence>
                <Script code = "A:=1; B:=1; C:=1; D:=1" />

                <AlwaysSuccess _onSuccess="B=42"/>

                <ForceSuccess>
                    <AlwaysSuccess _failureIf="A!=0" _onFailure="C=42"/>
                </ForceSuccess>

                <ForceSuccess>
                    <AlwaysFailure _onFailure="D=42"/>
                </ForceSuccess>
            </Sequence>
        </BehaviorTree>
    </root>)";

  auto tree = factory.createTreeFromText(xml_text);
  const auto status = tree.tickWhileRunning();
  ASSERT_EQ(status, NodeStatus::SUCCESS);

  ASSERT_EQ(tree.rootBlackboard()->get<int>("B"), 42);
  ASSERT_EQ(tree.rootBlackboard()->get<int>("C"), 42);
  ASSERT_EQ(tree.rootBlackboard()->get<int>("D"), 42);
}

TEST(PostConditions, Issue539)
{
  const std::string xml_text = R"(
    <root BTCPP_format="4" >
      <BehaviorTree ID="MainTree">
        <Sequence>
          <Script code = "x:=0; y:=0" />
          <RetryUntilSuccessful num_attempts="5">
            <AlwaysFailure _onFailure="x  += 1"  _post="y  += 1" />
          </RetryUntilSuccessful>
        </Sequence>
      </BehaviorTree>
    </root>)";

  BehaviorTreeFactory factory;
  auto tree = factory.createTreeFromText(xml_text);
  const auto status = tree.tickWhileRunning();
  ASSERT_EQ(status, NodeStatus::FAILURE);

  ASSERT_EQ(tree.rootBlackboard()->get<int>("x"), 5);
  ASSERT_EQ(tree.rootBlackboard()->get<int>("y"), 5);
}

TEST(PostConditions, Issue601)
{
  const std::string xml_text = R"(
  <root BTCPP_format="4" >
    <BehaviorTree ID="test_tree">
      <Sequence>
        <Script code="test := 'start'"/>
          <Parallel failure_count="1"
                    success_count="-1">
            <Sleep msec="1000"
                   _onHalted="test = 'halted'"
                   _post="test = 'post'"/>
            <AlwaysFailure/>
          </Parallel>
      </Sequence>
    </BehaviorTree>
  </root>)";

  BehaviorTreeFactory factory;
  auto tree = factory.createTreeFromText(xml_text);
  const auto status = tree.tickWhileRunning();

  ASSERT_EQ(tree.rootBlackboard()->get<std::string>("test"), "halted");
}
