#include "behaviortree_cpp/bt_factory.h"
#include "dummy_nodes.h"

using namespace BT;

// clang-format off
static const char* xml_tree = R"(
<root BTCPP_format="4">
  <BehaviorTree ID="MainTree">
    <Sequence>
      <Script code="val_A:= 'john' "/>
      <Script code="val_B:= 42 "/>
      <SaySomething message="{val_A}" />
      <SaySomething message="hello world" />
      <SubTree ID="Sub" val="{val_A}" _autoremap="true" />
      <SaySomething message="{reply}" />
    </Sequence>
  </BehaviorTree>
  <BehaviorTree ID="Sub">
    <Sequence>
      <SaySomething message="{val}" />
      <SaySomething message="{val_B}" />
      <Script code="reply:= 'done' "/>
    </Sequence>
  </BehaviorTree>
</root>
 )";

// clang-format on

int main()
{
  BehaviorTreeFactory factory;
  factory.registerNodeType<DummyNodes::SaySomething>("SaySomething");
  factory.registerBehaviorTreeFromText(xml_tree);

  auto tree = factory.createTree("MainTree");

  // We want to create a memory of the blackboard in memory.
  // This is conveninet when we want to reset its state to the
  // original one.
  // It is certainly more efficient than destroying and creating the tree again,
  // in many casess.

  const auto backup_before_tick = BlackboardBackup(tree);
  tree.tickWhileRunning();

  // Restore the original status of the blackboard
  BlackboardRestore(backup_before_tick, tree);
  tree.tickWhileRunning();

  // Alternatively, we may want to save he values of the element in the blackboard
  // to file, to restore them again. We use JSON serialization for that.
  nlohmann::json json_after_tick = ExportTreeToJSON(tree);

  // The JSOn object can be saved to file. See https://github.com/nlohmann/json
  // for details. For the time being, we just print it in the console

  std::cout << "--- blackboard serialized as JSON: ----\n"
            << json_after_tick.dump(2) << std::endl;

  // We can restore the values of the blackboards using the JSON
  ImportTreeFromJSON(json_after_tick, tree);

  return 0;
}
