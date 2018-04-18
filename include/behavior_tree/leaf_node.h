#ifndef LEAFNODE_H
#define LEAFNODE_H

#include <vector>

#include "behavior_tree/tree_node.h"

namespace BT
{
    class LeafNode : public TreeNode
    {
    protected:
    public:
        LeafNode(std::string name);
        ~LeafNode();
    void ResetColorState();
    int Depth();
    };
}

#endif
