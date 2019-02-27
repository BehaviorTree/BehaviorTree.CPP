#include "crossdoor_nodes.h"

// This function must be implemented in the .cpp file to create
// a plugin that can be loaded at run-time
BT_REGISTER_NODES(factory)
{
    CrossDoor::RegisterNodes(factory);
}

// For simplicity, in this example the status of the door is not shared
// using ports and blackboards
static bool _door_open   = false;
static bool _door_locked = true;

NodeStatus CrossDoor::IsDoorOpen()
{
    SleepMS(500);
    return _door_open ? NodeStatus::SUCCESS : NodeStatus::FAILURE;
}

NodeStatus CrossDoor::IsDoorLocked()
{
    SleepMS(500);
    return _door_locked ? NodeStatus::SUCCESS : NodeStatus::FAILURE;
}

NodeStatus CrossDoor::UnlockDoor()
{
    SleepMS(2000);
    _door_locked = false;
    return NodeStatus::SUCCESS;
}

NodeStatus CrossDoor::PassThroughDoor()
{
    SleepMS(1000);
    return _door_open ? NodeStatus::SUCCESS : NodeStatus::FAILURE;
}

NodeStatus CrossDoor::PassThroughWindow()
{
    SleepMS(1000);
    return NodeStatus::SUCCESS;
}

NodeStatus CrossDoor::OpenDoor()
{
    if (_door_locked)
    {
        SleepMS(2000);
        _door_open = true;
    }

    return NodeStatus::SUCCESS;
}

NodeStatus CrossDoor::CloseDoor()
{
    if (_door_open)
    {
        SleepMS(1500);
        _door_open = false;
    }
    return NodeStatus::SUCCESS;
}

// Register at once all the Actions and Conditions in this file
void CrossDoor::RegisterNodes(BehaviorTreeFactory& factory)
{
    factory.registerSimpleCondition("IsDoorOpen", std::bind(IsDoorOpen));
    factory.registerSimpleAction("PassThroughDoor", std::bind(PassThroughDoor));
    factory.registerSimpleAction("PassThroughWindow", std::bind(PassThroughWindow));
    factory.registerSimpleAction("OpenDoor", std::bind(OpenDoor));
    factory.registerSimpleAction("CloseDoor", std::bind(CloseDoor));
    factory.registerSimpleCondition("IsDoorLocked", std::bind(IsDoorLocked));
    factory.registerSimpleAction("UnlockDoor", std::bind(UnlockDoor));
}
