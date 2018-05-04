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

#include "behavior_tree_core/decorator_negation_node.h"

BT::DecoratorNegationNode::DecoratorNegationNode(std::string name) : DecoratorNode(name)
{
}

BT::NodeStatus BT::DecoratorNegationNode::tick()
{
    setStatus(BT::RUNNING);
    const NodeStatus child_state = child_node_->executeTick();

    switch (child_state)
    {
        case BT::SUCCESS:
        {
            setStatus(BT::FAILURE);
        }
        break;

        case BT::FAILURE:
        {
            setStatus(BT::SUCCESS);
        }
        break;

        case BT::RUNNING:
        {
            setStatus(BT::RUNNING);
        }
        break;

        default:
        {
            // TODO throw?
        }
    }
    return status();
}
