#include "behavior_tree_core/bt_factory.h"

using namespace BT;

namespace CrossDoor
{

BT::NodeStatus IsDoorOpen(const Blackboard::Ptr& blackboard)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(500) );
    bool door_open = blackboard->get<bool>("door_open");

    return  door_open ? NodeStatus::SUCCESS : NodeStatus::FAILURE;
}

BT::NodeStatus IsDoorLocked(const Blackboard::Ptr& blackboard)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(500) );
    bool door_locked = blackboard->get<bool>("door_locked");

    return door_locked ? NodeStatus::SUCCESS : NodeStatus::FAILURE;
}

BT::NodeStatus UnlockDoor(const Blackboard::Ptr& blackboard)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    blackboard->set("door_locked", false);

    return NodeStatus::SUCCESS;
}

BT::NodeStatus PassThroughDoor(const Blackboard::Ptr& blackboard)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    bool door_open = blackboard->get<bool>("door_open");

    return door_open ? NodeStatus::SUCCESS : NodeStatus::FAILURE;
}

BT::NodeStatus PassThroughWindow(const Blackboard::Ptr& blackboard)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    return NodeStatus::SUCCESS;
}

BT::NodeStatus OpenDoor(const Blackboard::Ptr& blackboard)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    bool door_locked = blackboard->get<bool>("door_locked");

    if (door_locked)
    {
        return NodeStatus::FAILURE;
    }

    blackboard->set("door_open", true);
    return NodeStatus::SUCCESS;
}

BT::NodeStatus CloseDoor(const Blackboard::Ptr& blackboard)
{
    bool door_open = blackboard->get<bool>("door_open");

    if (door_open)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1500));
        blackboard->set("door_open", false);
    }
    return NodeStatus::SUCCESS;
}

void RegisterNodes(BT::BehaviorTreeFactory& factory)
{
    factory.registerSimpleCondition("IsDoorOpen",   IsDoorOpen );
    factory.registerSimpleAction("PassThroughDoor", PassThroughDoor );
    factory.registerSimpleAction("PassThroughWindow", PassThroughWindow );
    factory.registerSimpleAction("OpenDoor", OpenDoor );
    factory.registerSimpleAction("CloseDoor", CloseDoor );
    factory.registerSimpleCondition("IsDoorLocked", IsDoorLocked );
    factory.registerSimpleAction("UnlockDoor", UnlockDoor );
}

}
