#ifndef LEAFNODE_H
#define LEAFNODE_H

#include <vector>

#include <tree_node.h>

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
