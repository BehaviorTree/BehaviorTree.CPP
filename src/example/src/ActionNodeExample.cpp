#include <ActionNodeExample.h>
#include <iostream>

using namespace BTExample;

ActionNodeExample::ActionNodeExample(std::string Name) :
		ActionNode::ActionNode(Name) {
	Type = BT::Action;
	status = BT::Failure;
	time = 1;
	// Thread start
	Thread = boost::thread(&ActionNodeExample::Exec, this);
}

ActionNodeExample::~ActionNodeExample() {
}

void ActionNodeExample::Exec() {

	while (true) {

		// Waiting for a tick to come
		Semaphore.Wait();

		if (ReadState() == BT::Exit) {    //SetColorState(Idle);

			// The behavior tree is going to be destroied
			return;
		}

		// Running state
		SetNodeState(BT::Running);
		std::cout << Name << " returning " << BT::Running << "!" << std::endl;

		// Perform action...
		/*
		 * HERE THE CODE TO EXECUTE AS LONG AS
		 * THE BEHAVIOR TREE DOES NOT HALT THE ACTION
		 */
		int i = 0;
		while (ReadState() == BT::Running and i++ < time) {


			std::cout << Name << " working!" << std::endl;
			boost::this_thread::sleep(boost::posix_time::milliseconds(1000));
			//If the action Succeeded
			//	setStatus(BT::Success);
			//If the action Failed
			// setStatus(BT::Failure);
			if (i == time-1){
				setStatus(BT::Success);
			}
		}

		/* == END OF ACTION CODE == */

		if (ReadState() == BT::Exit) {
			// The behavior tree is going to be destroyed
			return;
		} else {
			// trying to set the outcome state:
			if (WriteState(status) != true) {
				// meanwhile, my father halted me!
				std::cout << Name << " Halted!" << std::endl;

				// Resetting the state
				WriteState(BT::Idle);

				// Next loop
				continue;
			}

			std::cout << Name << " returning " << status << "!" << std::endl;
		}

		// Synchronization
		// (my father is telling me that it has read my new state)
		Semaphore.Wait();

		if (ReadState() == BT::Exit) {

			// The behavior tree is going to be destroyed
			return;
		}

		// Resetting the state
		WriteState(BT::Idle);
	}
}

bool ActionNodeExample::Halt() {
	// Lock acquisition
	boost::lock_guard<boost::mutex> LockGuard(StateMutex);

	// Checking for "Running" correctness
	if (State != BT::Running) {
		return false;
	}

	State = BT::Halted;
	return true;
}

void ActionNodeExample::setStatus(BT::NodeState status) {
	this->status = status;
}

void ActionNodeExample::SetTime(int time) {
	this->time = time;
}
