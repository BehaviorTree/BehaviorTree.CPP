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

#ifndef SEQUENCENODE_H
#define SEQUENCENODE_H

#include "behavior_tree_core/control_node.h"

namespace BT
{
/**
 * @brief The SequenceNode is used to execute a sequence of synchronous children.
 *
 * This control node ticks its children AS LONG AS they returns SUCCESS.
 *
 * If all the children return SUCCESS, the sequence is SUCCESS.
 * If any return FAILURE, the sequence returns FAILURE and it starts from the beginning.
 * If a child returns RUNNING, this node returns RUNNING and at the next tick it will continue
 * from the same index.
 * It is recommended for asynchronous children which may return RUNNING.
 *
 * Example: three children, A , B and C
 *
 * 1) A returns SUCCESS. Continue.
 * 2) B returns RUNNING. Stop and return RUNNING.
 * 3) A is ticked again and return SUCCESS. B is ticked and retuns SUCCESS. Continue.
 * 4) C returns SUCCESS. The entire sequence returns SUCCESS.
 */
class SequenceNode : public ControlNode
{
  public:
    SequenceNode(const std::string& name);

    virtual ~SequenceNode() override = default;

  private:
    virtual BT::NodeStatus tick() override;
};
}

#endif
