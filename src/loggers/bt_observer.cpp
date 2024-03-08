#include "behaviortree_cpp/loggers/bt_observer.h"
#include "behaviortree_cpp/decorator_node.h"
#include "behaviortree_cpp/control_node.h"

namespace BT
{
TreeObserver::TreeObserver(const BT::Tree& tree) : StatusChangeLogger(tree.rootNode())
{
  std::function<void(const TreeNode&)> recursiveStep;

  recursiveStep = [&](const TreeNode& node) {
    if(auto control = dynamic_cast<const ControlNode*>(&node))
    {
      for(const auto& child : control->children())
      {
        recursiveStep(*child);
      }
    }
    else if(auto decorator = dynamic_cast<const DecoratorNode*>(&node))
    {
      if(decorator->type() != NodeType::SUBTREE)
      {
        recursiveStep(*decorator->child());
      }
    }

    if(_path_to_uid.count(node.fullPath()) != 0)
    {
      throw LogicError("TreeObserver not built correctly. Report issue");
    }
    _path_to_uid[node.fullPath()] = node.UID();
  };

  for(const auto& subtree : tree.subtrees)
  {
    recursiveStep(*subtree->nodes.front());
  }

  for(const auto& [path, uid] : _path_to_uid)
  {
    _statistics[uid] = {};
    _uid_to_path[uid] = path;
  }
}

TreeObserver::~TreeObserver()
{}

void TreeObserver::callback(Duration timestamp, const TreeNode& node,
                            NodeStatus /*prev_status*/, NodeStatus status)
{
  auto& statistics = _statistics[node.UID()];
  statistics.current_status = status;
  statistics.last_timestamp = timestamp;

  if(status == NodeStatus::IDLE)
  {
    return;
  }

  statistics.transitions_count++;

  if(status == NodeStatus::SUCCESS)
  {
    statistics.last_result = status;
    statistics.success_count++;
  }
  else if(status == NodeStatus::FAILURE)
  {
    statistics.last_result = status;
    statistics.failure_count++;
  }
  else if(status == NodeStatus::SKIPPED)
  {
    statistics.skip_count++;
  }
}

const TreeObserver::NodeStatistics&
TreeObserver::getStatistics(const std::string& path) const
{
  auto it = _path_to_uid.find(path);
  if(it == _path_to_uid.end())
  {
    throw RuntimeError("TreeObserver::getStatistics: Invalid pattern");
  }
  return getStatistics(it->second);
}

const TreeObserver::NodeStatistics& TreeObserver::getStatistics(uint16_t uid) const
{
  auto it = _statistics.find(uid);
  if(it == _statistics.end())
  {
    throw RuntimeError("TreeObserver::getStatistics: Invalid UID");
  }
  return it->second;
}

const std::unordered_map<uint16_t, TreeObserver::NodeStatistics>&
TreeObserver::statistics() const
{
  return _statistics;
}

const std::unordered_map<std::string, uint16_t>& TreeObserver::pathToUID() const
{
  return _path_to_uid;
}

const std::map<uint16_t, std::string>& TreeObserver::uidToPath() const
{
  return _uid_to_path;
}

}  // namespace BT
