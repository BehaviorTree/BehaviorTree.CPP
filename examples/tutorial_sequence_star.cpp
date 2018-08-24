#include "behavior_tree_core/xml_parsing.h"
#include "behavior_tree_logger/bt_cout_logger.h"
#include "behavior_tree_logger/bt_file_logger.h"

#include "dummy_nodes.h"
#include "movebase_node.h"

using namespace BT;


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
    factory.registerSimpleCondition("TemperatureOK", std::bind( CheckBattery ));
    factory.registerSimpleCondition("BatteryOK", std::bind( CheckTemperature ));
    factory.registerNodeType<MoveBaseAction>("MoveBase");

    // Loog at the state transitions and messages using either
    // xml_text_sequence and xml_text_sequence_star

    // The main difference that you should notice is that the
    // actions BatteryOK and TempearaturOK are executed at each tick()
    // if Sequence is used, but only once if SequenceStar is used.

    for(auto& xml_text: {xml_text_sequence, xml_text_sequence_star})
    {
        std::cout << "\n------------ BUILDING A NEW TREE ------------\n\n" << std::endl;

        auto tree = buildTreeFromText(factory, xml_text);
        TreeNode::Ptr root_node = tree.first;

        // This logger will show all the state transitions on console
        StdCoutLogger logger_cout(root_node.get());

        // This other logger will save the state transition in a custom file format
        // simple_trace.fbl can be visualized using the command line tool [bt_log_cat]
        FileLogger file_file(root_node.get(), "simple_trace.fbl", 32);

        NodeStatus status;

        std::cout << "\n------- First executeTick() --------" << std::endl;
        status = root_node->executeTick();
        Assert( status == NodeStatus::RUNNING);

        std::cout << "\n------- sleep --------" << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        std::cout << "\n------- Second executeTick() --------" << std::endl;
        status = root_node->executeTick();
        Assert( status == NodeStatus::RUNNING);

        std::cout << "\n------- sleep --------" << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        std::cout << "\n------- Third executeTick() --------" << std::endl;
        status = root_node->executeTick();
        Assert( status == NodeStatus::SUCCESS);

        std::cout << std::endl;
    }
    return 0;
}
