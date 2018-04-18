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

#include "behavior_tree/tree_node.h"
#include <string>


BT::TreeNode::TreeNode(std::string name) : tick_engine(0)
{
    // Initialization
    name_ = name;
    is_state_updated_ = false;
    SetStatus(BT::IDLE);
}

BT::TreeNode::~TreeNode() {}

void BT::TreeNode::SetStatus(ReturnStatus new_status)
{
    // Lock acquistion
    std::unique_lock<std::mutex> UniqueLock(state_mutex_);

    // state_ update
    status_ = new_status;
}

BT::ReturnStatus BT::TreeNode::Status() const
{
    // Lock acquistion
    DEBUG_STDOUT(Name() << " is setting its status to " << status_);

    std::lock_guard<std::mutex> LockGuard(state_mutex_);
    return status_;
}



void BT::TreeNode::SetName(const std::string &new_name)
{
    name_ = new_name;
}

const std::string& BT::TreeNode::Name() const
{
    return name_;
}

bool BT::TreeNode::IsHalted() const
{
    return Status() == BT::HALTED;
}
