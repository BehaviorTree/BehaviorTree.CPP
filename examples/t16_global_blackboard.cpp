#include "behaviortree_cpp/bt_factory.h"

using namespace BT;

/**
 * This example introduces the concept of a "global blackboard",
 * and the syntax to use it.
 *
 * As you know know from previous tutorials, blackboard are "scoped",
 * i.e. each SubTree (including the one in the root) has its own
 * Blackboard, isolated by default, unless we do remapping.
 *
 * It is possible (since version 4.6) to create a global BB,
 * accessible from everywhere without remapping.
 *
 * In the example below we can access the entry "value" and
 * "value_sqr" from everywhere, as long as we use the pregix "@".
 *
 * Note as <SubTree ID="MySub"/> doesn't have any remapping
 *
 * In other words, the prefix "@" means: "search the entry in the top-level
 * blackboard of the hierarchy".
 *
 * In this case, the top-level blackboard will be [global_blackboard].
 */

// clang-format off
static const char* xml_main = R"(
<root BTCPP_format="4">

  <BehaviorTree ID="MainTree">
    <Sequence>
      <PrintNumber name="main_print" val="{@value}" />
      <SubTree ID="MySub"/>
    </Sequence>
  </BehaviorTree>

  <BehaviorTree ID="MySub">
    <Sequence>
      <PrintNumber name="sub_print" val="{@value}" />
      <Script code="@value_sqr := @value * @value" />
    </Sequence>
  </BehaviorTree>
</root>
 )";

// clang-format on

class PrintNumber : public BT::SyncActionNode
{
public:
  PrintNumber(const std::string& name, const BT::NodeConfig& config)
    : BT::SyncActionNode(name, config)
  {}

  NodeStatus tick() override
  {
    const int val = getInput<int>("val").value();
    // If you prefer not having a port and accessing the top-level blackboard
    // directly with an hardcoded address... you should question your own choices!
    // But this is the way it is done
    // val = config().blackboard-><int>("@value");
    std::cout << "[" << name() << "] val: " << val << std::endl;
    return NodeStatus::SUCCESS;
  }

  static BT::PortsList providedPorts()
  {
    return { BT::InputPort<int>("val") };
  }
};

//---------------------------------------------------
int main()
{
  BehaviorTreeFactory factory;

  factory.registerNodeType<PrintNumber>("PrintNumber");
  factory.registerBehaviorTreeFromText(xml_main);

  // No one "own" this blackboard
  auto global_blackboard = BT::Blackboard::create();
  // This blackboard will be owned by "MainTree". Its parent is global_blackboard
  auto root_blackboard = BT::Blackboard::create(global_blackboard);

  auto tree = factory.createTree("MainTree", root_blackboard);

  // we can interact directly with global_blackboard
  for(int i = 1; i <= 3; i++)
  {
    global_blackboard->set("value", i);
    tree.tickOnce();
    int value_sqr = global_blackboard->get<int>("value_sqr");
    std::cout << "[While loop] value: " << i << " value_sqr: " << value_sqr << "\n\n";
  }

  return 0;
}

/* Expected output:

[main_print] val: 1
[sub_print] val: 1
[While loop] value: 1 value_sqr: 1

[main_print] val: 2
[sub_print] val: 2
[While loop] value: 2 value_sqr: 4

[main_print] val: 3
[sub_print] val: 3
[While loop] value: 3 value_sqr: 9

*/
