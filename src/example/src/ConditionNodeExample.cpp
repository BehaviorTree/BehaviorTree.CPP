#include <ConditionNodeExample.h>
#include <iostream>

using namespace BTExample;

ConditionNodeExample::ConditionNodeExample(std::string Name) :
		BT::ConditionNode::ConditionNode(Name) {
	Type = BT::Condition;
	status = BT::Success;
	// Thread start
	Thread = boost::thread(&ConditionNodeExample::Exec, this);
}

ConditionNodeExample::~ConditionNodeExample() {
}

void ConditionNodeExample::Exec() {
	int i = 0;
	while (true) {

		// Waiting for a tick to come
		Semaphore.Wait();

		if (ReadState() == BT::Exit) {
			// The behavior tree is going to be destroied
			return;
		}

		// Condition checking and state update
		i++;
		if (i < 5) {
			SetNodeState(BT::Success);
			std::cout << Name << " returning Success" << BT::Success << "!"
					<< std::endl;
		} else if (i < 10) {
			SetNodeState (BT::Failure);
			std::cout << Name << " returning Failure" << BT::Failure << "!"
					<< std::endl;
		} else {
			std::cout << Name << " reset i!" << std::endl;
			SetNodeState (BT::Failure);
			i = 0;
		}

		// Resetting the state
		WriteState(BT::Idle);
	}
}

void ConditionNodeExample::SetBehavior(BT::NodeState status) {
	this->status = status;
}
