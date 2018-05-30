#ifndef ABSTRACT_LOGGER_H
#define ABSTRACT_LOGGER_H

#include "behavior_tree_core/behavior_tree.h"


namespace BT{

class StatusChangeLogger {

public:

    StatusChangeLogger(TreeNode* root_node);
    virtual ~StatusChangeLogger() = default;

    virtual void callback(BT::TimePoint timestamp,
                          const TreeNode& node,
                          NodeStatus prev_status,
                          NodeStatus status) = 0;

    virtual void flush() = 0;

    void setEnabled(bool enabled) { _enabled = enabled; }

    bool enabled() const          { return _enabled; }

    // false by default.
    bool showsTransitionToIdle() const { return _show_transition_to_idle; }

    void enableTransitionToIdle(bool enable ) { _show_transition_to_idle = enable; }

private:
    bool _enabled;
    bool _show_transition_to_idle;
    std::vector<TreeNode::StatusChangeSubscriber> _subscribers;
};

//--------------------------------------------

inline StatusChangeLogger::StatusChangeLogger(TreeNode *root_node):
    _enabled(true),
    _show_transition_to_idle(true)
{
    recursiveVisitor(root_node, [this](TreeNode* node)
    {
        _subscribers.push_back( node->subscribeToStatusChange(
                                    [this](TimePoint timestamp,
                                    const TreeNode& node,
                                    NodeStatus prev,
                                    NodeStatus status)
        {
            if(_enabled && ( status != NodeStatus::IDLE || _show_transition_to_idle))
            {
                this->callback(timestamp, node,prev,status);
            }
        }));
    });
}


}

#endif // ABSTRACT_LOGGER_H
