#include <gtest/gtest.h>
#include "behaviortree_cpp_v3/bt_factory.h"

using namespace BT;

class NodeWithPorts: public SyncActionNode
{
  public:
    NodeWithPorts(const std::string & name, const NodeConfiguration & config)
    : SyncActionNode(name, config)
    {
        std::cout << "ctor" << std::endl;
    }

      NodeStatus tick()
    {
        int val_A = 0;
        int val_B = 0;
        if( getInput("in_port_A", val_A) &&
            getInput("in_port_B", val_B) &&
            val_A == 42 && val_B == 66)
        {
            return NodeStatus::SUCCESS;
        }
        return NodeStatus::FAILURE;
    }

    static PortsList providedPorts()
    {
        return { BT::InputPort<int>("in_port_A", 42, "magic_number"),
                 BT::InputPort<int>("in_port_B") };
    }
};

TEST(PortTest, DefaultPorts)
{
    std::string xml_txt = R"(
    <root main_tree_to_execute = "MainTree" >
        <BehaviorTree ID="MainTree">
            <NodeWithPorts name = "first"  in_port_B="66" />
        </BehaviorTree>
    </root>)";

    BehaviorTreeFactory factory;
    factory.registerNodeType<NodeWithPorts>("NodeWithPorts");

    auto tree = factory.createTreeFromText(xml_txt);

    NodeStatus status = tree.tickRoot();
    ASSERT_EQ( status, NodeStatus::SUCCESS );

}

