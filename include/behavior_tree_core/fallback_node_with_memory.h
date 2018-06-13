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

#ifndef FALLBACK_NODE_WITH_MEMORY_H
#define FALLBACK_NODE_WITH_MEMORY_H

#include "behavior_tree_core/control_node.h"

namespace BT
{
class FallbackNodeWithMemory : public ControlNode
{

public:
    FallbackNodeWithMemory(const std::string& name, ResetPolicy reset_policy = BT::ON_SUCCESS_OR_FAILURE);

    // Reset policy passed by parameter [reset_policy]
    FallbackNodeWithMemory(const std::string&, const NodeParameters& params);

    virtual ~FallbackNodeWithMemory() override = default;

    virtual void halt() override;

    static const NodeParameters& requiredNodeParameters()
    {
        static NodeParameters params = { {RESET_POLICY, toStr(BT::ON_SUCCESS_OR_FAILURE)} };
        return params;
    }

private:
    unsigned int current_child_idx_;
    ResetPolicy reset_policy_;

    constexpr static const char* RESET_POLICY = "reset_policy";
    virtual BT::NodeStatus tick() override;
};


}

#endif   // FALLBACK_NODE_WITH_MEMORY_H
