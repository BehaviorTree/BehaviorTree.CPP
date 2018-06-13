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

#ifndef BEHAVIOR_TREE_H
#define BEHAVIOR_TREE_H

#include "behavior_tree_core/parallel_node.h"
#include "behavior_tree_core/fallback_node.h"
#include "behavior_tree_core/sequence_node.h"

#include "behavior_tree_core/sequence_node_with_memory.h"
#include "behavior_tree_core/fallback_node_with_memory.h"

#include "behavior_tree_core/action_node.h"
#include "behavior_tree_core/condition_node.h"

#include "behavior_tree_core/decorator_negation_node.h"
#include "behavior_tree_core/decorator_retry_node.h"
#include "behavior_tree_core/decorator_repeat_node.h"
#include "behavior_tree_core/decorator_subtree_node.h"

namespace BT
{
void recursiveVisitor(TreeNode* node, std::function<void(TreeNode*)> visitor);

/**
 * Debug function to print on a stream
 */
void printTreeRecursively(const TreeNode* root_node);

/**
 * @brief buildSerializedStatusSnapshot can be used to create a serialize buffer that can be stored
 * (or sent to a client application) to know the status of all the nodes of a tree.
 * It is not "human readable".
 *
 * @param root_node
 * @param serialized_buffer
 */

typedef std::vector<std::pair<uint16_t, uint8_t>> SerializedTreeStatus;

void buildSerializedStatusSnapshot(const TreeNode* root_node, SerializedTreeStatus& serialized_buffer);
}

#endif   // BEHAVIOR_TREE_H
