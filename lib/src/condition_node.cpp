#include <condition_node.h>


BT::ConditionNode::ConditionNode(std::string name) : LeafNode::LeafNode(name)
{
    type_ = BT::CONDITION_NODE;
}

BT::ConditionNode::~ConditionNode() {}

void BT::ConditionNode::Halt() {}

int BT::ConditionNode::DrawType()
{
    // Lock acquistion

    return BT::CONDITION;
}
