/*  Copyright (C) 2018-2019 Davide Faconti, Eurecat -  All Rights Reserved
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

#ifndef DECORATOR_BLACKBOARD_PRECONDITION_NODE_H
#define DECORATOR_BLACKBOARD_PRECONDITION_NODE_H

#include "behaviortree_cpp/decorator_node.h"

namespace BT
{
/**
 * This node excute its child only if the value of a given input port
 * is equal to the expected one.
 * If this precondition is met, this node will return the same status of the
 * child, otherwise it will return the value specified in "return_on_mismatch".
 *
 * Example:
 *
 * <BlackboardCheckInt key="${the_answer}"
 *                     expected="42"
 *                     return_on_mismatch="FAILURE" />
 */
template <typename T>
class BlackboardPreconditionNode : public DecoratorNode
{
  public:
    BlackboardPreconditionNode(const std::string& name, const NodeConfiguration& config)
      : DecoratorNode(name, config)
    {
        if( std::is_same<T,int>::value)
            setRegistrationID("BlackboardCheckInt");
        else if( std::is_same<T,double>::value)
            setRegistrationID("BlackboardCheckDouble");
        else if( std::is_same<T,std::string>::value)
            setRegistrationID("BlackboardCheckString");
    }

    virtual ~BlackboardPreconditionNode() override = default;

    static const PortsList& providedPorts()
    {
        static PortsList ports = {{"key", PortType::INPUT},
                                  {"expected", PortType::INPUT},
                                  {"return_on_mismatch", PortType::INPUT}};
        return ports;
    }

  private:
    virtual BT::NodeStatus tick() override;
};

//----------------------------------------------------

template<typename T> inline
NodeStatus BlackboardPreconditionNode<T>::tick()
{
    T expected_value;
    T current_value;
    NodeStatus default_return_status = NodeStatus::FAILURE;

    setStatus(NodeStatus::RUNNING);

    if( !getInput("key", current_value) ||
        !getInput("expected", expected_value) ||
        current_value != expected_value )
    {
        getInput("return_on_mismatch", default_return_status);
        return default_return_status;
    }
    auto child_status = child_node_->executeTick();
    if( child_status != NodeStatus::RUNNING )
    {
        haltChild();
    }
    return child_status;
}

}

#endif
