#include "behavior_tree_core/xml_parsing.h"
#include "behavior_tree_logger/bt_cout_logger.h"
#include "Blackboard/blackboard_local.h"

using namespace BT;

NodeStatus SimpleAction()
{
    std::cout << "SimpleActionFunc" << std::endl;
    return NodeStatus::SUCCESS;
}

class Foo
{
public:
    Foo(): _val(0) {}

    NodeStatus actionOne()
    {
        _val = 42;
        std::cout << "Foo::actionOne -> set val to 42" << std::endl;
        return NodeStatus::SUCCESS;
    }

    NodeStatus actionTwo()
    {
        std::cout << "Foo::actionTwo -> reading val => "<< _val << std::endl;
        _val = 0;
        return NodeStatus::SUCCESS;
    }

private:
     int _val;
};


class CustomAction: public ActionNodeBase
{
public:
    CustomAction(const std::string& name): ActionNodeBase(name) {}

    NodeStatus tick() override
    {
        std::cout << "CustomAction: " << this->name() << std::endl;
        return NodeStatus::SUCCESS;
    }
};


int main()
{
    Foo foo;

    BT::SequenceNode sequence_root("sequence");

    // Aimple funtions can be wrapped inside in ActionNodeBase
    // using the SimpleActionNode
    SimpleActionNode act_simple("simple_action", std::bind(SimpleAction) );

    // SimpleActionNode warks also with class methods, using std::bind
    SimpleActionNode act_1("action_one", std::bind( &Foo::actionOne, &foo) );
    SimpleActionNode act_2("action_two", std::bind( &Foo::actionTwo, &foo) );

    // Nevertheless, the way to be able to use ALL the funtionality of a TreeNode
    // is to create your own class that inherits from either:
    // - ConditionNode  (synchronous execution)
    // - ActionNodeBase (synchronous execution)
    // - ActionNode     (asynchronous execution is a separate thread).
    CustomAction act_custom("my_action");

    // Add children to the sequence.
    // they will be executed in the same order they are added.
    sequence_root.addChild(&act_simple);
    sequence_root.addChild(&act_1);
    sequence_root.addChild(&act_2);
    sequence_root.addChild(&act_custom);

    // the tick is propagated to all the children.
    // until one of the returns FAILURE or RUNNING.
    // In this case all of the return SUCCESS
    sequence_root.executeTick();

    return 0;
}
