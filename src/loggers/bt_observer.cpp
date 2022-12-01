#include "behaviortree_cpp/loggers/bt_observer.h"
#include "behaviortree_cpp/decorator_node.h"
#include "behaviortree_cpp/control_node.h"

namespace BT
{
TreeObserver::TreeObserver(const BT::Tree& tree) : StatusChangeLogger(tree.rootNode())
{
  std::function<void(const TreeNode&, std::string)> recursiveStep;

  recursiveStep = [&](const TreeNode& node, std::string prefix){


    if (auto subtree = dynamic_cast<const SubTreeNode*>(&node))
    {
      for(auto const& sub: tree.subtrees)
      {
        if(sub->nodes.front().get() == subtree->child())
        {
          auto sub_prefix = prefix + sub->instance_name;
          _path_to_uid[sub_prefix] = node.UID();
          recursiveStep(*subtree->child(), sub_prefix + "/");
          return;
        }
      }
    }

    auto new_prefix = prefix + node.name();
    if(node.name() == node.registrationName()) {
      new_prefix += "::" + std::to_string(node.UID());
    }

    if (auto control = dynamic_cast<const ControlNode*>(&node))
    {
      for (const auto& child : control->children())
      {
        recursiveStep(*child, prefix);
      }
    }
    else if (auto decorator = dynamic_cast<const DecoratorNode*>(&node))
    {
      recursiveStep(*decorator->child(), prefix);
    }

    if( _path_to_uid.count(new_prefix) != 0 ) {
      throw LogicError("TreeObserver not built correctly. Report issue");
    }
    _path_to_uid[new_prefix] = node.UID();
  };

  recursiveStep(*tree.subtrees.front()->nodes.front(),
                tree.subtrees.front()->instance_name);

  for(const auto& [path, uid]: _path_to_uid) {
    _statistics[uid] = {};
  }
}

TreeObserver::~TreeObserver()
{

}

void TreeObserver::callback(Duration timestamp, const TreeNode& node,
                            NodeStatus /*prev_status*/, NodeStatus status)
{
  auto& statistics = _statistics[node.UID()];

  statistics.tick_count++;
  statistics.current_status = status;
  statistics.last_timestamp = timestamp;

  if(status == NodeStatus::SUCCESS)
  {
    statistics.last_result = NodeStatus::SUCCESS;
    statistics.success_count++;
  }
  else if(status == NodeStatus::FAILURE)
  {
    statistics.last_result = NodeStatus::FAILURE;
    statistics.failure_count++;
  }
}

const TreeObserver::NodeStatistics& TreeObserver::getStatistics(const std::string &path) const
{
  auto it = _path_to_uid.find(path);
  if(it == _path_to_uid.end()) {
    throw RuntimeError("TreeObserver::getStatistics: Invalid pattern");
  }
  return getStatistics(it->second);
}

const TreeObserver::NodeStatistics &TreeObserver::getStatistics(uint16_t uid) const
{
  auto it = _statistics.find(uid);
  if(it == _statistics.end()) {
    throw RuntimeError("TreeObserver::getStatistics: Invalid UID");
  }
  return it->second;
}

const std::unordered_map<uint16_t, TreeObserver::NodeStatistics> &TreeObserver::statistics() const
{
  return _statistics;
}

const std::unordered_map<std::string, uint16_t> &TreeObserver::pathToUID() const {
  return _path_to_uid;
}


}   // namespace BT
