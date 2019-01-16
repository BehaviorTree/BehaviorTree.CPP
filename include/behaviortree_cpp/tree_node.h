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

#include "behaviortree_cpp/optional.hpp"
#include "behaviortree_cpp/tick_engine.h"
#include "behaviortree_cpp/exceptions.h"
#include "behaviortree_cpp/signal.h"
#include "behaviortree_cpp/basic_types.h"
#include "behaviortree_cpp/blackboard/blackboard.h"

namespace BT
{
// We call Parameters the set of Key/Values that can be read from file and are
// used to parametrize an object. It is up to the user's code to parse the string.
typedef std::unordered_map<std::string, std::string> NodeParameters;

typedef std::chrono::high_resolution_clock::time_point TimePoint;
typedef std::chrono::high_resolution_clock::duration Duration;

// Abstract base class for Behavior Tree Nodes
class TreeNode
{

  private:

    /// This calback will be executed only ONCE after the constructor of the node,
    /// before the very first tick.
    /// Override if necessary.
    virtual void onInit() {}

  public:
    /**
     * @brief TreeNode main constructor.
     *
     * @param name         name of the instance, not the type of sensor.
     * @param parameters   this might be empty. use getParam<T>(key) to parse the value.
     *
     * Note: a node that accepts a not empty set of NodeParameters must also implement the method:
     *
     * static const NodeParameters& requiredNodeParameters();
     */
    TreeNode(const std::string& name, const NodeParameters& parameters);
    virtual ~TreeNode() = default;

    typedef std::shared_ptr<TreeNode> Ptr;

    /// The method that will be executed to invoke tick(); and setStatus();
    virtual BT::NodeStatus executeTick();

    /// The method used to interrupt the execution of a RUNNING node
    virtual void halt() = 0;

    bool isHalted() const;

    NodeStatus status() const;

    void setStatus(NodeStatus new_status);

    void setBlackboard(const Blackboard::Ptr& bb);

    const std::string& name() const;

    /// Blocking function that will sleep until the setStatus() is called with
    /// either RUNNING, FAILURE or SUCCESS.
    BT::NodeStatus waitValidStatus();

    virtual NodeType type() const = 0;

    using StatusChangeSignal = Signal<TimePoint, const TreeNode&, NodeStatus, NodeStatus>;
    using StatusChangeSubscriber = StatusChangeSignal::Subscriber;
    using StatusChangeCallback = StatusChangeSignal::CallableFunction;

    /**
     * @brief subscribeToStatusChange is used to attach a callback to a status change.
     * When StatusChangeSubscriber goes out of scope (it is a shared_ptr) the callback
     * is unsubscribed automatically.
     *
     * @param callback. Must have signature void funcname(NodeStatus prev_status, NodeStatus new_status)
     *
     * @return the subscriber.
     */
    StatusChangeSubscriber subscribeToStatusChange(StatusChangeCallback callback);

    // get an unique identifier of this instance of treeNode
    uint16_t UID() const;

    /// registrationName is the ID used by BehaviorTreeFactory to create an instance.
    const std::string& registrationName() const;

    /// Parameters passed at construction time. Can never change after the
    /// creation of the TreeNode instance.
    const NodeParameters& initializationParameters() const;

    /** Get a parameter from the NodeParameters and convert it to type T.
     */
    template <typename T>
    BT::optional<T> getParam(const std::string& key) const
    {
        T out;
        return getParam(key, out) ? std::move(out) : BT::nullopt;
    }

    /** Get a parameter from the passed NodeParameters and convert it to type T.
     *  Return false either if there is no parameter with this key or if conversion failed.
     */
    template <typename T>
    bool getParam(const std::string& key, T& destination) const;

    static bool isBlackboardPattern(StringView str);

    typedef std::unordered_map<std::string, std::string> PortsRemap;

    const PortsRemap& outputPortsRemap() const&;

    bool setOutputPortRemap(const std::string& original_key, const std::string& remapped_key);

    template <typename T>
    bool setOutput(const std::string& key, const T& value);

    // deprecated because the user should use instead getParam() to read
    // and setOutput() to write
    [[deprecated]] const Blackboard::Ptr& blackboard() const;

protected:
    /// Method to be implemented by the user
    virtual BT::NodeStatus tick() = 0;

    /// registrationName() is set by the BehaviorTreeFactory
    void setRegistrationName(const std::string& registration_name);

    friend class BehaviorTreeFactory;

    void initializeOnce();

  private:

    bool not_initialized_;

    const std::string name_;

    NodeStatus status_;

    std::condition_variable state_condition_variable_;

    mutable std::mutex state_mutex_;

    StatusChangeSignal state_change_signal_;

    const uint16_t uid_;

    std::string registration_name_;

    const NodeParameters parameters_;

    Blackboard::Ptr bb_;

    PortsRemap output_remap_;

};

//-------------------------------------------------------
template <typename T> inline
bool TreeNode::getParam(const std::string& key, T& destination) const
{
    auto it = parameters_.find(key);
    if (it == parameters_.end())
    {
        return false;
    }
    const std::string& str = it->second;

    try
    {
        bool bb_pattern = isBlackboardPattern(str);
        if( bb_pattern && not_initialized_)
        {
             std::cerr << "you are calling getParam inside a constructor, but this is not allowed "
                          "when the parameter contains a blackboard.\n"
                          "You should call getParam inside your tick() method"<< std::endl;
             std::logic_error("Calling getParam inside a constructor");
        }
        // check if it follows this ${pattern}, if it does, search inside the blackboard
        if ( bb_pattern && bb_ )
        {
            const std::string stripped_key(&str[2], str.size() - 3);
            const SafeAny::Any* val = bb_->getAny(stripped_key);
            if( val )
            {
                if( std::is_same<T,std::string>::value == false &&
                    (val->type() == typeid (std::string) ||
                     val->type() == typeid (SafeAny::SimpleString)))
                {
                    destination = convertFromString<T>(val->cast<std::string>());
                }
                else{
                    destination = val->cast<T>();
                }
            }
            return val != nullptr;
        }
        else{
            destination = convertFromString<T>(str.c_str());
            return true;
        }
    }
    catch (std::runtime_error& err)
    {
        std::cout << "Exception at getParam(" << key << "): " << err.what() << std::endl;
        return false;
    }
}

template <typename T> inline
bool TreeNode::setOutput(const std::string& key, const T& value)
{
    if( !bb_ || not_initialized_ )
    {
        return false;
    }
    auto remap_it = output_remap_.find(key);
    const auto& KEY = ( remap_it == output_remap_.end()) ? key : remap_it->second;

    bb_->set(KEY, value);
    return true;
}

}
#endif
