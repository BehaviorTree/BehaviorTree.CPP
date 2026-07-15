#ifndef ABSTRACT_LOGGER_H
#define ABSTRACT_LOGGER_H

#include "behaviortree_cpp/behavior_tree.h"
#include "behaviortree_cpp/bt_factory.h"

#include <memory>

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

  virtual ~StatusChangeLogger();

  StatusChangeLogger(const StatusChangeLogger& other) = delete;
  StatusChangeLogger& operator=(const StatusChangeLogger& other) = delete;

  StatusChangeLogger(StatusChangeLogger&& other) = delete;
  StatusChangeLogger& operator=(StatusChangeLogger&& other) = delete;

  virtual void callback(BT::Duration timestamp, const TreeNode& node,
                        NodeStatus prev_status, NodeStatus status) = 0;

  virtual void flush() = 0;

  void setEnabled(bool enabled);

  void setTimestampType(TimestampType type);

  bool enabled() const;

  // false by default.
  bool showsTransitionToIdle() const;

  void enableTransitionToIdle(bool enable);

protected:
  /// Default constructor for deferred subscription. Call subscribeToTreeChanges() when ready.
  StatusChangeLogger();

  /// Subscribe to status changes. Call at end of constructor for deferred subscription.
  void subscribeToTreeChanges(TreeNode* root_node);

  /// Stop new callbacks and wait until callbacks already in progress have completed.
  /// Derived destructors must call this before destroying state used by callback().
  void unsubscribeFromTreeChanges();

private:
  /// Forward a status transition to callback(), unless disabled.
  void handleStatusChange(TimePoint timestamp, const TreeNode& node, NodeStatus prev,
                          NodeStatus status);

  // All state lives behind this pointer, so that changing it does not alter the
  // layout that derived classes (including user-defined loggers) compile against.
  struct PImpl;
  std::unique_ptr<PImpl> _p;
};
}  // namespace BT

#endif  // ABSTRACT_LOGGER_H
