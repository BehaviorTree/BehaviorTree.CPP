#include "behavior_tree_core/bt_factory.h"

using namespace BT;

class CrossDoor
{
  public:
    CrossDoor(BT::BehaviorTreeFactory& factory)
    {
        door_open_ = true;
        door_locked_ = false;

        factory.registerSimpleAction("IsDoorOpen", [this]() { return IsDoorOpen(); });
        factory.registerSimpleAction("PassThroughDoor", [this]() { return PassThroughDoor(); });
        factory.registerSimpleAction("PassThroughWindow", [this]() { return PassThroughWindow(); });
        factory.registerSimpleAction("OpenDoor", [this]() { return OpenDoor(); });
        factory.registerSimpleAction("CloseDoor", [this]() { return CloseDoor(); });
        factory.registerSimpleAction("IsDoorLocked", [this]() { return IsDoorLocked(); });
        factory.registerSimpleAction("UnlockDoor", [this]() { return UnlockDoor(); });
    }

    BT::NodeStatus IsDoorOpen()
    {
        return door_open_ ? NodeStatus::SUCCESS : NodeStatus::FAILURE;
    }

    BT::NodeStatus IsDoorLocked()
    {
        return door_locked_ ? NodeStatus::SUCCESS : NodeStatus::FAILURE;
    }

    BT::NodeStatus UnlockDoor()
    {
        door_locked_ = false;
        return NodeStatus::SUCCESS;
    }

    BT::NodeStatus PassThroughDoor()
    {
        return door_open_ ? NodeStatus::SUCCESS : NodeStatus::FAILURE;
    }

    BT::NodeStatus PassThroughWindow()
    {
        return NodeStatus::SUCCESS;
    }

    BT::NodeStatus OpenDoor()
    {
        if (door_locked_)
            return NodeStatus::FAILURE;
        door_open_ = true;
        return NodeStatus::SUCCESS;
    }

    BT::NodeStatus CloseDoor()
    {
        if (door_open_)
        {
            door_open_ = false;
            return NodeStatus::SUCCESS;
        }
        else
        {
            return NodeStatus::FAILURE;
        }
    }

  private:
    bool door_open_;
    bool door_locked_;
};
