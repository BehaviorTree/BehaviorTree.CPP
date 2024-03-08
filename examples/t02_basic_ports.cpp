#include "behaviortree_cpp/bt_factory.h"

#include "dummy_nodes.h"

using namespace BT;

/** This tutorial will teach you how basic input/output ports work.
 *
 * Ports are a mechanism to exchange information between Nodes using
 * a key/value storage called "Blackboard".
 * The type and number of ports of a Node is statically defined.
 *
 * Input Ports are like "argument" of a functions.
 * Output ports are, conceptually, like "return values".
 *
 * In this example, a Sequence of 5 Actions is executed:
 *
 *   - Actions 1 and 4 read the input "message" from a static string.
 *
 *   - Actions 3 and 5 read the input "message" from an entry in the
 *     blackboard called "the_answer".
 *
 *   - Action 2 writes something into the entry of the blackboard
 *     called "the_answer".
*/

// clang-format off
static const char* xml_text = R"(

 <root BTCPP_format="4" >

     <BehaviorTree ID="MainTree">
        <Sequence name="root">
            <SaySomething     message="hello" />
            <SaySomething2    message="this works too" />
            <ThinkWhatToSay   text="{the_answer}"/>
            <SaySomething2    message="{the_answer}" />
        </Sequence>
     </BehaviorTree>

 </root>
 )";
// clang-format on

class ThinkWhatToSay : public BT::SyncActionNode
{
public:
  ThinkWhatToSay(const std::string& name, const BT::NodeConfig& config)
    : BT::SyncActionNode(name, config)
  {}

  // This Action simply write a value in the port "text"
  BT::NodeStatus tick() override
  {
    setOutput("text", "The answer is 42");
    return BT::NodeStatus::SUCCESS;
  }

  // A node having ports MUST implement this STATIC method
  static BT::PortsList providedPorts()
  {
    return { BT::OutputPort<std::string>("text") };
  }
};

int main()
{
  using namespace DummyNodes;

  BehaviorTreeFactory factory;

  // The class SaySomething has a method called providedPorts() that define the INPUTS.
  // In this case, it requires an input called "message"
  factory.registerNodeType<SaySomething>("SaySomething");

  // Similarly to SaySomething, ThinkWhatToSay has an OUTPUT port called "text"
  // Both these ports are std::string, therefore they can connect to each other
  factory.registerNodeType<ThinkWhatToSay>("ThinkWhatToSay");

  // SimpleActionNodes can not define their own method providedPorts(), therefore
  // we have to pass the PortsList explicitly if we want the Action to use getInput()
  // or setOutput();
  PortsList say_something_ports = { InputPort<std::string>("message") };
  factory.registerSimpleAction("SaySomething2", SaySomethingSimple, say_something_ports);

  /* An INPUT can be either a string, for instance:
     *
     *     <SaySomething message="hello" />
     *
     * or contain a "pointer" to a type erased entry in the Blackboard,
     * using this syntax: {name_of_entry}. Example:
     *
     *     <SaySomething message="{the_answer}" />
     */

  auto tree = factory.createTreeFromText(xml_text);

  tree.tickWhileRunning();

  /*  Expected output:
     *
        Robot says: hello
        Robot says: this works too
        Robot says: The answer is 42
    *
    * The way we "connect" output ports to input ports is to "point" to the same
    * Blackboard entry.
    *
    * This means that ThinkSomething will write into the entry with key "the_answer";
    * SaySomething and SaySomething will read the message from the same entry.
    *
    */
  return 0;
}
