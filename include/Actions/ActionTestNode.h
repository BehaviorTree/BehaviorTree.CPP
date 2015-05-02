#ifndef ACTIONTEST_H
#define ACTIONTEST_H

#include <ActionNode.h>

namespace BT
{
    class ActionTestNode : public ActionNode
    {
    public:
        NodeState status;
        int seconds;
        // Constructor
        ActionTestNode(std::string Name);
        ~ActionTestNode();

        // The method that is going to be executed by the thread
        void Exec();
        void SetBehavior(NodeState status);

        // The method used to interrupt the execution of the node
        bool Halt();
        void SetTime(int seconds);

	

    };
}

#endif
