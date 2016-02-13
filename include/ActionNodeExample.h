#ifndef ACTIONTEST_H
#define ACTIONTEST_H

#include <ActionNode.h>

namespace BTExample
{
    class ActionNodeExample : public BT::ActionNode
    {
		public:
			BT::NodeState status;
			int time;
			// Constructor
			ActionNodeExample(std::string Name);
			virtual ~ActionNodeExample();

			// The method that is going to be executed by the thread
			void Exec();
			void SetBehavior(BT::NodeState status);

			// The method used to interrupt the execution of the node
			bool Halt();
			void SetTime(int seconds);
			void setStatus(BT::NodeState status);
    };
}

#endif
