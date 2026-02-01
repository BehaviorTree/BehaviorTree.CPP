#include "crossdoor_nodes.h"

inline void SleepMS(int ms)
{
  std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

using BT::NodeStatus;

NodeStatus CrossDoor::isDoorClosed()
{
  SleepMS(200);
  return !_door_open ? NodeStatus::SUCCESS : NodeStatus::FAILURE;
}

NodeStatus CrossDoor::passThroughDoor()
{
  SleepMS(500);
  return _door_open ? NodeStatus::SUCCESS : NodeStatus::FAILURE;
}

NodeStatus CrossDoor::openDoor()
{
  SleepMS(500);
  if(_door_locked)
  {
    return NodeStatus::FAILURE;
  }
  else
  {
    _door_open = true;
    return NodeStatus::SUCCESS;
  }
}

NodeStatus CrossDoor::pickLock()
{
  SleepMS(500);
  // succeed at 3rd attempt
  if(_pick_attempts++ > 3)
  {
    _door_locked = false;
    _door_open = true;
  }
  return _door_open ? NodeStatus::SUCCESS : NodeStatus::FAILURE;
}

NodeStatus CrossDoor::smashDoor()
{
  _door_locked = false;
  _door_open = true;
  // smash always works
  return NodeStatus::SUCCESS;
}

void CrossDoor::registerNodes(BT::BehaviorTreeFactory& factory)
{
  factory.registerSimpleCondition("IsDoorClosed",
                                  std::bind(&CrossDoor::isDoorClosed, this));

  factory.registerSimpleAction("PassThroughDoor",
                               std::bind(&CrossDoor::passThroughDoor, this));

  factory.registerSimpleAction("OpenDoor", std::bind(&CrossDoor::openDoor, this));

  factory.registerSimpleAction("PickLock", std::bind(&CrossDoor::pickLock, this));

  factory.registerSimpleCondition("SmashDoor", std::bind(&CrossDoor::smashDoor, this));
}

void CrossDoor::reset()
{
  _door_open = false;
  _door_locked = true;
  _pick_attempts = 0;
}

// This function must be implemented in the .cpp file to create
// a plugin that can be loaded at run-time
BT_REGISTER_NODES(factory)
{
  static CrossDoor cross_door;
  cross_door.registerNodes(factory);
}
