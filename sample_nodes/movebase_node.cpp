#include "movebase_node.h"
#include "behavior_tree_core/bt_factory.h"


// This function must be implemented in the .cpp file to create
// a plugin that can be loaded at run-time
BT_REGISTER_NODES(factory)
{
    factory.registerNodeType<MoveBaseAction>("MoveBase");
}

BT::NodeStatus MoveBaseAction::tick()
{
    Pose2D goal;

    // Retrieve the parameter using getParam()
    bool goal_passed = getParam<Pose2D>("goal", goal);

    //  You might want to create a node that fails if a mandatory parameter is missing.

//  if( !goal_passed )
//  {
//      printf("The NodeParameter does not contain the key [goal] "
//             " or the blackboard does not contain the provided key\n");
//      return BT::NodeStatus::FAILURE;
//  }

     // In this case, if "goal" was not passed, the default value is used instead.
    if( !goal_passed )
    {
        auto& default_goal_value =  requiredNodeParameters().begin()->second;
        goal = BT::convertFromString<Pose2D>( default_goal_value );
    }
    printf("[ MoveBase: STARTED ]. goal: x=%.f y=%.1f theta=%.2f\n",
           goal.x, goal.y, goal.theta);

    _halt_requested.store(false);
    int count = 0;

    // "compute" for 250 milliseconds or until _halt_requested is true
    while( !_halt_requested && count++ < 25)
    {
        SleepMS(10);
    }

    std::cout << "[ MoveBase: FINISHED ]" << std::endl;
    return _halt_requested ? BT::NodeStatus::SUCCESS : BT::NodeStatus::SUCCESS;
}

void MoveBaseAction::halt()
{
    _halt_requested.store(true);
}
