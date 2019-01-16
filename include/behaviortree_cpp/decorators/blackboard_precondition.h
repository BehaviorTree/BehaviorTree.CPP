/*  Copyright (C) 2018 Davide Faconti -  All Rights Reserved
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
template <typename T>
class BlackboardPreconditionNode : public DecoratorNode
{
  public:
    BlackboardPreconditionNode(const std::string& name, const NodeParameters& params)
      : DecoratorNode(name, params)
    {
        if( std::is_same<T,int>::value)
            setRegistrationName("BlackboardCheckInt");
        else if( std::is_same<T,double>::value)
            setRegistrationName("BlackboardCheckDouble");
        else if( std::is_same<T,std::string>::value)
            setRegistrationName("BlackboardCheckString");
    }

    virtual ~BlackboardPreconditionNode() override = default;

    static const NodeParameters& requiredNodeParameters()
    {
        static NodeParameters params = {{"current", "${BB_key}"}, {"expected", "*"}};
        return params;
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

    setStatus(NodeStatus::RUNNING);

    if( !getParam("current", current_value) ||
        !getParam("expected", expected_value) ||
        current_value != expected_value )
    {
        return NodeStatus::FAILURE;
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
