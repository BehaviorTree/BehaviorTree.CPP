#ifndef CONTROLNODE_H
#define CONTROLNODE_H

#include <vector>

#include <tree_node.h>

namespace BT
{
    class ControlNode : public TreeNode
    {
    protected:
        // Children vector
        std::vector<TreeNode*> children_nodes_;

        // Children states
        std::vector<ReturnStatus> children_states_;

        // Vector size
        unsigned int N_of_children_;
        //child i status. Used to rout the ticks
        ReturnStatus child_i_status_;

    public:
        // Constructor
        ControlNode(std::string name);
        ~ControlNode();

        // The method used to fill the child vector
        void AddChild(TreeNode* child);

        // The method used to know the number of children
        unsigned int GetChildrenNumber();
	std::vector<TreeNode*> GetChildren();
        // The method used to interrupt the execution of the node
        void Halt();
        void ResetColorState();
        void HaltChildren(int i);
        int Depth();

        // Methods used to access the node state without the
        // conditional waiting (only mutual access)
        bool WriteState(ReturnStatus new_state);
    };
}

#endif
