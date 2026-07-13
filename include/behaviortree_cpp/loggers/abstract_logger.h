#ifndef ABSTRACT_LOGGER_H
#define ABSTRACT_LOGGER_H

#include "behaviortree_cpp/behavior_tree.h"
#include "behaviortree_cpp/bt_factory.h"

#include <condition_variable>

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

  void setEnabled(bool enabled)
  {
    const std::lock_guard lk(callback_mutex_);
    enabled_ = enabled;
  }

  void setTimestampType(TimestampType type)
  {
    const std::lock_guard lk(callback_mutex_);
    type_ = type;
  }

  bool enabled() const
  {
    const std::lock_guard lk(callback_mutex_);
    return enabled_;
  }

  // false by default.
  bool showsTransitionToIdle() const
  {
    const std::lock_guard lk(callback_mutex_);
    return show_transition_to_idle_;
  }

  void enableTransitionToIdle(bool enable)
  {
    const std::lock_guard lk(callback_mutex_);
    show_transition_to_idle_ = enable;
  }

protected:
  /// Default constructor for deferred subscription. Call subscribeToTreeChanges() when ready.
  StatusChangeLogger() = default;

  /// Subscribe to status changes. Call at end of constructor for deferred subscription.
  void subscribeToTreeChanges(TreeNode* root_node);

  /// Stop new callbacks and wait until callbacks already in progress have completed.
  /// Derived destructors must call this before destroying state used by callback().
  void unsubscribeFromTreeChanges();

private:
  struct CallbackState
  {
    bool tryStartCallback()
    {
      const std::lock_guard lk(mutex);
      if(!accepting_callbacks)
      {
        return false;
      }
      running_callbacks++;
      return true;
    }

    void callbackCompleted()
    {
      const std::lock_guard lk(mutex);
      running_callbacks--;
      if(running_callbacks == 0)
      {
        condition_variable.notify_all();
      }
    }

    std::mutex mutex;
    std::condition_variable condition_variable;
    bool accepting_callbacks = true;
    size_t running_callbacks = 0;
  };

  struct CallbackGuard
  {
    explicit CallbackGuard(std::shared_ptr<CallbackState> state) : state(std::move(state))
    {}

    CallbackGuard(const CallbackGuard&) = delete;
    CallbackGuard& operator=(const CallbackGuard&) = delete;
    CallbackGuard(CallbackGuard&&) = delete;
    CallbackGuard& operator=(CallbackGuard&&) = delete;

    ~CallbackGuard()
    {
      state->callbackCompleted();
    }

    std::shared_ptr<CallbackState> state;
  };

  bool enabled_ = true;
  bool show_transition_to_idle_ = true;
  std::vector<TreeNode::StatusChangeSubscriber> subscribers_;
  TimestampType type_ = TimestampType::absolute;
  BT::TimePoint first_timestamp_ = {};
  mutable std::mutex callback_mutex_;
  std::shared_ptr<CallbackState> callback_state_ = std::make_shared<CallbackState>();
};

//--------------------------------------------

inline StatusChangeLogger::StatusChangeLogger(TreeNode* root_node)
{
  subscribeToTreeChanges(root_node);
}

inline StatusChangeLogger::~StatusChangeLogger()
{
  unsubscribeFromTreeChanges();
}

inline void StatusChangeLogger::subscribeToTreeChanges(TreeNode* root_node)
{
  first_timestamp_ = std::chrono::high_resolution_clock::now();
  auto callback_state = callback_state_;

  {
    const std::lock_guard lk(callback_state->mutex);
    callback_state->accepting_callbacks = true;
  }

  auto subscribeCallback = [this, callback_state](TimePoint timestamp,
                                                  const TreeNode& node, NodeStatus prev,
                                                  NodeStatus status) {
    if(!callback_state->tryStartCallback())
    {
      return;
    }
    const CallbackGuard callback_guard(callback_state);

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

inline void StatusChangeLogger::unsubscribeFromTreeChanges()
{
  auto callback_state = callback_state_;
  {
    const std::lock_guard lk(callback_state->mutex);
    callback_state->accepting_callbacks = false;
  }

  subscribers_.clear();

  std::unique_lock lk(callback_state->mutex);
  callback_state->condition_variable.wait(
      lk, [&callback_state] { return callback_state->running_callbacks == 0; });
}
}  // namespace BT

#endif  // ABSTRACT_LOGGER_H
