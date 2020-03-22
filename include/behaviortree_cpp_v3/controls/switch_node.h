/* Copyright (C) 2020-2020 Davide Faconti -  All Rights Reserved
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

#ifndef SWITCH_NODE_H
#define SWITCH_NODE_H

#include "behaviortree_cpp_v3/control_node.h"

namespace BT
{
/**
 * @brief The SwitchNode
 *
 */
template <size_t NUM_CASES>
class SwitchNode : public ControlNode
{
  public:
    SwitchNode(const std::string& name, const BT::NodeConfiguration& config)
    : ControlNode::ControlNode(name, config ),
      running_child_(-1)
    {
        setRegistrationID("Switch");
    }

    virtual ~SwitchNode() override = default;

    void halt() override
    {
        running_child_ = -1;
        ControlNode::halt();
    }

    static PortsList providedPorts()
    {
        PortsList ports;
        ports.insert( BT::InputPort<std::string>("variable") );
        for(unsigned i=0; i < NUM_CASES; i++)
        {
            char case_str[20];
            sprintf(case_str, "case_%d", i+1);
            ports.insert( BT::InputPort<std::string>(case_str) );
        }
        return ports;
    }

  private:
    int running_child_;
    virtual BT::NodeStatus tick() override;
};

template<size_t NUM_CASES> inline
NodeStatus SwitchNode<NUM_CASES>::tick()
{
    constexpr const char * case_port_names[9] = {
      "case_1", "case_2", "case_3", "case_4", "case_5", "case_6", "case_7", "case_8", "case_9"};

    if( childrenCount() != NUM_CASES+1)
    {
        throw LogicError("Wrong number of children in SwitchNode; "
                         "must be (num_cases + default)");
    }

    std::string variable;
    std::string value;
    int child_index = NUM_CASES; // default index;

    if (getInput("variable", variable)) // no variable? jump to default
    {
        // check each case until you find a match
        for (unsigned index = 0; index < NUM_CASES; ++index)
        {
            bool found = false;
            if( index < 9 )
            {
                found = (bool)getInput(case_port_names[index], value);
            }
            else{
                char case_str[20];
                sprintf(case_str, "case_%d", index+1);
                found = (bool)getInput(case_str, value);
            }

            if (found && variable == value)
            {
                child_index = index;
                break;
            }
        }
    }

    // if another one was running earlier, halt it
    if( running_child_ != -1 && running_child_ != child_index)
    {
        haltChild(running_child_);
    }

    auto& selected_child = children_nodes_[child_index];
    NodeStatus ret = selected_child->executeTick();
    if( ret == NodeStatus::RUNNING )
    {
        running_child_ = child_index;
    }
    else{
        haltChildren();
        running_child_ = -1;
    }
    return ret;
}

}

#endif // SWITCH_NODE_H
