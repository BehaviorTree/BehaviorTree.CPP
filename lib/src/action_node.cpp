#include <action_node.h>



BT::ActionNode::ActionNode(std::string name) : LeafNode::LeafNode(name)
{
    type_ = BT::ACTION_NODE;
}

BT::ActionNode::~ActionNode() {}


BT::ReturnStatus BT::ActionNode::Tick(){ return BT::EXIT;}//not used in action node.
                                                       //An action node runs the WaitForTick() instead throuh the tick engine.


int BT::ActionNode::DrawType()
{
    // Lock acquistion

    return BT::ACTION;
}
