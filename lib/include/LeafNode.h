#ifndef LEAFNODE_H
#define LEAFNODE_H

#include <vector>

#include <TreeNode.h>

namespace BT
{
    class LeafNode : public TreeNode
    {
    protected:
    public:
        LeafNode(std::string Name);
        ~LeafNode();
    void ResetColorState();
    int GetDepth();
    };
}

#endif
