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

#ifndef BEHAVIORTREECORE_ACTIONNODE_H
#define BEHAVIORTREECORE_ACTIONNODE_H

#include <atomic>
#include "leaf_node.h"

namespace BT
{
class ActionNode : public LeafNode
{
  public:
    // Constructor
    ActionNode(std::string name);
    ~ActionNode() = default;

    // The method that is going to be executed by the thread
    void waitForTick();

    // This method triggers the TickEngine. Do NOT remove the "final" keyword.
    virtual NodeStatus tick() override final;

    // method to be implemented by the user.
    virtual NodeStatus asyncTick() = 0;

    virtual NodeType type() const override final
    {
        return ACTION_NODE;
    }

    void stopAndJoinThread();

  protected:
    // The thread that will execute the node
    std::thread thread_;

    // Node semaphore to simulate the tick
    // (and to synchronize fathers and children)
    TickEngine tick_engine_;

    std::atomic<bool> loop_;
};
}

#endif
