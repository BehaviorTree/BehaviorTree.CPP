#include "behaviortree_cpp/loggers/abstract_logger.h"

#include "behaviortree_cpp/utils/callback_gate.h"

#include <mutex>

namespace BT
{

struct StatusChangeLogger::PImpl
{
  bool enabled = true;
  bool show_transition_to_idle = true;
  std::vector<TreeNode::StatusChangeSubscriber> subscribers;
  TimestampType type = TimestampType::absolute;
  BT::TimePoint first_timestamp = {};
  std::mutex callback_mutex;
  details::CallbackGate::Ptr callback_gate = std::make_shared<details::CallbackGate>();
};

StatusChangeLogger::StatusChangeLogger() : _p(std::make_unique<PImpl>())
{}

StatusChangeLogger::StatusChangeLogger(TreeNode* root_node) : StatusChangeLogger()
{
  subscribeToTreeChanges(root_node);
}

StatusChangeLogger::~StatusChangeLogger()
{
  unsubscribeFromTreeChanges();
}

void StatusChangeLogger::setEnabled(bool enabled)
{
  const std::lock_guard lk(_p->callback_mutex);
  _p->enabled = enabled;
}

void StatusChangeLogger::setTimestampType(TimestampType type)
{
  const std::lock_guard lk(_p->callback_mutex);
  _p->type = type;
}

bool StatusChangeLogger::enabled() const
{
  const std::lock_guard lk(_p->callback_mutex);
  return _p->enabled;
}

bool StatusChangeLogger::showsTransitionToIdle() const
{
  const std::lock_guard lk(_p->callback_mutex);
  return _p->show_transition_to_idle;
}

void StatusChangeLogger::enableTransitionToIdle(bool enable)
{
  const std::lock_guard lk(_p->callback_mutex);
  _p->show_transition_to_idle = enable;
}

void StatusChangeLogger::subscribeToTreeChanges(TreeNode* root_node)
{
  _p->first_timestamp = std::chrono::high_resolution_clock::now();

  auto subscribeCallback =
      [this, gate = _p->callback_gate](TimePoint timestamp, const TreeNode& node,
                                       NodeStatus prev, NodeStatus status) {
        if(!gate->tryStart())
        {
          return;
        }
        const details::CallbackGate::Guard callback_guard(gate);
        handleStatusChange(timestamp, node, prev, status);
      };

  auto visitor = [this, subscribeCallback](TreeNode* node) {
    _p->subscribers.push_back(
        node->subscribeToStatusChange(std::move(subscribeCallback)));
  };

  applyRecursiveVisitor(root_node, visitor);
}

void StatusChangeLogger::handleStatusChange(TimePoint timestamp, const TreeNode& node,
                                            NodeStatus prev, NodeStatus status)
{
  // Copy state under lock, then release before calling user code
  // This prevents recursive mutex locking when multiple nodes change status
  bool should_callback = false;
  Duration adjusted_timestamp;
  {
    std::unique_lock lk(_p->callback_mutex);
    if(_p->enabled && (status != NodeStatus::IDLE || _p->show_transition_to_idle))
    {
      should_callback = true;
      adjusted_timestamp = (_p->type == TimestampType::absolute) ?
                               timestamp.time_since_epoch() :
                               (timestamp - _p->first_timestamp);
    }
  }

  if(should_callback)
  {
    this->callback(adjusted_timestamp, node, prev, status);
  }
}

void StatusChangeLogger::unsubscribeFromTreeChanges()
{
  _p->callback_gate->closeAndDrain();
  _p->subscribers.clear();
}

}  // namespace BT
