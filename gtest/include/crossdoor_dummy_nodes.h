#include "behavior_tree_core/bt_factory.h"

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
        return door_open_ ? BT::SUCCESS : BT::FAILURE;
    }

    BT::NodeStatus IsDoorLocked()
    {
        return door_locked_ ? BT::SUCCESS : BT::FAILURE;
    }

    BT::NodeStatus UnlockDoor()
    {
        door_locked_ = false;
        return BT::SUCCESS;
    }

    BT::NodeStatus PassThroughDoor()
    {
        return door_open_ ? BT::SUCCESS : BT::FAILURE;
    }

    BT::NodeStatus PassThroughWindow()
    {
        return BT::SUCCESS;
    }

    BT::NodeStatus OpenDoor()
    {
        if (door_locked_)
            return BT::FAILURE;
        door_open_ = true;
        return BT::SUCCESS;
    }

    BT::NodeStatus CloseDoor()
    {
        if (door_open_)
        {
            door_open_ = false;
            return BT::SUCCESS;
        }
        else
        {
            return BT::FAILURE;
        }
    }

  private:
    bool door_open_;
    bool door_locked_;
};
