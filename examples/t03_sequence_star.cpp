#include "behavior_tree_core/xml_parsing.h"
#include "behavior_tree_logger/bt_cout_logger.h"
#include "behavior_tree_logger/bt_file_logger.h"

#include "dummy_nodes.h"
#include "movebase_node.h"

using namespace BT;

/** This tutorial will tech you:
 *
 *  - The difference between Sequence and SequenceStar
 *  - How to create and use a TreeNode that accepts NodeParameters
 *  - How to create an asynchronous ActionNode (an action that is execute in
 *    its own thread).
*/


// clang-format off

const std::string xml_text_sequence = R"(

 <root main_tree_to_execute = "MainTree" >

     <BehaviorTree ID="MainTree">
        <Sequence name="root">
            <BatteryOK/>
            <TemperatureOK />
            <MoveBase goal="1;2;3"/>
        </Sequence>
     </BehaviorTree>

 </root>
 )";

const std::string xml_text_sequence_star = R"(

 <root main_tree_to_execute = "MainTree" >

     <BehaviorTree ID="MainTree">
        <SequenceStar name="root">
             <BatteryOK/>
             <TemperatureOK />
             <MoveBase goal="1;2;3"/>
        </SequenceStar>
     </BehaviorTree>

 </root>
 )";

// clang-format on

void Assert(bool condition)
{
    if( !condition ) throw std::runtime_error("this is not what I expected");
}

int main()
{
    BehaviorTreeFactory factory;
    factory.registerSimpleCondition("TemperatureOK", std::bind( DummyNodes::CheckBattery ));
    factory.registerSimpleCondition("BatteryOK", std::bind( DummyNodes::CheckTemperature ));
    factory.registerNodeType<MoveBaseAction>("MoveBase");

    // Compare the state transitions and messages using either
    // xml_text_sequence and xml_text_sequence_star

    // The main difference that you should notice is:
    //  1) When Sequence is used, BatteryOK and TempearaturOK is executed at each tick()
    //  2) When SequenceStar is used, those ConditionNodes are executed only once.

    for(auto& xml_text: {xml_text_sequence, xml_text_sequence_star})
    {
        std::cout << "\n------------ BUILDING A NEW TREE ------------" << std::endl;

        auto tree = buildTreeFromText(factory, xml_text);

        // This logger will show all the state transitions on console
        StdCoutLogger logger_cout(tree.root_node);
        logger_cout.enableTransitionToIdle(false);// make the log less verbose
        logger_cout.seTimestampType( TimestampType::RELATIVE );

        // FileLogger will save the state transitions in a custom file format
        // simple_trace.fbl, that can be visualized using the command line tool [bt_log_cat]
        FileLogger logger_file(tree.root_node, "simple_trace.fbl");

        NodeStatus status;

        std::cout << "\n--- 1st executeTick() ---" << std::endl;
        status = tree.root_node->executeTick();
        Assert( status == NodeStatus::RUNNING);

        SleepMS(150);
        std::cout << "\n--- 2nd executeTick() ---" << std::endl;
        status = tree.root_node->executeTick();
        Assert( status == NodeStatus::RUNNING);

        SleepMS(150);
        std::cout << "\n--- 3rd executeTick() ---" << std::endl;
        status = tree.root_node->executeTick();
        Assert( status == NodeStatus::SUCCESS);

        std::cout << std::endl;
    }
    return 0;
}

/*
 Expected output:

------------ BUILDING A NEW TREE ------------

--- 1st executeTick() ---
[0.000]: root                      IDLE -> RUNNING
[ Temperature: OK ]
[0.000]: BatteryOK                 IDLE -> SUCCESS
[ Battery: OK ]
[0.000]: TemperatureOK             IDLE -> SUCCESS
[0.000]: MoveBase                  IDLE -> RUNNING
[ MoveBase: STARTED ]. goal: x=1 y=2.0 theta=3.00

--- 2nd executeTick() ---
[ Temperature: OK ]
[ Battery: OK ]
[ MoveBase: FINISHED ]
[0.253]: MoveBase                  RUNNING -> SUCCESS

--- 3rd executeTick() ---
[ Temperature: OK ]
[ Battery: OK ]
[0.301]: root                      RUNNING -> SUCCESS


------------ BUILDING A NEW TREE ------------

--- 1st executeTick() ---
[0.000]: root                      IDLE -> RUNNING
[ Temperature: OK ]
[0.000]: BatteryOK                 IDLE -> SUCCESS
[ Battery: OK ]
[0.000]: TemperatureOK             IDLE -> SUCCESS
[0.000]: MoveBase                  IDLE -> RUNNING
[ MoveBase: STARTED ]. goal: x=1 y=2.0 theta=3.00

--- 2nd executeTick() ---
[ MoveBase: FINISHED ]
[0.254]: MoveBase                  RUNNING -> SUCCESS

--- 3rd executeTick() ---
[0.301]: root                      RUNNING -> SUCCESS

*/

