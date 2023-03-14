#include <gtest/gtest.h>
#include <string>
#include "behaviortree_cpp/basic_types.h"
#include "behaviortree_cpp/bt_factory.h"
#include "test_helper.hpp"
#include "../sample_nodes/dummy_nodes.h"

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

enum DeviceType { BATT=1, CONTROLLER=2 };
BT::NodeStatus checkLevel2(BT::TreeNode &self)
{
  double percent = self.getInput<double>("percentage").value();
  DeviceType devType;
  auto res = self.getInput("deviceType", devType);
  if(!res) {
    throw std::runtime_error(res.error());
  }

  if(devType == DeviceType::BATT)
  {
    self.setOutput("isLowBattery", (percent < 25));
  }
  std::cout << "Device: " << devType << " Level: " << percent << std::endl;
  return BT::NodeStatus::SUCCESS;
}

TEST(SkippingLogic, RepeatedSkippingReactiveSequence)
{
  BehaviorTreeFactory factory;
  std::array<int, 2> counters;
  RegisterTestTick(factory, "Test", counters);

  //! setting the battery level = 50
  const std::string xml_text = R"(

    <root BTCPP_format="4" >
       <BehaviorTree ID="PowerManagerT">
          <ReactiveSequence>
             <Script code=" LOW_BATT:=50 "/>
             <CheckLevel deviceType="BATT"
                         percentage="{LOW_BATT}"
                         isLowBattery="{isLowBattery}"/>
             <TestA _skipIf="!isLowBattery"/>
             <TestB/>
         </ReactiveSequence>
       </BehaviorTree>
    </root>)";

  factory.registerNodeType<DummyNodes::SaySomething>("SaySomething");
  factory.registerSimpleCondition("CheckLevel", std::bind(checkLevel2, std::placeholders::_1),
                                  { BT::InputPort("percentage"),
                                   BT::InputPort("deviceType"),
                                   BT::OutputPort("isLowBattery")});
  factory.registerScriptingEnums<DeviceType>();

  auto tree = factory.createTreeFromText(xml_text);

  BT::NodeStatus status;
  try {
    int runs = 0;
    while(runs < 5)
    {
      status = tree.tickOnce();
      tree.sleep(std::chrono::milliseconds(10));
      runs++;
    }
  } catch (BT::LogicError err) {
    std::cout << err.what() << std::endl;
  }

  ASSERT_EQ(status, NodeStatus::SUCCESS);
  ASSERT_EQ(counters[0], 0);
  ASSERT_EQ(counters[1], 5);
}

TEST(SkippingLogic, RepeatedNoSkippingReactiveSequence)
{
  BehaviorTreeFactory factory;
  std::array<int, 2> counters;
  RegisterTestTick(factory, "Test", counters);

  //! setting the battery level = 10
  const std::string xml_text = R"(

    <root BTCPP_format="4" >
       <BehaviorTree ID="PowerManagerT">
          <ReactiveSequence>
             <Script code=" LOW_BATT:=10 "/>
             <CheckLevel deviceType="BATT"
                         percentage="{LOW_BATT}"
                         isLowBattery="{isLowBattery}"/>
             <TestA _skipIf="!isLowBattery"/>
             <TestB/>
         </ReactiveSequence>
       </BehaviorTree>
    </root>)";

  factory.registerNodeType<DummyNodes::SaySomething>("SaySomething");
  factory.registerSimpleCondition("CheckLevel", std::bind(checkLevel2, std::placeholders::_1),
                                  { BT::InputPort("percentage"),
                                   BT::InputPort("deviceType"),
                                   BT::OutputPort("isLowBattery")});
  factory.registerScriptingEnums<DeviceType>();

  auto tree = factory.createTreeFromText(xml_text);

  BT::NodeStatus status;
  try {
    int runs = 0;
    while(runs < 5)
    {
      status = tree.tickOnce();
      tree.sleep(std::chrono::milliseconds(10));
      runs++;
    }
  } catch (BT::LogicError err) {
    std::cout << err.what() << std::endl;
  }

  ASSERT_EQ(status, NodeStatus::SUCCESS);
  ASSERT_EQ(counters[0], 5);
  ASSERT_EQ(counters[1], 5);
}
