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
#include "behaviortree_cpp_v3/behavior_tree.h"
#include "behaviortree_cpp_v3/bt_factory.h"
#include "behaviortree_cpp_v3/blackboard.h"
#include "behaviortree_cpp_v3/xml_parsing.h"

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
        auto res = getInput<int>("in_port");
        if(!res)
        {
            throw RuntimeError("BB_TestNode needs input", res.error());
        }
        value = res.value()*2;
        if( !setOutput("out_port", value) )
        {
            throw RuntimeError("BB_TestNode failed output");
        }
        return NodeStatus::SUCCESS;
    }

    static PortsList providedPorts()
    {
        return { BT::InputPort<int>("in_port"),
                 BT::OutputPort<int>("out_port") };
    }
};


class BB_TypedTestNode: public SyncActionNode
{
  public:
    BB_TypedTestNode(const std::string& name, const NodeConfiguration& config):
      SyncActionNode(name, config)
    { }

    NodeStatus tick()
    {
        return NodeStatus::SUCCESS;
    }

    static PortsList providedPorts()
    {
        return { BT::InputPort("input"),
                 BT::InputPort<int>("input_int"),
                 BT::InputPort<std::string>("input_string"),

                 BT::OutputPort("output"),
                 BT::OutputPort<int>("output_int"),
                 BT::OutputPort<std::string>("output_string") };
    }
};


TEST(BlackboardTest, GetInputsFromBlackboard)
{
    auto bb = Blackboard::create();

    NodeConfiguration config;
    assignDefaultRemapping<BB_TestNode>( config );

    config.blackboard = bb;
    bb->set("in_port", 11 );

    BB_TestNode node("good_one", config);

    // this should read and write "my_entry" in tick()
    node.executeTick();

    ASSERT_EQ( bb->get<int>("out_port"), 22 );
}

TEST(BlackboardTest, BasicRemapping)
{
    auto bb = Blackboard::create();

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
    auto bb = Blackboard::create();

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

TEST(BlackboardTest, SetOutputFromText)
{
    const char* xml_text = R"(

     <root main_tree_to_execute = "MainTree" >
         <BehaviorTree ID="MainTree">
            <Sequence>
                <BB_TestNode in_port="11" out_port="{my_port}"/>
                <SetBlackboard output_key="my_port" value="-43" />
            </Sequence>
         </BehaviorTree>
     </root>
    )";

    BehaviorTreeFactory factory;
    factory.registerNodeType<BB_TestNode>("BB_TestNode");

    auto bb = Blackboard::create();

    auto tree = factory.createTreeFromText(xml_text, bb);
    tree.root_node->executeTick();
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

    auto bb = Blackboard::create();

    auto tree = factory.createTreeFromText(xml_text, bb);
    NodeStatus status = tree.root_node->executeTick();

    ASSERT_EQ( status, NodeStatus::SUCCESS );
    ASSERT_EQ( bb->get<int>("my_input_port"), 44 );
    ASSERT_EQ( bb->get<int>("my_output_port"), 88 );
}


TEST(BlackboardTest, TypoInPortName)
{
    BehaviorTreeFactory factory;
    factory.registerNodeType<BB_TestNode>("BB_TestNode");

    const std::string xml_text = R"(

    <root main_tree_to_execute = "MainTree" >
        <BehaviorTree ID="MainTree">
             <BB_TestNode inpuuuut_port="{value}" />
        </BehaviorTree>
    </root>)";

    ASSERT_THROW( factory.createTreeFromText(xml_text), RuntimeError );
}


TEST(BlackboardTest, CheckPortType)
{
    BehaviorTreeFactory factory;
    factory.registerNodeType<BB_TypedTestNode>("TypedNode");

    //-----------------------------
    std::string good_one = R"(
    <root main_tree_to_execute = "MainTree" >
        <BehaviorTree ID="MainTree">
            <Sequence>
                <TypedNode name = "first"  output_int="{matching}"  output_string="{whatever}" output="{no_problem}" />
                <TypedNode name = "second" input_int="{matching}"   input="{whatever}"         input_string="{no_problem}"  />
            </Sequence>
        </BehaviorTree>
    </root>)";

    auto tree = factory.createTreeFromText(good_one);
    ASSERT_NE( tree.root_node, nullptr );
    //-----------------------------
    std::string bad_one = R"(
    <root main_tree_to_execute = "MainTree" >
        <BehaviorTree ID="MainTree">
            <Sequence>
                <TypedNode name = "first"  output_int="{value}" />
                <TypedNode name = "second" input_string="{value}" />
            </Sequence>
        </BehaviorTree>
    </root>)";

    ASSERT_THROW( factory.createTreeFromText(bad_one), RuntimeError);
}

class RefCountClass {
  public:
    RefCountClass(std::shared_ptr<int> value): sptr_(std::move(value))
    {
        std::cout<< "Constructor: ref_count " << sptr_.use_count() << std::endl;
    }

    RefCountClass(const RefCountClass &from) : sptr_(from.sptr_)
    {
        std::cout<< "ctor copy: ref_count " << sptr_.use_count() << std::endl;
    }

    RefCountClass& operator=(const RefCountClass &from)
    {
        sptr_ = (from.sptr_);
        std::cout<< "equal copied: ref_count " << sptr_.use_count() << std::endl;
        return *this;
    }

    virtual ~RefCountClass() {
        std::cout<<("Destructor")<< std::endl;
    }

    int refCount() const { return sptr_.use_count(); }

  private:
    std::shared_ptr<int> sptr_;
};

TEST(BlackboardTest, MoveVsCopy)
{
    auto blackboard = Blackboard::create();

    RefCountClass test( std::make_shared<int>() );

    ASSERT_EQ( test.refCount(), 1);

    std::cout<<("----- before set -----")<< std::endl;
    blackboard->set("testmove", test );
    std::cout<<(" ----- after set -----")<< std::endl;

    ASSERT_EQ( test.refCount(), 2);

    RefCountClass other( blackboard->get<RefCountClass>("testmove") );

    ASSERT_EQ( test.refCount(), 3);
}


TEST(BlackboardTest, CheckTypeSafety)
{
    //TODO check type safety when ports are created.
    // remember that std::string is considered a type erased type.

    bool is = std::is_constructible<BT::StringView, char*>::value;
    ASSERT_TRUE( is );

    is = std::is_constructible<BT::StringView, std::string>::value;
    ASSERT_TRUE( is );
}

