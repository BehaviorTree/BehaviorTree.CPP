#ifndef ABSTRACT_LOGGER_H
#define ABSTRACT_LOGGER_H

#include "behaviortree_cpp/behavior_tree.h"
#include "behaviortree_cpp/bt_factory.h"

namespace BT
{
enum class TimestampType
{
  absolute,
  relative
};

class StatusChangeLogger
{
public:
  StatusChangeLogger(TreeNode* root_node);
  virtual ~StatusChangeLogger() = default;

  StatusChangeLogger(const StatusChangeLogger& other) = delete;
  StatusChangeLogger& operator=(const StatusChangeLogger& other) = delete;

  StatusChangeLogger(StatusChangeLogger&& other) = default;
  StatusChangeLogger& operator=(StatusChangeLogger&& other) = default;

  virtual void callback(BT::Duration timestamp, const TreeNode& node,
                        NodeStatus prev_status, NodeStatus status) = 0;

  virtual void flush() = 0;

  void setEnabled(bool enabled)
  {
    enabled_ = enabled;
  }

  void setTimestampType(TimestampType type)
  {
    type_ = type;
  }

  bool enabled() const
  {
    return enabled_;
  }

  // false by default.
  bool showsTransitionToIdle() const
  {
    return show_transition_to_idle_;
  }

  void enableTransitionToIdle(bool enable)
  {
    show_transition_to_idle_ = enable;
  }

private:
  bool enabled_;
  bool show_transition_to_idle_;
  std::vector<TreeNode::StatusChangeSubscriber> subscribers_;
  TimestampType type_;
  BT::TimePoint first_timestamp_ = {};
  std::mutex callback_mutex_;
};

//--------------------------------------------

inline StatusChangeLogger::StatusChangeLogger(TreeNode* root_node)
  : enabled_(true), show_transition_to_idle_(true), type_(TimestampType::absolute)
{
  first_timestamp_ = std::chrono::high_resolution_clock::now();

  auto subscribeCallback = [this](TimePoint timestamp, const TreeNode& node,
                                  NodeStatus prev, NodeStatus status) {
    std::unique_lock lk(callback_mutex_);
    if(enabled_ && (status != NodeStatus::IDLE || show_transition_to_idle_))
    {
      if(type_ == TimestampType::absolute)
      {
        this->callback(timestamp.time_since_epoch(), node, prev, status);
      }
      else
      {
        this->callback(timestamp - first_timestamp_, node, prev, status);
      }
    }
  };

  auto visitor = [this, subscribeCallback](TreeNode* node) {
    subscribers_.push_back(node->subscribeToStatusChange(std::move(subscribeCallback)));
  };

  applyRecursiveVisitor(root_node, visitor);
}
}  // namespace BT

#endif  // ABSTRACT_LOGGER_H
