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
  /// Construct and immediately subscribe to status changes.
  StatusChangeLogger(TreeNode* root_node);

  virtual ~StatusChangeLogger() = default;

  StatusChangeLogger(const StatusChangeLogger& other) = delete;
  StatusChangeLogger& operator=(const StatusChangeLogger& other) = delete;

  StatusChangeLogger(StatusChangeLogger&& other) = delete;
  StatusChangeLogger& operator=(StatusChangeLogger&& other) = delete;

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

protected:
  /// Default constructor for deferred subscription. Call subscribeToTreeChanges() when ready.
  StatusChangeLogger() = default;

  /// Subscribe to status changes. Call at end of constructor for deferred subscription.
  void subscribeToTreeChanges(TreeNode* root_node);

private:
  bool enabled_ = true;
  bool show_transition_to_idle_ = true;
  std::vector<TreeNode::StatusChangeSubscriber> subscribers_;
  TimestampType type_ = TimestampType::absolute;
  BT::TimePoint first_timestamp_ = {};
  std::mutex callback_mutex_;
};

//--------------------------------------------

inline StatusChangeLogger::StatusChangeLogger(TreeNode* root_node)
{
  subscribeToTreeChanges(root_node);
}

inline void StatusChangeLogger::subscribeToTreeChanges(TreeNode* root_node)
{
  first_timestamp_ = std::chrono::high_resolution_clock::now();

  auto subscribeCallback = [this](TimePoint timestamp, const TreeNode& node,
                                  NodeStatus prev, NodeStatus status) {
    // Copy state under lock, then release before calling user code
    // This prevents recursive mutex locking when multiple nodes change status
    bool should_callback = false;
    Duration adjusted_timestamp;
    {
      std::unique_lock lk(callback_mutex_);
      if(enabled_ && (status != NodeStatus::IDLE || show_transition_to_idle_))
      {
        should_callback = true;
        adjusted_timestamp = (type_ == TimestampType::absolute) ?
                                 timestamp.time_since_epoch() :
                                 (timestamp - first_timestamp_);
      }
    }
    if(should_callback)
    {
      this->callback(adjusted_timestamp, node, prev, status);
    }
  };

  auto visitor = [this, subscribeCallback](TreeNode* node) {
    subscribers_.push_back(node->subscribeToStatusChange(std::move(subscribeCallback)));
  };

  applyRecursiveVisitor(root_node, visitor);
}
}  // namespace BT

#endif  // ABSTRACT_LOGGER_H
