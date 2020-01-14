/* Copyright (C) 2015-2018 Michele Colledanchise -  All Rights Reserved
 * Copyright (C) 2018-2019 Davide Faconti, Eurecat -  All Rights Reserved
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

#include "behaviortree_cpp_v3/tree_node.h"
#include <cstring>

namespace BT
{
static uint16_t getUID()
{
    static uint16_t uid = 1;
    return uid++;
}

TreeNode::TreeNode(std::string name, NodeConfiguration config)
  : name_(std::move(name)),
    status_(NodeStatus::IDLE),
    uid_(getUID()),
    config_(std::move(config))
{
}

NodeStatus TreeNode::executeTick()
{
    const NodeStatus status = tick();
    setStatus(status);
    return status;
}

void TreeNode::setStatus(NodeStatus new_status)
{
    NodeStatus prev_status;
    {
        std::unique_lock<std::mutex> UniqueLock(state_mutex_);
        prev_status = status_;
        status_ = new_status;
    }
    if (prev_status != new_status)
    {
        state_condition_variable_.notify_all();
        state_change_signal_.notify(std::chrono::high_resolution_clock::now(), *this, prev_status,
                                    new_status);
    }
}

NodeStatus TreeNode::status() const
{
    std::lock_guard<std::mutex> lock(state_mutex_);
    return status_;
}

NodeStatus TreeNode::waitValidStatus()
{
    std::unique_lock<std::mutex> lock(state_mutex_);

    while( isHalted() )
    {
        state_condition_variable_.wait(lock);
    }
    return status_;
}

const std::string& TreeNode::name() const
{
    return name_;
}

bool TreeNode::isHalted() const
{
    return status_ == NodeStatus::IDLE;
}

TreeNode::StatusChangeSubscriber
TreeNode::subscribeToStatusChange(TreeNode::StatusChangeCallback callback)
{
    return state_change_signal_.subscribe(std::move(callback));
}

uint16_t TreeNode::UID() const
{
    return uid_;
}

const std::string& TreeNode::registrationName() const
{
    return registration_ID_;
}

const NodeConfiguration &TreeNode::config() const
{
    return config_;
}

bool TreeNode::isBlackboardPointer(StringView str)
{
    const auto size = str.size();
    if( size >= 3 && str.back() == '}')
    {
        if( str[0] == '{') {
            return true;
        }
        if( size >= 4 && str[0] == '$' && str[1] == '{') {
            return true;
        }
    }
    return false;
}

StringView TreeNode::stripBlackboardPointer(StringView str)
{
    const auto size = str.size();
    if( size >= 3 && str.back() == '}')
    {
        if( str[0] == '{') {
            return str.substr(1, size-2);
        }
        if( str[0] == '$' && str[1] == '{') {
            return str.substr(2, size-3);
        }
    }
    return {};
}

Optional<StringView> TreeNode::getRemappedKey(StringView port_name, StringView remapping_value)
{
    if( remapping_value == "=" )
    {
        return {port_name};
    }
    if( isBlackboardPointer( remapping_value ) )
    {
        return {stripBlackboardPointer(remapping_value)};
    }
    return nonstd::make_unexpected("Not a blackboard pointer");
}

void TreeNode::modifyPortsRemapping(const PortsRemapping &new_remapping)
{
    for (const auto& new_it: new_remapping)
    {
        auto it = config_.input_ports.find( new_it.first );
        if( it != config_.input_ports.end() )
        {
            it->second = new_it.second;
        }
        it = config_.output_ports.find( new_it.first );
        if( it != config_.output_ports.end() )
        {
            it->second = new_it.second;
        }
    }
}

}   // end namespace
