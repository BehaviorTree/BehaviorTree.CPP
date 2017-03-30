#include <control_node.h>


BT::ControlNode::ControlNode(std::string name) : TreeNode::TreeNode(name)
{
    type_ = BT::CONTROL_NODE;
    ReturnStatus child_i_status_ = BT::IDLE;

}

BT::ControlNode::~ControlNode() {}

void BT::ControlNode::AddChild(TreeNode* child)
{
//    // Checking if the child is not already present
//    for (unsigned int i=0; i<children_nodes_.size(); i++)
//    {
//        if (children_nodes_[i] == child)
//        {
//            throw BehaviorTreeException("'" + child->get_name() + "' is already a '" + get_name() + "' child.");
//        }
//    }

    children_nodes_.push_back(child);
    children_states_.push_back(BT::IDLE);
}

unsigned int BT::ControlNode::GetChildrenNumber()
{
    return children_nodes_.size();
}

void BT::ControlNode::Halt()
{
    DEBUG_STDOUT("HALTING: "<< get_name());
    HaltChildren(0);
    set_status(BT::HALTED);
}



std::vector<BT::TreeNode*> BT::ControlNode::GetChildren()
{
    return children_nodes_;
}


void BT::ControlNode::ResetColorState()
{

    set_color_status(BT::IDLE);
    for(unsigned int i = 0; i < children_nodes_.size(); i++)
    {
        children_nodes_[i]->ResetColorState();
    }
}

void BT::ControlNode::HaltChildren(int i){
    for(unsigned int j=i; j<children_nodes_.size(); j++)
    {
        //TODO FIX this
        //        if (children_nodes_[j]->get_type() != BT::CONDITION_NODE  && children_nodes_[j]->get_status() != BT::IDLE)

        if (children_nodes_[j]->get_type() == BT::CONDITION_NODE)
        {
            children_nodes_[i]->ResetColorState();
        }
        else
        {

            if (children_nodes_[j]->get_status() == BT::RUNNING)
            {
                DEBUG_STDOUT("SENDING HALT TO CHILD " << children_nodes_[j]-> get_name());
                children_nodes_[j]->Halt();

            }
            else
            {
                DEBUG_STDOUT("NO NEED TO HALT " << children_nodes_[j]-> get_name() << "STATUS" << children_nodes_[j]->get_status());
            }
        }
    }

}

int BT::ControlNode::Depth()
{
        int depMax = 0;
        int dep = 0;
        for (int i = 0; i < children_nodes_.size(); i++)
        {
            dep = (children_nodes_[i]->Depth());
           if (dep > depMax)
           {
               depMax = dep;
           }

        }
      return 1 + depMax;
}

