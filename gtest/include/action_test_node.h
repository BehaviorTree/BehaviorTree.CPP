#ifndef ACTIONTEST_H
#define ACTIONTEST_H

#include <action_node.h>

namespace BT
{
    class ActionTestNode : public ActionNode
    {

    public:
        // Constructor
        ActionTestNode(std::string Name);
        ~ActionTestNode();

        // The method that is going to be executed by the thread
        BT::ReturnStatus Tick();
        void set_time(int time);
	
        // The method used to interrupt the execution of the node
        void Halt();
        void set_boolean_value(bool boolean_value);
    private:
        int time_;
        bool boolean_value_;

        ///ReturnStatus status_;

    };
}

#endif
