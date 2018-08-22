#include "behavior_tree_core/xml_parsing.h"
#include "behavior_tree_logger/bt_cout_logger.h"
#include "behavior_tree_logger/bt_file_logger.h"

using namespace BT;


NodeStatus checkBattery()
{
    std::cout << "[ Battery: OK ]" << std::endl;
    return NodeStatus::SUCCESS;
}

NodeStatus CheckTemperature()
{
    std::cout << "[ Temperature: OK ]" << std::endl;
    return NodeStatus::SUCCESS;
}

// This is an asynchronous operation that will run in a separate thread
class MoveAction: public ActionNode
{
public:
    MoveAction(const std::string& name): ActionNode(name) {}
    NodeStatus tick() override
    {
        std::cout << "[ Move: started ]" << std::endl;
        std::this_thread::sleep_for( std::chrono::milliseconds(80) );
        std::cout << "[ Move: finished ]" << std::endl;
        return NodeStatus::SUCCESS;
    }
};

// clang-format off

const std::string xml_text_sequence = R"(

 <root main_tree_to_execute = "MainTree" >

     <BehaviorTree ID="MainTree">
        <Sequence name="root">
            <BatteryOK/>
            <TemperatureOK />
            <Move/>
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
             <Move/>
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
    factory.registerSimpleCondition("TemperatureOK", std::bind( checkBattery ));
    factory.registerSimpleCondition("BatteryOK", std::bind( CheckTemperature ));
    factory.registerNodeType<MoveAction>("Move");

    // Loog at the state transitions and messages using either
    // xml_text_sequence and xml_text_sequence_star

    // The main difference that you should notice is that the
    // actions BatteryOK and TempearaturOK are executed at each tick()
    // is Sequence is used and only once if SequenceStar is used.

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
        std::this_thread::sleep_for(std::chrono::milliseconds(50));

        std::cout << "\n------- Second executeTick() --------" << std::endl;
        status = root_node->executeTick();
        Assert( status == NodeStatus::RUNNING);

        std::cout << "\n------- sleep --------" << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(50));

        std::cout << "\n------- Third executeTick() --------" << std::endl;
        status = root_node->executeTick();
        Assert( status == NodeStatus::SUCCESS);

        std::cout << std::endl;

        // The main difference that you should notice is that the
        // actions BatteryOK and TempearaturOK are executed at each tick()
        // is Sequence is used and only once if SequenceStar is used.
    }
    return 0;
}
