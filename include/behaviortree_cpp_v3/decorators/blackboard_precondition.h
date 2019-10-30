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

#include "behaviortree_cpp_v3/decorator_node.h"

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
 * <BlackboardCheckInt value_A="{the_answer}"
 *                     value_B="42"
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

    static PortsList providedPorts()
    {
        return {InputPort("value_A"),
                InputPort("value_B"),
                InputPort<NodeStatus>("return_on_mismatch") };
    }

  private:
    virtual BT::NodeStatus tick() override;
};

//----------------------------------------------------

template<typename T> inline
NodeStatus BlackboardPreconditionNode<T>::tick()
{
    T value_A;
    T value_B;
    NodeStatus default_return_status = NodeStatus::FAILURE;

    setStatus(NodeStatus::RUNNING);

    if( getInput("value_A", value_A) &&
        getInput("value_B", value_B) &&
        value_B == value_A )
    {
        return child_node_->executeTick();
    }

    if( child()->status() == NodeStatus::RUNNING )
    {
        haltChild();
    }
    getInput("return_on_mismatch", default_return_status);
    return default_return_status;
}

} // end namespace

#endif
