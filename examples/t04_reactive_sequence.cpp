#include "behaviortree_cpp_v3/bt_factory.h"

#include "dummy_nodes.h"
#include "movebase_node.h"

using namespace BT;

/** This tutorial will teach you:
 *
 *  - The difference between Sequence and SequenceStar
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

        NodeStatus status;

        std::cout << "\n--- 1st executeTick() ---" << std::endl;
        status = tree.tickRoot();
        Assert(status == NodeStatus::RUNNING);

        SleepMS(150);
        std::cout << "\n--- 2nd executeTick() ---" << std::endl;
        status = tree.tickRoot();
        Assert(status == NodeStatus::RUNNING);

        SleepMS(150);
        std::cout << "\n--- 3rd executeTick() ---" << std::endl;
        status = tree.tickRoot();
        Assert(status == NodeStatus::SUCCESS);

        std::cout << std::endl;
    }
    return 0;
}

/*
 Expected output:

------------ BUILDING A NEW TREE ------------

--- 1st executeTick() ---
[ Battery: OK ]
Robot says: "mission started..."
[ MoveBase: STARTED ]. goal: x=1 y=2.0 theta=3.00

--- 2nd executeTick() ---
[ Battery: OK ]
[ MoveBase: FINISHED ]

--- 3rd executeTick() ---
[ Battery: OK ]
Robot says: "mission completed!"


------------ BUILDING A NEW TREE ------------

--- 1st executeTick() ---
[ Battery: OK ]
Robot says: "mission started..."
[ MoveBase: STARTED ]. goal: x=1 y=2.0 theta=3.00

--- 2nd executeTick() ---
[ MoveBase: FINISHED ]

--- 3rd executeTick() ---
Robot says: "mission completed!"

*/
