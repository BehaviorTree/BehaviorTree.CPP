#include <gtest/gtest.h>
#include <string>
#include "behaviortree_cpp_v3/basic_types.h"
#include "behaviortree_cpp_v3/bt_factory.h"

using namespace BT;

TEST(BlackboardPreconditionTest, IntEquals)
{
  BehaviorTreeFactory factory;

  const std::string xml_text = R"(

    <root main_tree_to_execute = "MainTree" >
        <BehaviorTree ID="MainTree">
            <Sequence>
                <SetBlackboard output_key="a" value="1" />
                <SetBlackboard output_key="b" value="1" />
                
                <BlackboardCheckInt value_A="{a}" value_B="{b}" return_on_mismatch="SUCCESS">
                    <AlwaysFailure />
                </BlackboardCheckInt>
            </Sequence>
        </BehaviorTree>
    </root>)";

  auto tree = factory.createTreeFromText(xml_text);
  const auto status = tree.tickRoot();
  ASSERT_EQ(status, NodeStatus::FAILURE);
}

TEST(BlackboardPreconditionTest, IntDoesNotEqual)
{
  BehaviorTreeFactory factory;

  const std::string xml_text = R"(

    <root main_tree_to_execute = "MainTree" >
        <BehaviorTree ID="MainTree">
            <Sequence>
                <SetBlackboard output_key="a" value="1" />
                <SetBlackboard output_key="b" value="2" />
                
                <BlackboardCheckInt value_A="{a}" value_B="{b}" return_on_mismatch="SUCCESS">
                    <AlwaysFailure />
                </BlackboardCheckInt>
            </Sequence>
        </BehaviorTree>
    </root>)";

  auto tree = factory.createTreeFromText(xml_text);
  const auto status = tree.tickRoot();
  ASSERT_EQ(status, NodeStatus::SUCCESS);
}

TEST(BlackboardPreconditionTest, DoubleEquals)
{
  BehaviorTreeFactory factory;

  const std::string xml_text = R"(

    <root main_tree_to_execute = "MainTree" >
        <BehaviorTree ID="MainTree">
            <Sequence>
                <SetBlackboard output_key="a" value="1.1" />
                <SetBlackboard output_key="b" value="1.1" />
                
                <BlackboardCheckDouble value_A="{a}" value_B="{b}" return_on_mismatch="SUCCESS">
                    <AlwaysFailure />
                </BlackboardCheckDouble>
            </Sequence>
        </BehaviorTree>
    </root>)";

  auto tree = factory.createTreeFromText(xml_text);
  const auto status = tree.tickRoot();
  ASSERT_EQ(status, NodeStatus::FAILURE);
}

TEST(BlackboardPreconditionTest, DoubleDoesNotEqual)
{
  BehaviorTreeFactory factory;

  const std::string xml_text = R"(

    <root main_tree_to_execute = "MainTree" >
        <BehaviorTree ID="MainTree">
            <Sequence>
                <SetBlackboard output_key="a" value="1.1" />
                <SetBlackboard output_key="b" value="2.1" />
                
                <BlackboardCheckDouble value_A="{a}" value_B="{b}" return_on_mismatch="SUCCESS">
                    <AlwaysFailure />
                </BlackboardCheckDouble>
            </Sequence>
        </BehaviorTree>
    </root>)";

  auto tree = factory.createTreeFromText(xml_text);
  const auto status = tree.tickRoot();
  ASSERT_EQ(status, NodeStatus::SUCCESS);
}

TEST(BlackboardPreconditionTest, StringEquals)
{
  BehaviorTreeFactory factory;

  const std::string xml_text = R"(

    <root main_tree_to_execute = "MainTree" >
        <BehaviorTree ID="MainTree">
            <Sequence>
                <SetBlackboard output_key="a" value="foo" />
                <SetBlackboard output_key="b" value="foo" />
                
                <BlackboardCheckString value_A="{a}" value_B="{b}" return_on_mismatch="SUCCESS">
                    <AlwaysFailure />
                </BlackboardCheckString>
            </Sequence>
        </BehaviorTree>
    </root>)";

  auto tree = factory.createTreeFromText(xml_text);
  const auto status = tree.tickRoot();
  ASSERT_EQ(status, NodeStatus::FAILURE);
}

TEST(BlackboardPreconditionTest, StringDoesNotEqual)
{
  BehaviorTreeFactory factory;

  const std::string xml_text = R"(

    <root main_tree_to_execute = "MainTree" >
        <BehaviorTree ID="MainTree">
            <Sequence>
                <SetBlackboard output_key="a" value="foo" />
                <SetBlackboard output_key="b" value="bar" />
                
                <BlackboardCheckString value_A="{a}" value_B="{b}" return_on_mismatch="SUCCESS">
                    <AlwaysFailure />
                </BlackboardCheckString>
            </Sequence>
        </BehaviorTree>
    </root>)";

  auto tree = factory.createTreeFromText(xml_text);
  const auto status = tree.tickRoot();
  ASSERT_EQ(status, NodeStatus::SUCCESS);
}

TEST(BlackboardPreconditionTest, BoolEquals)
{
  BehaviorTreeFactory factory;

  const std::string xml_text = R"(

    <root main_tree_to_execute = "MainTree" >
        <BehaviorTree ID="MainTree">
            <Sequence>
                <SetBlackboard output_key="a" value="true" />
                <SetBlackboard output_key="b" value="true" />
                
                <BlackboardCheckBool value_A="{a}" value_B="{b}" return_on_mismatch="SUCCESS">
                    <AlwaysFailure />
                </BlackboardCheckBool>
            </Sequence>
        </BehaviorTree>
    </root>)";

  auto tree = factory.createTreeFromText(xml_text);
  const auto status = tree.tickRoot();
  ASSERT_EQ(status, NodeStatus::FAILURE);
}

TEST(BlackboardPreconditionTest, BoolDoesNotEqual)
{
  BehaviorTreeFactory factory;

  const std::string xml_text = R"(

    <root main_tree_to_execute = "MainTree" >
        <BehaviorTree ID="MainTree">
            <Sequence>
                <SetBlackboard output_key="a" value="true" />
                <SetBlackboard output_key="b" value="false" />
                
                <BlackboardCheckBool value_A="{a}" value_B="{b}" return_on_mismatch="SUCCESS">
                    <AlwaysFailure />
                </BlackboardCheckBool>
            </Sequence>
        </BehaviorTree>
    </root>)";

  auto tree = factory.createTreeFromText(xml_text);
  const auto status = tree.tickRoot();
  ASSERT_EQ(status, NodeStatus::SUCCESS);
}
