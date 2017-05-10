/* Copyright (C) 2015-2017 Michele Colledanchise - All Rights Reserved
*
*   Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"),
*   to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
*   and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
*   The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
*
*   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
*   WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/


#include <control_node.h>
#include <string>
#include <vector>

BT::ControlNode::ControlNode(std::string name) : TreeNode::TreeNode(name)
{
    type_ = BT::CONTROL_NODE;

    // TODO(...) In case it is desired to set to idle remove the ReturnStatus
    // type in order to set the member variable
    // ReturnStatus child_i_status_ = BT::IDLE;  // commented out as unused
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
    for (unsigned int i = 0; i < children_nodes_.size(); i++)
    {
        children_nodes_[i]->ResetColorState();
    }
}

void BT::ControlNode::HaltChildren(int i)
{
    for (unsigned int j=i; j < children_nodes_.size(); j++)
    {
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
                DEBUG_STDOUT("NO NEED TO HALT " << children_nodes_[j]-> get_name()
                             << "STATUS" << children_nodes_[j]->get_status());
            }
        }
    }
}

int BT::ControlNode::Depth()
{
    int depMax = 0;
    int dep = 0;
    for (unsigned int i = 0; i < children_nodes_.size(); i++)
    {
        dep = (children_nodes_[i]->Depth());
        if (dep > depMax)
        {
            depMax = dep;
        }
    }
    return 1 + depMax;
}

