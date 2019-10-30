#include <gtest/gtest.h>
#include "behaviortree_cpp_v3/bt_factory.h"
#include "../sample_nodes/dummy_nodes.h"

using namespace BT;

TEST(SubTree, SiblingPorts_Issue_72)
{

static const char* xml_text = R"(

<root main_tree_to_execute = "MainTree" >

    <BehaviorTree ID="MainTree">
        <Sequence>
            <SetBlackboard value="hello" output_key="myParam" />
            <SubTree ID="mySubtree" param="myParam" />
            <SetBlackboard value="world" output_key="myParam" />
            <SubTree ID="mySubtree" param="myParam" />
        </Sequence>
    </BehaviorTree>

    <BehaviorTree ID="mySubtree">
            <SaySomething ID="AlwaysSuccess" message="{param}" />
    </BehaviorTree>
</root> )";

    BehaviorTreeFactory factory;
    factory.registerNodeType<DummyNodes::SaySomething>("SaySomething");

    Tree tree = factory.createTreeFromText(xml_text);

    for( auto& bb: tree.blackboard_stack)
    {
        bb->debugMessage();
        std::cout << "-----" << std::endl;
    }

    auto ret = tree.root_node->executeTick();

    ASSERT_EQ(ret, NodeStatus::SUCCESS );
    ASSERT_EQ(tree.blackboard_stack.size(), 3 );
}
