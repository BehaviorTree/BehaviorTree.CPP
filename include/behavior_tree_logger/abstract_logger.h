#ifndef ABSTRACT_LOGGER_H
#define ABSTRACT_LOGGER_H

#include "behavior_tree_core/behavior_tree.h"

namespace BT
{
class StatusChangeLogger
{
  public:
    StatusChangeLogger(TreeNode *root_node);
    virtual ~StatusChangeLogger() = default;

    virtual void callback(BT::TimePoint timestamp, const TreeNode& node, NodeStatus prev_status, NodeStatus status) = 0;

    virtual void flush() = 0;

    void setEnabled(bool enabled)
    {
        enabled_ = enabled;
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
};

//--------------------------------------------

inline StatusChangeLogger::StatusChangeLogger(TreeNode* root_node) :
    enabled_(true),
    show_transition_to_idle_(true)
{
    applyRecursiveVisitor(root_node, [this](TreeNode* node)
    {
        subscribers_.push_back(node->subscribeToStatusChange(
            [this](TimePoint timestamp, const TreeNode& node, NodeStatus prev, NodeStatus status) {
                if (enabled_ && (status != NodeStatus::IDLE || show_transition_to_idle_))
                {
                    this->callback(timestamp, node, prev, status);
                }
            }));
    });
}
}

#endif   // ABSTRACT_LOGGER_H
