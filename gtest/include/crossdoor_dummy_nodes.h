#include "behavior_tree_core/bt_factory.h"

using namespace BT;

class CrossDoor
{
    int _multiplier;

  public:
    CrossDoor(BT::BehaviorTreeFactory& factory, bool fast = true)
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

        _multiplier = fast ? 1 : 10;
    }

    BT::NodeStatus IsDoorOpen()
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(50) * _multiplier);
        return door_open_ ? NodeStatus::SUCCESS : NodeStatus::FAILURE;
    }

    BT::NodeStatus IsDoorLocked()
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(50) * _multiplier);
        return door_locked_ ? NodeStatus::SUCCESS : NodeStatus::FAILURE;
    }

    BT::NodeStatus UnlockDoor()
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(200) * _multiplier);
        door_locked_ = false;
        return NodeStatus::SUCCESS;
    }

    BT::NodeStatus PassThroughDoor()
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100) * _multiplier);
        return door_open_ ? NodeStatus::SUCCESS : NodeStatus::FAILURE;
    }

    BT::NodeStatus PassThroughWindow()
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100) * _multiplier);
        return NodeStatus::SUCCESS;
    }

    BT::NodeStatus OpenDoor()
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(200) * _multiplier);
        if (door_locked_)
            return NodeStatus::FAILURE;
        door_open_ = true;
        return NodeStatus::SUCCESS;
    }

    BT::NodeStatus CloseDoor()
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(150) * _multiplier);
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
