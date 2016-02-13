#ifndef CONDITIONTEST_H
#define CONDITIONTEST_H

#include <ConditionNode.h>

namespace BTExample
{
    class ConditionNodeExample : public BT::ConditionNode
    {
		public:
			// Constructor
			ConditionNodeExample(std::string Name);
			~ConditionNodeExample();
			void SetBehavior(BT::NodeState status);

			// The method that is going to be executed by the thread
			void Exec();
		private:
			BT::NodeState status;
    };
}

#endif
