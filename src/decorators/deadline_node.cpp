#include "behavior_tree_core/decorators/deadline_node.h"

namespace BT {

DeadlineNode::DeadlineNode(const std::string &name,
                               unsigned milliseconds):
    DecoratorNode(name, {}),
    child_halted_(false),
    msec_(milliseconds)
{

}

DeadlineNode::DeadlineNode(const std::string &name,
                               const BT::NodeParameters &params) :
    DecoratorNode(name, params),
    child_halted_(false),
    msec_(0)
{
    auto param = getParam<unsigned>("msec");
    if( param )
    {
        msec_ = param.value();
    }
}

NodeStatus DeadlineNode::tick()
{
    if( status() == NodeStatus::IDLE )
    {
        setStatus(NodeStatus::RUNNING);
        child_halted_ = false;

        if( msec_ > 0 )
        {
            timer_id_ = timer().add( std::chrono::milliseconds(msec_),
                                     [this](bool aborted)
            {
                if( !aborted && child()->status() == NodeStatus::RUNNING )
                {
                    child()->halt();
                    child()->setStatus( NodeStatus::IDLE );
                    child_halted_ = true;
                }
            });
        }
    }

    if( child_halted_ )
    {
        setStatus( NodeStatus::FAILURE );
    }
    else{
        auto child_status =  child()->executeTick() ;
        if( child_status != NodeStatus::RUNNING)
        {
            child()->setStatus(NodeStatus::IDLE);
            timer().cancel( timer_id_ );
        }
        setStatus(child_status);
    }

    return status();
}

}
