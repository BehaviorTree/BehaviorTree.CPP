#ifndef MOVEBASE_BT_NODES_H
#define MOVEBASE_BT_NODES_H

#include "behavior_tree_core/behavior_tree.h"

// Custom type
struct Pose2D
{
    double x,y,theta;
};

namespace BT{

// This template specialization is needed only if you want
// to AUTOMATICALLY convert a NodeParameter into a Pose2D
// In other words, implement it if you want to be able to do:
//
//   TreeNode::getParam<Pose2D>(key, ...)
//
template <> Pose2D convertFromString(const std::string& key)
{
    // three real numbers separated by semicolons
    auto parts = BT::splitString(key, ';');
    if( parts.size() != 3)
    {
        throw std::runtime_error("invalid input)");
    }
    else{
        Pose2D output;
        output.x     = convertFromString<double>( parts[0] );
        output.y     = convertFromString<double>( parts[1] );
        output.theta = convertFromString<double>( parts[2] );
        return output;
    }
}

}

// This is an asynchronous operation that will run in a separate thread.
// It requires the NodeParameter "goal"

class MoveBaseAction: public BT::ActionNode
{
public:
    // When your TreeNode requires a NodeParameter, use this
    // kind of constructor
    MoveBaseAction(const std::string& name, const BT::NodeParameters& params):
        ActionNode(name, params) {}

    // This is mandatory to tell to the factory which parameter(s)
    // are required
    static const BT::NodeParameters& requiredNodeParameters()
    {
        static BT::NodeParameters params = {{"goal","0;0;0"}};
        return params;
    }

    BT::NodeStatus tick() override
    {
        Pose2D goal;

        // retrieve the parameter using getParam()
        if( getParam<Pose2D>("goal", goal) )
        {
            printf("[MoveBase] goal: x=%.f y=%.1f theta=%.2f\n",
                   goal.x, goal.y, goal.theta);

            std::this_thread::sleep_for( std::chrono::milliseconds(180) );

            std::cout << "[ Move: finished ]" << std::endl;
            return BT::NodeStatus::SUCCESS;
        }
        else{
           printf("The NodeParameter does not contain the key [goal] "
                  " or the blackboard does not contain the provided key\n");
           return BT::NodeStatus::FAILURE;
        }
    }
};





#endif // MOVEBASE_BT_NODES_H
