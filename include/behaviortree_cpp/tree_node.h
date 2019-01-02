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

#include "behaviortree_cpp/optional.hpp"
#include "behaviortree_cpp/tick_engine.h"
#include "behaviortree_cpp/exceptions.h"
#include "behaviortree_cpp/signal.h"
#include "behaviortree_cpp/basic_types.h"
#include "behaviortree_cpp/blackboard/blackboard.h"

namespace BT
{

/// This information is used mostly by the XMLParser.
struct TreeNodeManifest
{
    NodeType type;
    std::string registration_ID;
    PortsList ports;
};

struct NodeConfiguration
{
    NodeConfiguration() {}

    // initialize with no remapping and no blackboard
    NodeConfiguration(StringView ID)
    {
        registration_ID = ID.to_string();
    }

    Blackboard::Ptr blackboard;
    std::string     registration_ID;
    PortsRemapping  input_ports;
    PortsRemapping  output_ports;
};

// Abstract base class for Behavior Tree Nodes
class TreeNode
{
  public:
    /**
     * @brief TreeNode main constructor.
     *
     * @param name     name of the instance, not the type of sensor.
     * @param config
     *
     * Note: a node that accepts a not empty set of NodeParameters must also implement the method:
     *
     * static const PortsList& providedPorts();
     */
    TreeNode(const std::string& name, const NodeConfiguration& config); 

    virtual ~TreeNode() = default;

    typedef std::shared_ptr<TreeNode> Ptr;

    /// The method that will be executed to invoke tick(); and setStatus();
    virtual BT::NodeStatus executeTick();

    /// The method used to interrupt the execution of a RUNNING node
    virtual void halt() = 0;

    bool isHalted() const;

    NodeStatus status() const;

    void setStatus(NodeStatus new_status);

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
    const NodeConfiguration& config() const;

    /** Get a parameter from the NodeParameters and convert it to type T.
     */
    template <typename T>
    BT::optional<T> getInput(const std::string& key) const
    {
        T out;
        return getInput(key, out) ? BT::optional<T>(std::move(out)) : BT::nullopt;
    }

    /** Read an Input Port and convert it to type T.
     *  Return false either if there is no parameter with this key or if conversion failed.
     */
    template <typename T>
    bool getInput(const std::string& key, T& destination) const;

    static bool isBlackboardPointer(StringView str);

    template <typename T>
    bool setOutput(const std::string& key, const T& value);

protected:
    /// Method to be implemented by the user
    virtual BT::NodeStatus tick() = 0;

    friend class BehaviorTreeFactory;

  private:

    const std::string name_;

    NodeStatus status_;

    std::condition_variable state_condition_variable_;

    mutable std::mutex state_mutex_;

    StatusChangeSignal state_change_signal_;

    const uint16_t uid_;

    const NodeConfiguration config_;

};

//-------------------------------------------------------
template <typename T> inline
bool TreeNode::getInput(const std::string& key, T& destination) const
{
    auto remap_it = config_.input_ports.find(key);
    if( remap_it == config_.input_ports.end() )
    {
        std::cerr << "getInput() failed because NodeConfiguration::input_ports "
                  << "does not contain the key: [" << key << "]" << std::endl;
        return false;
    }
    StringView remapped_key = remap_it->second;
    if( remapped_key == "=")
    {
        remapped_key = key;
    }
    try
    {
        if( !isBlackboardPointer(remapped_key))
        {
            destination = convertFromString<T>(remapped_key);
            return true;
        }

        if ( !config_.blackboard )
        {
            std::cerr << "getInput() trying to access a Blackboard(BB) entry, but BB is invalid"
                      << std::endl;
            return false;
        }

        remapped_key = remapped_key.substr( 2, remapped_key.size()-3 );

        const SafeAny::Any* val = config_.blackboard->getAny( remapped_key.to_string() );
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
            return true;
        }

        std::cerr << "getInput() failed because it was unable to find the key ["
                  << key << "] remapped to [" << remapped_key << "]" << std::endl;
        return false;
    }
    catch (std::runtime_error& err)
    {
        std::cerr << "Exception at getInput(" << key << "): " << err.what() << std::endl;
        return false;
    }
}

template <typename T> inline
bool TreeNode::setOutput(const std::string& key, const T& value)
{
    if ( !config_.blackboard )
    {
        std::cerr << "setOutput() trying to access a Blackboard(BB) entry, but BB is invalid"
                  << std::endl;
        return false;
    }

    auto remap_it = config_.output_ports.find(key);
    if( remap_it == config_.output_ports.end() )
    {
        std::cerr << "setOutput() failed because NodeConfiguration::output_ports "
                  << "does not contain the key: [" << key << "]" << std::endl;
        return false;
    }
    StringView remapped_key = remap_it->second;
    if( remapped_key == "=")
    {
        remapped_key = key;
    }
    if( isBlackboardPointer(remapped_key) )
    {
        remapped_key = remapped_key.substr( 2, remapped_key.size() -3 );
    }

    config_.blackboard->set( remapped_key.to_string(), value);
    return true;
}

}
#endif
