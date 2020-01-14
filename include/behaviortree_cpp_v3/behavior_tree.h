/* Copyright (C) 2015-2018 Michele Colledanchise -  All Rights Reserved
 * Copyright (C) 2018-2019 Davide Faconti, Eurecat -  All Rights Reserved
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

#ifndef BEHAVIOR_TREE_H
#define BEHAVIOR_TREE_H

#include "behaviortree_cpp_v3/controls/parallel_node.h"
#include "behaviortree_cpp_v3/controls/reactive_sequence.h"
#include "behaviortree_cpp_v3/controls/reactive_fallback.h"
#include "behaviortree_cpp_v3/controls/fallback_node.h"
#include "behaviortree_cpp_v3/controls/sequence_node.h"
#include "behaviortree_cpp_v3/controls/sequence_star_node.h"

#include "behaviortree_cpp_v3/action_node.h"
#include "behaviortree_cpp_v3/condition_node.h"

#include "behaviortree_cpp_v3/decorators/inverter_node.h"
#include "behaviortree_cpp_v3/decorators/retry_node.h"
#include "behaviortree_cpp_v3/decorators/repeat_node.h"
#include "behaviortree_cpp_v3/decorators/subtree_node.h"

#include "behaviortree_cpp_v3/actions/always_success_node.h"
#include "behaviortree_cpp_v3/actions/always_failure_node.h"
#include "behaviortree_cpp_v3/actions/set_blackboard_node.h"

#include "behaviortree_cpp_v3/decorators/force_success_node.h"
#include "behaviortree_cpp_v3/decorators/force_failure_node.h"
#include "behaviortree_cpp_v3/decorators/blackboard_precondition.h"
#include "behaviortree_cpp_v3/decorators/timeout_node.h"

namespace BT
{

//Call the visitor for each node of the tree, given a root.
void applyRecursiveVisitor(const TreeNode* root_node,
                           const std::function<void(const TreeNode*)>& visitor);

//Call the visitor for each node of the tree, given a root.
void applyRecursiveVisitor(TreeNode* root_node, const std::function<void(TreeNode*)>& visitor);

/**
 * Debug function to print on screen the hierarchy of the tree.
 */
void printTreeRecursively(const TreeNode* root_node);

/// Invoke AsyncActionNode::stopAndJoinThread() to the entire tree,
/// when needed.
void haltAllActions(TreeNode* root_node);


typedef std::vector<std::pair<uint16_t, uint8_t>> SerializedTreeStatus;

/**
 * @brief buildSerializedStatusSnapshot can be used to create a buffer that can be stored
 * (or sent to a client application) to know the status of all the nodes of a tree.
 * It is not "human readable".
 *
 * @param root_node
 * @param serialized_buffer is the output.
 */
void buildSerializedStatusSnapshot(const TreeNode* root_node,
                                   SerializedTreeStatus& serialized_buffer);

/// Simple way to extract the type of a TreeNode at COMPILE TIME.
/// Useful to avoid the cost of without dynamic_cast or the virtual method TreeNode::type().
template <typename T>
inline NodeType getType()
{
    // clang-format off
    if( std::is_base_of<ActionNodeBase, T>::value )        return NodeType::ACTION;
    if( std::is_base_of<ConditionNode, T>::value )         return NodeType::CONDITION;
    if( std::is_base_of<DecoratorSubtreeNode, T>::value )  return NodeType::SUBTREE;
    if( std::is_base_of<DecoratorNode, T>::value )         return NodeType::DECORATOR;
    if( std::is_base_of<ControlNode, T>::value )           return NodeType::CONTROL;
    return NodeType::UNDEFINED;
    // clang-format on
}
}

#endif   // BEHAVIOR_TREE_H
