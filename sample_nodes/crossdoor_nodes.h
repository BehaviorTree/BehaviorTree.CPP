#include "behavior_tree_core/bt_factory.h"

using namespace BT;

namespace CrossDoor
{

inline void SleepMS(int ms)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

BT::NodeStatus IsDoorOpen(const Blackboard::Ptr& blackboard);

BT::NodeStatus IsDoorLocked(const Blackboard::Ptr& blackboard);

BT::NodeStatus UnlockDoor(const Blackboard::Ptr& blackboard);

BT::NodeStatus PassThroughDoor(const Blackboard::Ptr& blackboard);

BT::NodeStatus PassThroughWindow(const Blackboard::Ptr& blackboard);

BT::NodeStatus OpenDoor(const Blackboard::Ptr& blackboard);

BT::NodeStatus CloseDoor(const Blackboard::Ptr& blackboard);

void RegisterNodes(BT::BehaviorTreeFactory& factory);

}


