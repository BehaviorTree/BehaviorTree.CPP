#include "behaviortree_cpp/bt_factory.h"

#include "dummy_nodes.h"
#include "movebase_node.h"

using namespace BT;

/** This tutorial will teach you:
 *
 *  - The difference between Sequence and ReactiveSequence
 *
 *  - How to create an asynchronous ActionNode.
*/

// clang-format off

static const char* xml_text_sequence = R"(

 <root BTCPP_format="4" >

     <BehaviorTree ID="MainTree">
        <Sequence name="root">
            <BatteryOK/>
            <SaySomething   message="mission started..." />
            <MoveBase       goal="1;2;3"/>
            <SaySomething   message="mission completed!" />
        </Sequence>
     </BehaviorTree>

 </root>
 )";

static const char* xml_text_reactive = R"(

 <root BTCPP_format="4" >

     <BehaviorTree ID="MainTree">
        <ReactiveSequence name="root">
            <BatteryOK/>
            <Sequence>
                <SaySomething   message="mission started..." />
                <MoveBase       goal="1;2;3"/>
                <SaySomething   message="mission completed!" />
            </Sequence>
        </ReactiveSequence>
     </BehaviorTree>

 </root>
 )";

// clang-format on

using namespace DummyNodes;

int main()
{
  BehaviorTreeFactory factory;

  factory.registerSimpleCondition("BatteryOK", std::bind(CheckBattery));
  factory.registerNodeType<MoveBaseAction>("MoveBase");
  factory.registerNodeType<SaySomething>("SaySomething");

  // Compare the state transitions and messages using either
  // xml_text_sequence and xml_text_reactive.

  // The main difference that you should notice is:
  //  1) When Sequence is used, the ConditionNode is executed only __once__ because it returns SUCCESS.
  //  2) When ReactiveSequence is used, BatteryOK is executed at __each__ tick()

  for(auto& xml_text : { xml_text_sequence, xml_text_reactive })
  {
    std::cout << "\n------------ BUILDING A NEW TREE ------------\n\n";

    auto tree = factory.createTreeFromText(xml_text);

    NodeStatus status = NodeStatus::IDLE;
#if 0
    // Tick the root until we receive either SUCCESS or RUNNING
    // same as: tree.tickRoot(Tree::WHILE_RUNNING)
    std::cout << "--- ticking\n";
    status = tree.tickWhileRunning();
    std::cout << "--- status: " << toStr(status) << "\n\n";
#else
    // If we need to run code between one tick() and the next,
    // we can implement our own while loop
    while(status != NodeStatus::SUCCESS)
    {
      std::cout << "--- ticking\n";
      status = tree.tickOnce();
      std::cout << "--- status: " << toStr(status) << "\n\n";

      // if still running, add some wait time
      if(status == NodeStatus::RUNNING)
      {
        tree.sleep(std::chrono::milliseconds(100));
      }
    }
#endif
  }
  return 0;
}
