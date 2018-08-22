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


const std::string xml_text = R"(

 <root main_tree_to_execute = "MainTree" >

     <BehaviorTree ID="MainTree">
        <Sequence name="root_sequence">
            <SimpleAction   name="action_one"/>
            <ActionOne      name="action_two"/>
            <ActionTwo      name="my_action"/>
            <CustomAction   name="my_action"/>
        </Sequence>
     </BehaviorTree>

 </root>
 )";

int main()
{
    BehaviorTreeFactory factory;

    Foo foo;

    // This is the syntax to register SimpleActionNodes
    factory.registerSimpleAction("SimpleAction", std::bind(SimpleAction) );
    factory.registerSimpleAction("ActionOne", std::bind( &Foo::actionOne, &foo));
    factory.registerSimpleAction("ActionTwo", std::bind( &Foo::actionTwo, &foo));

    factory.registerNodeType<CustomAction>("CustomAction");

    auto res = buildTreeFromText(factory, xml_text);
    const TreeNode::Ptr& root_node = res.first;

    // the tick is propagated to all the children.
    // until one of the returns FAILURE or RUNNING.
    // In this case all of the return SUCCESS
    root_node->executeTick();

    return 0;
}
