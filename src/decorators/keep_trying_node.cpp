
#include "behaviortree_cpp_v3/decorators/keep_trying_node.h"

namespace BT
{
constexpr const char* KeepTryingNode::NUM_ATTEMPTS;

KeepTryingNode::KeepTryingNode(const std::string& name, int NTries)
    : DecoratorNode(name, {} ),
    max_attempts_(NTries),
    try_index_(0),
    read_parameter_from_ports_(false)
{
    setRegistrationID("KeepTryingUntilSuccessful");
}

KeepTryingNode::KeepTryingNode(const std::string& name, const NodeConfiguration& config)
  : DecoratorNode(name, config),
    max_attempts_(0),
    try_index_(0),
    read_parameter_from_ports_(true)
{
}

void KeepTryingNode::halt()
{
    try_index_ = 0;
    DecoratorNode::halt();
}

NodeStatus KeepTryingNode::tick()
{
    if( read_parameter_from_ports_ )
    {
        if( !getInput(NUM_ATTEMPTS, max_attempts_) )
        {
            throw RuntimeError("Missing parameter [", NUM_ATTEMPTS,"] in KeepTryingNode");
        }
    }

    setStatus(NodeStatus::RUNNING);

    if(try_index_ < max_attempts_ || max_attempts_ == -1)
    {
        NodeStatus child_state = child_node_->executeTick();
        switch (child_state)
        {   
            case NodeStatus::SUCCESS:
            {
                try_index_ = 0;
                haltChild();
                return (NodeStatus::SUCCESS);
            }

            case NodeStatus::FAILURE:
            {
                try_index_++;
                haltChild();
                return NodeStatus::RUNNING;
            }
            break;

            case NodeStatus::RUNNING:
            {
                return NodeStatus::RUNNING;
            }

            default:
            {
                throw LogicError("A child node must never return IDLE");
            }
        }
    }else{
        try_index_ = 0;
        return NodeStatus::FAILURE;
    }
}

}
