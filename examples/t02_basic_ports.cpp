#include "behaviortree_cpp/xml_parsing.h"

#include "dummy_nodes.h"
#include "movebase_node.h"

using namespace BT;

/** This tutorial will teach you how basic input/output ports work
*/

// clang-format off

static const char* xml_text = R"(

 <root main_tree_to_execute = "MainTree" >

     <BehaviorTree ID="MainTree">
        <Sequence name="root">
            <SaySomething        message="start thinking..." />
            <ThinkSomethingToSay text="{the_answer}"/>
            <SaySomething        message="{the_answer}" />
            <SaySomething2       message="SaySomething2 works too..." />
            <SaySomething2       message="{the_answer}" />
        </Sequence>
     </BehaviorTree>

 </root>
 )";


// clang-format on


class ThinkSomethingToSay : public BT::SyncActionNode
{
  public:
    ThinkSomethingToSay(const std::string& name, const BT::NodeConfiguration& config)
      : BT::SyncActionNode(name, config)
    {
    }

    BT::NodeStatus tick() override
    {
        setOutput("text","The answer is 42");
        return BT::NodeStatus::SUCCESS;
    }

    static const BT::PortsList& providedPorts()
    {
        static BT::PortsList ports = {{"text", {BT::PortType::OUTPUT, typeid(std::string)} }};
        return ports;
    }
};


int main()
{
    using namespace DummyNodes;

    BehaviorTreeFactory factory;
    factory.registerNodeType<SaySomething>("SaySomething");
    factory.registerNodeType<ThinkSomethingToSay>("ThinkSomethingToSay");

    PortsList say_something_ports = {{"message", PortType::INPUT}};
    factory.registerSimpleAction("SaySomething2", SaySomethingSimple, say_something_ports );

    auto tree = buildTreeFromText(factory, xml_text);

    NodeStatus status = tree.root_node->executeTick();

    return 0;
}

/*
 Expected output:



*/
