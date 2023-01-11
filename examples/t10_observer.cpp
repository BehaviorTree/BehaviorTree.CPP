#include "behaviortree_cpp/bt_factory.h"
#include "behaviortree_cpp/loggers/bt_observer.h"

/** Show the use of the TreeObserver.
 */

// clang-format off

static const char* xml_text = R"(
<root BTCPP_format="4">

    <BehaviorTree ID="MainTree">
        <Sequence>
            <Fallback>
                <AlwaysFailure name="failing_action"/>
                <SubTree ID="SubTreeA" name="mysub"/>
            </Fallback>
            <AlwaysSuccess name="last_action"/>
        </Sequence>
    </BehaviorTree>

    <BehaviorTree ID="SubTreeA">
        <Sequence>
            <AlwaysSuccess name="action_subA"/>
            <SubTree ID="SubTreeB" name="sub_nested"/>
            <SubTree ID="SubTreeB" />
        </Sequence>
    </BehaviorTree>

    <BehaviorTree ID="SubTreeB">
        <AlwaysSuccess name="action_subB"/>
    </BehaviorTree>

</root>
 )";

// clang-format on

int main()
{
  BT::BehaviorTreeFactory factory;

  factory.registerBehaviorTreeFromText(xml_text);
  auto tree = factory.createTree("MainTree");

  // helper function to print the tree
  BT::printTreeRecursively(tree.rootNode());
  BT::TreeObserver observer(tree);

  std::map<uint16_t, std::string> ordered_UID_to_path;
  for(const auto& [name, uid]: observer.pathToUID()) {
    ordered_UID_to_path[uid] = name;
  }

  for(const auto& [uid, name]: ordered_UID_to_path) {
    std::cout << uid << " -> " << name << std::endl;
  }

  // Tick multiple times, until action_B is finally ticked.
  const auto& action_B_stats = observer.getStatistics("last_action");

  std::cout << "----------------" << std::endl;

  while(action_B_stats.tick_count == 0) {
    tree.tickOnce();
  }

  std::cout << "----------------" << std::endl;

  for(const auto& [uid, name]: ordered_UID_to_path) {
    const auto& stats = observer.getStatistics(uid);

    std::cout << "[" << name
              << "] \tT/S/F:  " << stats.tick_count
              << "/" << stats.success_count
              << "/" << stats.failure_count
              << std::endl;
  }

  return 0;
}
