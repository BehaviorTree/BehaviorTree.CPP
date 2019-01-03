/* Copyright (C) 2018 Davide Faconti - All Rights Reserved
*
*   Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"),
*   to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
*   and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
*   The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
*
*   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
*   WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <gtest/gtest.h>
#include "action_test_node.h"
#include "condition_test_node.h"
#include "behaviortree_cpp/behavior_tree.h"
#include "behaviortree_cpp/bt_factory.h"
#include "behaviortree_cpp/blackboard/blackboard_local.h"
#include "behaviortree_cpp/xml_parsing.h"

using namespace BT;

class BB_TestNode: public SyncActionNode
{
  public:
    BB_TestNode(const std::string& name, const NodeConfiguration& config):
      SyncActionNode(name, config),
      _value(0)
    {
        if(!getInput("in_port", _value))
        {
            throw  std::runtime_error("need input");
        }
    }

    NodeStatus tick()
    {
        _value *= 2;
        setOutput("out_port", _value);
        return NodeStatus::SUCCESS;
    }

    static const PortsList& providedPorts()
    {
        static PortsList ports = {{"in_port", PortType::INPUT},
                                  {"out_port", PortType::OUTPUT}};
        return ports;
    }

  private:
    int _value;
};




/****************TESTS START HERE***************************/

TEST(BlackboardTest, GetInputsFromBlackboard)
{
    auto bb = Blackboard::create<BlackboardLocal>();

    NodeConfiguration config;

    //Fails because config does not contain input/output ports
    EXPECT_ANY_THROW( BB_TestNode("missing_port", config) );

    assignDefaultRemapping<BB_TestNode>( config );

    //Fails because config.blackboard is still empty.
    EXPECT_ANY_THROW( BB_TestNode("missing_bb", config) );

    config.blackboard = bb;
    bb->set("in_port", 11 );

    // NO throw
    BB_TestNode node("good_one", config);

    // this should read and write "my_entry", respectively in onInit() and tick()
    node.executeTick();

    ASSERT_EQ( bb->get<int>("out_port"), 22 );
}

TEST(BlackboardTest, BasicRemapping)
{
    auto bb = Blackboard::create<BlackboardLocal>();

    NodeConfiguration config;

    config.blackboard = bb;
    config.input_ports["in_port"]   = "${my_input_port}";
    config.output_ports["out_port"] = "${my_output_port}";
    bb->set("my_input_port", 11 );

    BB_TestNode node("good_one", config);
    node.executeTick();

    ASSERT_EQ( bb->get<int>("my_output_port"), 22 );
}

TEST(BlackboardTest, GetInputsFromText)
{
    auto bb = Blackboard::create<BlackboardLocal>();

    NodeConfiguration config;

    config.blackboard = bb;
    config.input_ports["in_port"] = "11";
    config.output_ports["out_port"] = "=";

    BB_TestNode node("good_one", config);
    node.executeTick();

    ASSERT_EQ( bb->get<int>("out_port"), 22 );
}

TEST(BlackboardTest, WithFactory)
{
    auto bb = Blackboard::create<BlackboardLocal>();
    BehaviorTreeFactory factory;

    factory.registerNodeType<BB_TestNode>("BB_TestNode");

    const std::string xml_text = R"(

    <root main_tree_to_execute = "MainTree" >
        <BehaviorTree ID="MainTree">
            <Sequence>
                <BB_TestNode name="nodeA" in_port="11"
                                          out_port="${my_output_port_A}"/>
                <BB_TestNode name="nodeB" in_port="${my_input_port}"
                                          out_port="${my_output_port_B}" />
            </Sequence>
        </BehaviorTree>
    </root>)";

    bb->set( "my_input_port", 42 );

    auto tree = buildTreeFromText(factory, xml_text, bb);
    NodeStatus status = tree.root_node->executeTick();

    ASSERT_EQ( status, NodeStatus::SUCCESS );
    ASSERT_EQ( bb->get<int>("my_output_port_A"), 22 );
    ASSERT_EQ( bb->get<int>("my_output_port_B"), 84 );

}

