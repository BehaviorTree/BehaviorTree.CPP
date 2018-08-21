#include "behavior_tree_core/xml_parsing.h"
#include "behavior_tree_logger/bt_cout_logger.h"
#include "behavior_tree_logger/bt_file_logger.h"

// clang-format off

class BatteryCondition: public BT::ConditionNode
{
public:
    BatteryCondition(const std::string& name): BT::ConditionNode(name) {}
    BT::NodeStatus tick() override {
        std::cout << "[ Battery: OK ]" << std::endl;
        return BT::NodeStatus::SUCCESS; }
};

class TemperatureCondition: public BT::ConditionNode
{
public:
    TemperatureCondition(const std::string& name): BT::ConditionNode(name) {}
    BT::NodeStatus tick() override {
        std::cout << "[ Temperature: OK ]" << std::endl;
        return BT::NodeStatus::SUCCESS; }
};

class MoveAction: public BT::ActionNode
{
public:
    MoveAction(const std::string& name): BT::ActionNode(name) {}
    BT::NodeStatus tick() override {
        std::cout << "[ Move: started ]" << std::endl;
        std::this_thread::sleep_for( std::chrono::milliseconds(80) );
        std::cout << "[ Move: finished ]" << std::endl;
        return BT::NodeStatus::SUCCESS;
    }
};


const std::string xml_text_A = R"(

 <root main_tree_to_execute = "MainTree" >

     <BehaviorTree ID="MainTree">
        <Sequence name="root">
             <Sequence name="sequence_conditions">
                 <Condition ID="BatteryOK" />
                 <Condition ID="TemperatureOK" />
             </Sequence>
             <Action ID="Move" />
        </Sequence>
     </BehaviorTree>

 </root>
 )";

const std::string xml_text_B = R"(

 <root main_tree_to_execute = "MainTree" >

     <BehaviorTree ID="MainTree">
        <Sequence name="root">
             <Condition ID="BatteryOK" />
             <Condition ID="TemperatureOK" />
             <Action ID="Move" />
        </Sequence>
     </BehaviorTree>

 </root>
 )";

// clang-format on

int main(int argc, char** argv)
{
    using namespace BT;

    BT::BehaviorTreeFactory factory;
    factory.registerNodeType<TemperatureCondition>("TemperatureOK");
    factory.registerNodeType<BatteryCondition>("BatteryOK");
    factory.registerNodeType<MoveAction>("Move");

    BT::XMLParser parser(factory);
    parser.loadFromText(xml_text_A);

    std::vector<BT::TreeNodePtr> nodes;
    BT::TreeNodePtr root_node = parser.instantiateTree(nodes);

    BT::StdCoutLogger logger_cout(root_node.get());
    BT::FileLogger file_file(root_node.get(), "simple_trace.fbl", 32);

    std::cout << "\n------- First executeTick() --------" << std::endl;
    root_node->executeTick();
    std::cout << "\n------- sleep --------" << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    std::cout << "\n------- Second executeTick() --------" << std::endl;
    root_node->executeTick();
    std::cout << "\n------- sleep --------" << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    std::cout << "\n------- Third executeTick() --------" << std::endl;
    root_node->executeTick();
    std::cout << std::endl;

    std::cout << "\n-------\n";
    BT::XMLWriter writer(factory);
    std::cout << writer.writeXML( root_node.get() ) << std::endl;

    return 0;
}
