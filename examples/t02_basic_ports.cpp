#include "behaviortree_cpp/xml_parsing.h"

#include "dummy_nodes.h"
#include "movebase_node.h"

using namespace BT;

/** This tutorial will teach you how basic input/output ports work.
*/

// clang-format off
static const char* xml_text = R"(

 <root main_tree_to_execute = "MainTree" >

     <BehaviorTree ID="MainTree">
        <Sequence name="root">
            <SaySomething     message="start thinking..." />
            <ThinkWhatToSay   text="{the_answer}"/>
            <SaySomething     message="{the_answer}" />
            <SaySomething2    message="SaySomething2 works too..." />
            <SaySomething2    message="{the_answer}" />
        </Sequence>
     </BehaviorTree>

 </root>
 )";
// clang-format on

class ThinkWhatToSay : public BT::SyncActionNode
{
  public:
    ThinkWhatToSay(const std::string& name, const BT::NodeConfiguration& config)
      : BT::SyncActionNode(name, config)
    {
    }

    BT::NodeStatus tick() override
    {
        setOutput("text", "The answer is 42" );
        return BT::NodeStatus::SUCCESS;
    }

    static const BT::PortsList& providedPorts()
    {
        static BT::PortsList ports = { BT::OutputPort<std::string>("text") };
        return ports;
    }
};


int main()
{
    using namespace DummyNodes;

    BehaviorTreeFactory factory;

    // The class SaySomething has a method called providedPorts() that define the INPUTS
    // in this case it requires an input called "message"
    factory.registerNodeType<SaySomething>("SaySomething");


    // Similarly to SaySomething, ThinkWhatToSay has an OUTPUT port called "text"
    // Both these ports are std::string, therefore they can connect to each other
    factory.registerNodeType<ThinkWhatToSay>("ThinkWhatToSay");

    // SimpleActionNodes can not define their own method providedPorts(), therefore
    // we have to pass the PortsList explicitly if we want the Action to use getInput()
    // or setOutput();
    PortsList say_something_ports = { InputPort<std::string>("message") };
    factory.registerSimpleAction("SaySomething2", SaySomethingSimple, say_something_ports );

    /* An INPUT can be either a string, for instance:
     *
     *     <SaySomething message="start thinking..." />
     *
     * or contain a "pointer" to a type erased entry in the Blackboard,
     * using this syntax: {name_of_entry}. Example:
     *
     *     <SaySomething message="{the_answer}" />
     */

    auto tree = factory.createTreeFromText(xml_text);

    tree.root_node->executeTick();

    /*  Expected output:
     *
        Robot says: start thinking...
        Robot says: The answer is 42
        Robot says: SaySomething2 works too...
        Robot says: The answer is 42
    *
    * The way we "connect output ports to input ports is to "point" to the same
    * Blackboard entry.
    *
    * This means that ThinkSomething will write into the entry with key "the_answer";
    * SaySomething and SaySomething2 will read the message from the same entry.
    *
    */
    return 0;
}


