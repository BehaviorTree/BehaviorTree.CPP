#include "movebase_node.h"
#include "behaviortree_cpp_v3/bt_factory.h"

// This function must be implemented in the .cpp file to create
// a plugin that can be loaded at run-time
BT_REGISTER_NODES(factory)
{
    factory.registerNodeType<MoveBaseAction>("MoveBase");
}

BT::NodeStatus MoveBaseAction::tick()
{
    Pose2D goal;
    if ( !getInput<Pose2D>("goal", goal))
    {
        throw BT::RuntimeError("missing required input [goal]");
    }

    printf("[ MoveBase: STARTED ]. goal: x=%.f y=%.1f theta=%.2f\n", goal.x, goal.y, goal.theta);

    _halt_requested.store(false);
    int count = 0;

    // Pretend that "computing" takes 250 milliseconds.
    // It is up to you to check periodicall _halt_requested and interrupt
    // this tick() if it is true.
    while (!_halt_requested && count++ < 25)
    {
        SleepMS(10);
    }

    std::cout << "[ MoveBase: FINISHED ]" << std::endl;
    return _halt_requested ? BT::NodeStatus::FAILURE : BT::NodeStatus::SUCCESS;
}

void MoveBaseAction::halt()
{
    _halt_requested.store(true);
}
