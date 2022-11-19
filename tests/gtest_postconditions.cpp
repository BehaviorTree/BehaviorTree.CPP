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
