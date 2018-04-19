#ifndef LEAFNODE_H
#define LEAFNODE_H

#include <vector>

#include "behavior_tree_core/tree_node.h"

namespace BT
{
    class LeafNode : public TreeNode
    {
    protected:
    public:
        LeafNode(std::string name);
        ~LeafNode() = default;

    };
}

#endif
