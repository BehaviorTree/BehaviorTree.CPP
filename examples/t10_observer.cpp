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

  // Helper function to print the tree.
  BT::printTreeRecursively(tree.rootNode());

  // The purpose of the observer is to save some statistics about the number of times
  // a certain node returns SUCCESS or FAILURE.
  // This is particularly useful to create unit tests and to check if
  // a certain set of transitions happened as expected
  BT::TreeObserver observer(tree);

  std::map<int, std::string> UID_to_path;

  // Print the unique ID and the corresponding human readable path
  // Path is also expected to be unique.
  tree.applyVisitor([&UID_to_path](BT::TreeNode* node) {
    UID_to_path[node->UID()] = node->fullPath();
    std::cout << node->UID() << " -> " << node->fullPath() << std::endl;
  });

  tree.tickWhileRunning();

  // You can access a specific statistic, using is full path or the UID
  const auto& last_action_stats = observer.getStatistics("last_action");
  assert(last_action_stats.transitions_count > 0);

  std::cout << "----------------" << std::endl;
  // print all the statistics
  for(const auto& [uid, name] : UID_to_path)
  {
    const auto& stats = observer.getStatistics(uid);

    std::cout << "[" << name << "] \tT/S/F:  " << stats.transitions_count << "/"
              << stats.success_count << "/" << stats.failure_count << std::endl;
  }

  return 0;
}
