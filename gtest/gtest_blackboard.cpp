/* Copyright (C) 2018-2019 Davide Faconti, Eurecat - All Rights Reserved
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
      SyncActionNode(name, config)
    { }

    NodeStatus tick()
    {
        int value = 0;
        if(!getInput("in_port", value))
        {
            throw RuntimeError("BB_TestNode needs input");
        }

        value *= 2;
        if( !setOutput("out_port", value) )
        {
            throw RuntimeError("BB_TestNode failed output");
        }
        return NodeStatus::SUCCESS;
    }

    static const PortsList& providedPorts()
    {
        static PortsList ports = {{"in_port",  {PortType::INPUT, typeid(int)}},
                                  {"out_port", {PortType::OUTPUT, typeid(int)}}
                                 };
        return ports;
    }

  private:

};


TEST(BlackboardTest, GetInputsFromBlackboard)
{
    auto bb = Blackboard::create<BlackboardLocal>();

    NodeConfiguration config;
    assignDefaultRemapping<BB_TestNode>( config );

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
    config.input_ports["in_port"]   = "{my_input_port}";
    config.output_ports["out_port"] = "{my_output_port}";
    bb->set("my_input_port", 11 );

    BB_TestNode node("good_one", config);
    node.executeTick();

    ASSERT_EQ( bb->get<int>("my_output_port"), 22 );
}

TEST(BlackboardTest, GetInputsFromText)
{
    auto bb = Blackboard::create<BlackboardLocal>();

    NodeConfiguration config;
    config.input_ports["in_port"] = "11";

    BB_TestNode missing_out("missing_output", config);
    EXPECT_THROW( missing_out.executeTick(), RuntimeError );

    config.blackboard = bb;
    config.output_ports["out_port"] = "=";

    BB_TestNode node("good_one", config);
    node.executeTick();

    ASSERT_EQ( bb->get<int>("out_port"), 22 );
}

TEST(BlackboardTest, WithFactory)
{
    BehaviorTreeFactory factory;

    factory.registerNodeType<BB_TestNode>("BB_TestNode");

    const std::string xml_text = R"(

    <root main_tree_to_execute = "MainTree" >
        <BehaviorTree ID="MainTree">
            <Sequence>
                <BB_TestNode name = "first" in_port="11"
                             out_port="{my_input_port}"/>

                <BB_TestNode name = "second" in_port="{my_input_port}"
                             out_port="{my_input_port}" />

                <BB_TestNode name = "third" in_port="{my_input_port}"
                             out_port="{my_output_port}" />
            </Sequence>
        </BehaviorTree>
    </root>)";

    auto bb = Blackboard::create<BlackboardLocal>();

    auto tree = buildTreeFromText(factory, xml_text, bb);
    NodeStatus status = tree.root_node->executeTick();

    ASSERT_EQ( status, NodeStatus::SUCCESS );
    ASSERT_EQ( bb->get<int>("my_input_port"), 44 );
    ASSERT_EQ( bb->get<int>("my_output_port"), 88 );
}

