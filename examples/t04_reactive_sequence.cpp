#include "behaviortree_cpp_v3/bt_factory.h"

#include "dummy_nodes.h"
#include "movebase_node.h"

using namespace BT;

/** This tutorial will teach you:
 *
 *  - The difference between Sequence and ReactiveSequence
 *
 *  - How to create an asynchronous ActionNode (an action that is execute in
 *    its own thread).
*/

// clang-format off

static const char* xml_text_sequence = R"(

 <root main_tree_to_execute = "MainTree" >

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

 <root main_tree_to_execute = "MainTree" >

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

void Assert(bool condition)
{
  if (!condition)
    throw RuntimeError("this is not what I expected");
}

int main()
{
  using namespace DummyNodes;
  using std::chrono::milliseconds;

  BehaviorTreeFactory factory;
  factory.registerSimpleCondition("BatteryOK", std::bind(CheckBattery));
  factory.registerNodeType<MoveBaseAction>("MoveBase");
  factory.registerNodeType<SaySomething>("SaySomething");

  // Compare the state transitions and messages using either
  // xml_text_sequence and xml_text_sequence_star

  // The main difference that you should notice is:
  //  1) When Sequence is used, BatteryOK is executed at __each__ tick()
  //  2) When SequenceStar is used, those ConditionNodes are executed only __once__.

  for (auto& xml_text : {xml_text_sequence, xml_text_reactive})
  {
    std::cout << "\n------------ BUILDING A NEW TREE ------------" << std::endl;

    auto tree = factory.createTreeFromText(xml_text);

    // Here, instead of tree.tickRootWhileRunning(),
    // we prefer our own loop.
    std::cout << "--- ticking\n";
    NodeStatus status = tree.tickRoot();
    std::cout << "--- status: " << toStr(status) << "\n\n";

    while(status == NodeStatus::RUNNING)
    {
      // Sleep to avoid busy loops.
      // do NOT use other sleep functions!
      // Small sleep time is OK, here we use a large one only to
      // have less messages on the console.
      tree.sleep(std::chrono::milliseconds(100));

      std::cout << "--- ticking\n";
      status = tree.tickRoot();
      std::cout << "--- status: " << toStr(status) << "\n\n";
    }
  }
  return 0;
}

/*
* Expected output:

------------ BUILDING A NEW TREE ------------
--- ticking
[ Battery: OK ]
Robot says: mission started...
--- status: RUNNING

[ MoveBase: STARTED ]. goal: x=1 y=2.0 theta=3.00
--- ticking
--- status: RUNNING

--- ticking
--- status: RUNNING

[ MoveBase: FINISHED ]
--- ticking
Robot says: mission completed!
--- status: SUCCESS


------------ BUILDING A NEW TREE ------------
--- ticking
[ Battery: OK ]
Robot says: mission started...
--- status: RUNNING

[ MoveBase: STARTED ]. goal: x=1 y=2.0 theta=3.00
--- ticking
[ Battery: OK ]
--- status: RUNNING

--- ticking
[ Battery: OK ]
--- status: RUNNING

[ MoveBase: FINISHED ]
--- ticking
[ Battery: OK ]
Robot says: mission completed!
--- status: SUCCESS
*/
