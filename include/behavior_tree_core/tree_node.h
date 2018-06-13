/* Copyright (C) 2015-2018 Michele Colledanchise -  All Rights Reserved
 * Copyright (C) 2018 Davide Faconti -  All Rights Reserved
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

#ifndef BEHAVIORTREECORE_TREENODE_H
#define BEHAVIORTREECORE_TREENODE_H

#include <iostream>
#include <string>
#include <map>
#include <set>

#include "behavior_tree_core/tick_engine.h"
#include "behavior_tree_core/exceptions.h"
#include "behavior_tree_core/signal.h"
#include "behavior_tree_core/basic_types.h"

namespace BT
{

// We call Parameters the set of Key/Values that can be read from file and are
// used to parametrize an object. It is up to the user's code to parse the string.
typedef std::map<std::string, std::string> NodeParameters;

typedef std::chrono::high_resolution_clock::time_point TimePoint;


// Abstract base class for Behavior Tree Nodes
class TreeNode
{
  private:
    // Node name
    std::string name_;
    NodeStatus status_;
    std::condition_variable state_condition_variable_;
    mutable std::mutex state_mutex_;

  protected:

    // Method to be implemented by the user
    virtual BT::NodeStatus tick() = 0;

  public:
    // The constructor and the destructor
    TreeNode(const std::string& name, const NodeParameters& parameters);
    virtual ~TreeNode() = default;

    // The method that is going to be executed when the node receive a tick
    virtual BT::NodeStatus executeTick();

    // The method used to interrupt the execution of the node
    virtual void halt() = 0;

    bool isHalted() const;

    NodeStatus status() const;
    void setStatus(NodeStatus new_status);

    const std::string& name() const;
    void setName(const std::string& new_name);

    BT::NodeStatus waitValidStatus();

    virtual NodeType type() const = 0;

    using StatusChangeSignal = Signal<TimePoint, const TreeNode&, NodeStatus,NodeStatus>;
    using StatusChangeSubscriber = StatusChangeSignal::Subscriber;
    using StatusChangeCallback   = StatusChangeSignal::CallableFunction;

    /**
     * @brief subscribeToStatusChange is used to attach a callback to a status change.
     * AS soon as StatusChangeSubscriber goes out of scope (it is a shared_ptr) the callback
     * is unsubscribed
     *
     * @param callback. Must have signature void funcname(NodeStatus prev_status, NodeStatus new_status)
     *
     * @return the subscriber.
     */
     StatusChangeSubscriber subscribeToStatusChange(StatusChangeCallback callback);

     // get an unique identifier of this instance of treeNode
     uint16_t UID() const;

     void setRegistrationName(const std::string& registration_name);

     const std::string& registrationName() const;

protected:
     template <typename T> T getParam(const std::string& key) const
     {
         auto it = parameters_.find(key);
         if( it == parameters_.end() )
         {
             throw std::invalid_argument( std::string("Can't find the parameter with key: ") + key );
         }
         return convertFromString<T>( key.c_str() );
     }

private:

  StatusChangeSignal state_change_signal_;

  const uint16_t uid_;

  std::string registration_name_;

  const NodeParameters parameters_;

};

typedef std::shared_ptr<TreeNode> TreeNodePtr;

// The term "Builder" refers to the Builder Pattern (https://en.wikipedia.org/wiki/Builder_pattern)
typedef std::function<std::unique_ptr<TreeNode>(const std::string&, const NodeParameters&)> NodeBuilder;


}

#endif
