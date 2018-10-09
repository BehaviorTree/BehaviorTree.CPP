#ifndef DEADLINE_NODE_H
#define DEADLINE_NODE_H

#include "behavior_tree_core/decorator_node.h"
#include <atomic>
#include "timer_queue.h"

namespace BT
{
class DeadlineNode : public DecoratorNode
{
  public:

    DeadlineNode(const std::string& name, unsigned milliseconds);

    DeadlineNode(const std::string& name,const NodeParameters& params);

    static const NodeParameters& requiredNodeParameters()
    {
        static NodeParameters params = {{"msec", "0"}};
        return params;
    }

  private:
    static TimerQueue& timer()
    {
        static TimerQueue timer_queue;
        return timer_queue;
    }

    virtual BT::NodeStatus tick() override;

    std::atomic<bool> child_halted_;
    uint64_t timer_id_;

    unsigned msec_;
};
}

#endif // DEADLINE_NODE_H
