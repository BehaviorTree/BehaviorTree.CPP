#include "crossdoor_nodes.h"
#include "behaviortree_cpp_v3/bt_factory.h"
#include "behaviortree_cpp_v3/loggers/groot2_publisher.h"
#include "behaviortree_cpp_v3/xml_parsing.h"

/** We are using the same example in Tutorial 5,
 *  But this time we also show how to connect
 */

// clang-format off

static const char* xml_text = R"(
<root BTCPP_format="3">

  <BehaviorTree ID="MainTree">
    <Sequence>
      <Fallback>
        <IsDoorOpen/>
        <SubTree ID="DoorClosed"/>
      </Fallback>
      <PassThroughDoor/>
    </Sequence>
  </BehaviorTree>

  <BehaviorTree ID="DoorClosed">
    <Fallback name="tryOpen">
      <OpenDoor/>
      <RetryUntilSuccessful num_attempts="5">
        <UnlockDoor/>
      </RetryUntilSuccessful>>
    </Fallback>
  </BehaviorTree>

</root>
 )";

// clang-format on

int main()
{
  BT::BehaviorTreeFactory factory;

  CrossDoor::RegisterNodes(factory);

  // Groot2 editor requires a model of your registered Nodes.
  // You don't need to write that by hand, it can be automatically
  // generated using the following command.
  std::string xml_models = BT::writeTreeNodesModelXML(factory);

  factory.registerBehaviorTreeFromText(xml_text);
  auto tree = factory.createTree("MainTree");


  std::cout << "----------- XML file  ----------\n"
            << BT::WriteTreeToXML(tree, true, false)
            << "--------------------------------\n";

  // Connect the Groot2Publisher. This will allow Groot2 to
  // get the tree and poll status updates.
  BT::Groot2Publisher publisher(tree);

  // Add also two logger which save the transitions into a file.
  // Both formats are compatible with Groot2
  while(1)
  {
    std::cout << "Start" << std::endl;
    CrossDoor::ResetVariables();
    tree.tickRootWhileRunning();
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
  }

  return 0;
}
