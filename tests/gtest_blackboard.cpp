/* Copyright (C) 2018-2023 Davide Faconti, Eurecat - All Rights Reserved
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
#include "behaviortree_cpp/bt_factory.h"
#include "behaviortree_cpp/blackboard.h"

#include "../sample_nodes/dummy_nodes.h"

using namespace BT;

class BB_TestNode : public SyncActionNode
{
public:
  BB_TestNode(const std::string& name, const NodeConfig& config)
    : SyncActionNode(name, config)
  {}

  NodeStatus tick()
  {
    int value = 0;
    auto res = getInput<int>("in_port");
    if(!res)
    {
      throw RuntimeError("BB_TestNode needs input: ", res.error());
    }
    value = res.value() * 2;
    if(!setOutput("out_port", value))
    {
      throw RuntimeError("BB_TestNode failed output");
    }
    return NodeStatus::SUCCESS;
  }

  static PortsList providedPorts()
  {
    return { BT::InputPort<int>("in_port"), BT::OutputPort<int>("out_port") };
  }
};

class BB_TypedTestNode : public SyncActionNode
{
public:
  BB_TypedTestNode(const std::string& name, const NodeConfig& config)
    : SyncActionNode(name, config)
  {}

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

  NodeConfig config;
  assignDefaultRemapping<BB_TestNode>(config);

  config.blackboard = bb;
  bb->set("in_port", 11);

  BB_TestNode node("good_one", config);

  // this should read and write "my_entry" in tick()
  node.executeTick();

  ASSERT_EQ(bb->get<int>("out_port"), 22);
}

TEST(BlackboardTest, BasicRemapping)
{
  auto bb = Blackboard::create();

  NodeConfig config;

  config.blackboard = bb;
  config.input_ports["in_port"] = "{my_input_port}";
  config.output_ports["out_port"] = "{my_output_port}";
  bb->set("my_input_port", 11);

  BB_TestNode node("good_one", config);
  node.executeTick();

  ASSERT_EQ(bb->get<int>("my_output_port"), 22);
}

TEST(BlackboardTest, GetInputsFromText)
{
  auto bb = Blackboard::create();

  NodeConfig config;
  config.input_ports["in_port"] = "11";

  BB_TestNode missing_out("missing_output", config);
  EXPECT_THROW(missing_out.executeTick(), RuntimeError);

  config.blackboard = bb;
  config.output_ports["out_port"] = "{=}";

  BB_TestNode node("good_one", config);
  node.executeTick();

  ASSERT_EQ(bb->get<int>("out_port"), 22);
}

TEST(BlackboardTest, SetOutputFromText)
{
  const char* xml_text = R"(

     <root BTCPP_format="4" >
         <BehaviorTree ID="MainTree">
            <Sequence>
                <BB_TestNode in_port="11" out_port="{my_port}"/>
                <Script code="my_port=-43" />
            </Sequence>
         </BehaviorTree>
     </root>
    )";

  BehaviorTreeFactory factory;
  factory.registerNodeType<BB_TestNode>("BB_TestNode");

  auto bb = Blackboard::create();

  auto tree = factory.createTreeFromText(xml_text, bb);
  tree.tickWhileRunning();
}

TEST(BlackboardTest, WithFactory)
{
  BehaviorTreeFactory factory;

  factory.registerNodeType<BB_TestNode>("BB_TestNode");

  const std::string xml_text = R"(

    <root BTCPP_format="4" >
        <BehaviorTree ID="MainTree">
            <Sequence>
                <BB_TestNode in_port="11"
                             out_port="{my_input_port}"/>

                <BB_TestNode in_port="{my_input_port}"
                             out_port="{my_input_port}" />

                <BB_TestNode in_port="{my_input_port}"
                             out_port="{my_output_port}" />
            </Sequence>
        </BehaviorTree>
    </root>)";

  auto bb = Blackboard::create();

  auto tree = factory.createTreeFromText(xml_text, bb);
  NodeStatus status = tree.tickWhileRunning();

  ASSERT_EQ(status, NodeStatus::SUCCESS);
  ASSERT_EQ(bb->get<int>("my_input_port"), 44);
  ASSERT_EQ(bb->get<int>("my_output_port"), 88);
}

TEST(BlackboardTest, TypoInPortName)
{
  BehaviorTreeFactory factory;
  factory.registerNodeType<BB_TestNode>("BB_TestNode");

  const std::string xml_text = R"(

    <root BTCPP_format="4" >
        <BehaviorTree ID="MainTree">
             <BB_TestNode inpuuuut_port="{value}" />
        </BehaviorTree>
    </root>)";

  ASSERT_THROW(auto tree = factory.createTreeFromText(xml_text), RuntimeError);
}

TEST(BlackboardTest, CheckPortType)
{
  BehaviorTreeFactory factory;
  factory.registerNodeType<BB_TypedTestNode>("TypedNode");

  //-----------------------------
  std::string good_one = R"(
    <root BTCPP_format="4" >
        <BehaviorTree ID="MainTree">
            <Sequence>
                <TypedNode name = "first"  output_int="{matching}"  output_string="{whatever}" output="{no_problem}" />
                <TypedNode name = "second" input_int="{matching}"   input="{whatever}"         input_string="{no_problem}"  />
            </Sequence>
        </BehaviorTree>
    </root>)";

  auto tree = factory.createTreeFromText(good_one);
  ASSERT_NE(tree.rootNode(), nullptr);
  //-----------------------------
  std::string bad_one = R"(
    <root BTCPP_format="4" >
        <BehaviorTree ID="MainTree">
            <Sequence>
                <TypedNode name = "first"  output_int="{value}" />
                <TypedNode name = "second" input_string="{value}" />
            </Sequence>
        </BehaviorTree>
    </root>)";

  ASSERT_THROW(auto tree = factory.createTreeFromText(bad_one), RuntimeError);
}

class RefCountClass
{
public:
  RefCountClass(std::shared_ptr<int> value) : sptr_(std::move(value))
  {
    std::cout << "Constructor: ref_count " << sptr_.use_count() << std::endl;
  }

  RefCountClass(const RefCountClass& from) : sptr_(from.sptr_)
  {
    std::cout << "ctor copy: ref_count " << sptr_.use_count() << std::endl;
  }

  RefCountClass& operator=(const RefCountClass& from)
  {
    sptr_ = (from.sptr_);
    std::cout << "equal copied: ref_count " << sptr_.use_count() << std::endl;
    return *this;
  }

  virtual ~RefCountClass()
  {
    std::cout << ("Destructor") << std::endl;
  }

  int refCount() const
  {
    return sptr_.use_count();
  }

private:
  std::shared_ptr<int> sptr_;
};

TEST(BlackboardTest, MoveVsCopy)
{
  auto blackboard = Blackboard::create();

  RefCountClass test(std::make_shared<int>());

  ASSERT_EQ(test.refCount(), 1);

  std::cout << ("----- before set -----") << std::endl;
  blackboard->set("testmove", test);
  std::cout << (" ----- after set -----") << std::endl;

  ASSERT_EQ(test.refCount(), 2);

  RefCountClass other(blackboard->get<RefCountClass>("testmove"));

  ASSERT_EQ(test.refCount(), 3);
}

TEST(BlackboardTest, CheckTypeSafety)
{
  //TODO check type safety when ports are created.
  // remember that std::string is considered a type erased type.

  bool is = std::is_constructible<BT::StringView, char*>::value;
  ASSERT_TRUE(is);

  is = std::is_constructible<BT::StringView, std::string>::value;
  ASSERT_TRUE(is);
}

TEST(BlackboardTest, AnyPtrLocked)
{
  auto blackboard = Blackboard::create();
  long value = 0;
  long* test_obj = &value;

  blackboard->set("testmove", test_obj);

  auto const timeout = std::chrono::milliseconds(250);

  // Safe way to access a pointer
  {
    std::atomic_llong cycles = 0;
    auto func = [&]() {
      auto start = std::chrono::system_clock::now();
      while((std::chrono::system_clock::now() - start) < timeout)
      {
        auto r1 = blackboard->getAnyLocked("testmove");
        auto value_ptr = (r1.get()->cast<long*>());
        (*value_ptr)++;
        cycles++;
      }
    };

    auto t1 = std::thread(func);  // other thread
    func();                       // this thread
    t1.join();

    // number of increments and cycles is expected to be the same
    ASSERT_EQ(cycles, value);
  }
  //------------------
  // UNSAFE way to access a pointer
  {
    std::atomic_llong cycles = 0;
    auto func = [&]() {
      auto start = std::chrono::system_clock::now();
      while((std::chrono::system_clock::now() - start) < timeout)
      {
        auto value_ptr = blackboard->get<long*>("testmove");
        (*value_ptr)++;
        cycles++;
      }
    };

    auto t1 = std::thread(func);
    func();
    t1.join();
    // since the operation value_ptr++ is not thread safe, cycle and value will unlikely be the same
    ASSERT_NE(cycles, value);
  }
}

TEST(BlackboardTest, SetStringView)
{
  auto bb = Blackboard::create();

  constexpr BT::StringView string_view_const = "BehaviorTreeCpp";
  bb->set("string_view", string_view_const);

  ASSERT_NO_THROW(bb->set("string_view", string_view_const));
}

TEST(ParserTest, Issue605_whitespaces)
{
  BT::BehaviorTreeFactory factory;

  const std::string xml_text = R"(
  <root BTCPP_format="4" >
    <BehaviorTree ID="MySubtree">
      <Script code=" sub_value:=false " />
    </BehaviorTree>

    <BehaviorTree ID="MainTree">
      <Sequence>
        <Script code=" my_value:=true " />
        <SubTree ID="MySubtree" sub_value="{my_value}  "/>
      </Sequence>
    </BehaviorTree>
  </root> )";

  factory.registerBehaviorTreeFromText(xml_text);
  auto tree = factory.createTree("MainTree");
  const auto status = tree.tickWhileRunning();

  for(auto const& subtree : tree.subtrees)
  {
    subtree->blackboard->debugMessage();
  }

  ASSERT_EQ(status, BT::NodeStatus::SUCCESS);
  ASSERT_EQ(false, tree.rootBlackboard()->get<bool>("my_value"));
}

class ComparisonNode : public BT::ConditionNode
{
public:
  ComparisonNode(const std::string& name, const BT::NodeConfiguration& config)
    : BT::ConditionNode(name, config)
  {}

  static BT::PortsList providedPorts()
  {
    return { BT::InputPort<int32_t>("first"), BT::InputPort<int32_t>("second"),
             BT::InputPort<std::string>("operator") };
  }

  BT::NodeStatus tick() override
  {
    int32_t firstValue = 0;
    int32_t secondValue = 0;
    std::string inputOperator;
    if(!getInput("first", firstValue) || !getInput("second", secondValue) ||
       !getInput("operator", inputOperator))
    {
      throw RuntimeError("can't access input");
    }
    if((inputOperator == "==" && firstValue == secondValue) ||
       (inputOperator == "!=" && firstValue != secondValue) ||
       (inputOperator == "<=" && firstValue <= secondValue) ||
       (inputOperator == ">=" && firstValue >= secondValue) ||
       (inputOperator == "<" && firstValue < secondValue) ||
       (inputOperator == ">" && firstValue > secondValue))
    {
      return BT::NodeStatus::SUCCESS;
    }
    // skipping the rest of the implementation
    return BT::NodeStatus::FAILURE;
  }
};

TEST(BlackboardTest, IssueSetBlackboard)
{
  BT::BehaviorTreeFactory factory;

  const std::string xml_text = R"(
  <root BTCPP_format="4" >
    <BehaviorTree ID="MySubtree">
      <ComparisonNode first="{value}" second="42" operator="==" />
    </BehaviorTree>

    <BehaviorTree ID="MainTree">
      <Sequence>
        <SetBlackboard value="42" output_key="value" />
        <SubTree ID="MySubtree" value="{value}  "/>
      </Sequence>
    </BehaviorTree>
  </root> )";

  factory.registerNodeType<ComparisonNode>("ComparisonNode");
  factory.registerBehaviorTreeFromText(xml_text);
  auto tree = factory.createTree("MainTree");
  const auto status = tree.tickWhileRunning();

  ASSERT_EQ(status, BT::NodeStatus::SUCCESS);
  ASSERT_EQ(42, tree.rootBlackboard()->get<int>("value"));
}

struct Point
{
  double x;
  double y;
};

// Template specialization to converts a string to Point.
namespace BT
{
template <>
[[nodiscard]] Point convertFromString(StringView str)
{
  // We expect real numbers separated by semicolons
  auto parts = splitString(str, ';');
  if(parts.size() != 2)
  {
    throw RuntimeError("invalid input)");
  }
  else
  {
    Point output{ 0.0, 0.0 };
    output.x = convertFromString<double>(parts[0]);
    output.y = convertFromString<double>(parts[1]);
    // std::cout << "Building a position 2d object " << output.x << "; " << output.y << "\n" << std::flush;
    return output;
  }
}
}  // end namespace BT

TEST(BlackboardTest, SetBlackboard_Issue725)
{
  BT::BehaviorTreeFactory factory;

  const std::string xml_text = R"(
  <root BTCPP_format="4">
    <BehaviorTree ID="MainTree">
      <SetBlackboard value="{first_point}" output_key="other_point" />
    </BehaviorTree>
  </root> )";

  factory.registerNodeType<DummyNodes::SaySomething>("SaySomething");
  factory.registerBehaviorTreeFromText(xml_text);
  auto tree = factory.createTree("MainTree");
  auto& blackboard = tree.subtrees.front()->blackboard;

  const Point point = { 2, 7 };
  blackboard->set("first_point", point);

  const auto status = tree.tickOnce();

  Point other_point = blackboard->get<Point>("other_point");

  ASSERT_EQ(status, BT::NodeStatus::SUCCESS);
  ASSERT_EQ(other_point.x, point.x);
  ASSERT_EQ(other_point.y, point.y);
}

TEST(BlackboardTest, NullOutputRemapping)
{
  auto bb = Blackboard::create();

  NodeConfig config;

  config.blackboard = bb;
  config.input_ports["in_port"] = "{my_input_port}";
  config.output_ports["out_port"] = "";
  bb->set("my_input_port", 11);

  BB_TestNode node("good_one", config);

  // This will throw because setOutput should fail in BB_TestNode::tick()
  ASSERT_ANY_THROW(node.executeTick());
}

TEST(BlackboardTest, BlackboardBackup)
{
  BT::BehaviorTreeFactory factory;

  const std::string xml_text = R"(
  <root BTCPP_format="4" >
    <BehaviorTree ID="MySubtree">
      <Sequence>
        <Script code=" important_value:= sub_value " />
        <Script code=" my_value=false " />
        <SaySomething message="{message}" />
      </Sequence>
    </BehaviorTree>
    <BehaviorTree ID="MainTree">
      <Sequence>
        <Script code=" my_value:=true; another_value:='hi' " />
        <SubTree ID="MySubtree" sub_value="true" message="{another_value}" _autoremap="true" />
      </Sequence>
    </BehaviorTree>
  </root> )";

  factory.registerNodeType<DummyNodes::SaySomething>("SaySomething");
  factory.registerBehaviorTreeFromText(xml_text);
  auto tree = factory.createTree("MainTree");

  // Blackboard Backup
  const auto bb_backup = BlackboardBackup(tree);

  std::vector<std::vector<std::string>> expected_keys;
  for(const auto& sub : tree.subtrees)
  {
    std::vector<std::string> keys;
    for(const auto& str_view : sub->blackboard->getKeys())
    {
      keys.push_back(std::string(str_view));
    }
    expected_keys.push_back(keys);
  }

  auto status = tree.tickWhileRunning();

  ASSERT_EQ(status, BT::NodeStatus::SUCCESS);

  // Restore Blackboard
  ASSERT_EQ(bb_backup.size(), tree.subtrees.size());
  BlackboardRestore(bb_backup, tree);

  for(size_t i = 0; i < tree.subtrees.size(); i++)
  {
    const auto keys = tree.subtrees[i]->blackboard->getKeys();
    ASSERT_EQ(expected_keys[i].size(), keys.size());
    for(size_t a = 0; a < keys.size(); a++)
    {
      ASSERT_EQ(expected_keys[i][a], keys[a]);
    }
  }
  status = tree.tickWhileRunning();
  ASSERT_EQ(status, BT::NodeStatus::SUCCESS);
}

TEST(BlackboardTest, RootBlackboard)
{
  BT::BehaviorTreeFactory factory;

  const std::string xml_text = R"(
  <root BTCPP_format="4" >
    <BehaviorTree ID="SubA">
      <Sequence>
        <SubTree ID="SubB" />
        <Script code=" @var3:=3 " />
      </Sequence>
    </BehaviorTree>

    <BehaviorTree ID="SubB">
      <Sequence>
        <SaySomething message="{@msg}" />
        <Script code=" @var4:=4 " />
      </Sequence>
    </BehaviorTree>

    <BehaviorTree ID="Sub_Issue823">
      <BB_TestNode in_port="2" out_port="{@var5}" />
    </BehaviorTree>

    <BehaviorTree ID="MainTree">
      <Sequence>
        <Script code=" msg:='hello' " />
        <SubTree ID="SubA" />

        <Script code="@var5:=0" />
        <SubTree ID="Sub_Issue823" />

        <Script code=" var1:=1 " />
        <Script code=" @var2:=2 " />
      </Sequence>
    </BehaviorTree>
  </root> )";

  factory.registerNodeType<DummyNodes::SaySomething>("SaySomething");
  factory.registerNodeType<BB_TestNode>("BB_TestNode");
  factory.registerBehaviorTreeFromText(xml_text);
  auto tree = factory.createTree("MainTree");

  auto status = tree.tickWhileRunning();
  ASSERT_EQ(status, BT::NodeStatus::SUCCESS);

  ASSERT_EQ(1, tree.rootBlackboard()->get<int>("var1"));
  ASSERT_EQ(2, tree.rootBlackboard()->get<int>("var2"));
  ASSERT_EQ(3, tree.rootBlackboard()->get<int>("var3"));
  ASSERT_EQ(4, tree.rootBlackboard()->get<int>("var4"));
  ASSERT_EQ(4, tree.rootBlackboard()->get<int>("var5"));
}

TEST(BlackboardTest, TimestampedInterface)
{
  auto bb = BT::Blackboard::create();

  // still empty, expected to fail
  int value;
  ASSERT_FALSE(bb->getStamped<int>("value"));
  ASSERT_FALSE(bb->getStamped("value", value));

  auto nsec_before = std::chrono::steady_clock::now().time_since_epoch().count();
  bb->set("value", 42);
  auto result = bb->getStamped<int>("value");
  auto stamp_opt = bb->getStamped<int>("value", value);

  ASSERT_EQ(result->value, 42);
  ASSERT_EQ(result->stamp.seq, 1);
  ASSERT_GE(result->stamp.time.count(), nsec_before);

  ASSERT_EQ(value, 42);
  ASSERT_TRUE(stamp_opt);
  ASSERT_EQ(stamp_opt->seq, 1);
  ASSERT_GE(stamp_opt->time.count(), nsec_before);

  //---------------------------------
  nsec_before = std::chrono::steady_clock::now().time_since_epoch().count();
  bb->set("value", 69);
  result = bb->getStamped<int>("value");
  stamp_opt = bb->getStamped<int>("value", value);

  ASSERT_EQ(result->value, 69);
  ASSERT_EQ(result->stamp.seq, 2);
  ASSERT_GE(result->stamp.time.count(), nsec_before);

  ASSERT_EQ(value, 69);
  ASSERT_TRUE(stamp_opt);
  ASSERT_EQ(stamp_opt->seq, 2);
  ASSERT_GE(stamp_opt->time.count(), nsec_before);
}

TEST(BlackboardTest, SetBlackboard_Upd_Ts_SeqId)
{
  BT::BehaviorTreeFactory factory;

  const std::string xml_text = R"(
  <root BTCPP_format="4">
    <BehaviorTree ID="MainTree">
      <Sequence>
        <Script code="other_point:=first_point" />
        <Sleep msec="5" />
        <SetBlackboard value="{second_point}" output_key="other_point" />
      </Sequence>
    </BehaviorTree>
  </root> )";

  factory.registerBehaviorTreeFromText(xml_text);
  auto tree = factory.createTree("MainTree");
  auto& blackboard = tree.subtrees.front()->blackboard;

  const Point point1 = { 2, 2 };
  const Point point2 = { 3, 3 };
  blackboard->set("first_point", point1);
  blackboard->set("second_point", point2);

  tree.tickExactlyOnce();
  const auto entry_ptr = blackboard->getEntry("other_point");
  const auto ts1 = entry_ptr->stamp;
  const auto seq_id1 = entry_ptr->sequence_id;
  tree.tickWhileRunning();
  const auto ts2 = entry_ptr->stamp;
  const auto seq_id2 = entry_ptr->sequence_id;

  ASSERT_GT(ts2.count(), ts1.count());
  ASSERT_GT(seq_id2, seq_id1);
}

TEST(BlackboardTest, SetBlackboard_ChangeType1)
{
  BT::BehaviorTreeFactory factory;

  const std::string xml_text = R"(
  <root BTCPP_format="4">
    <BehaviorTree ID="MainTree">
      <Sequence>
        <SetBlackboard value="{first_point}" output_key="other_point" />
        <Sleep msec="5" />
        <SetBlackboard value="{random_str}" output_key="other_point" />
      </Sequence>
    </BehaviorTree>
  </root> )";

  factory.registerBehaviorTreeFromText(xml_text);
  auto tree = factory.createTree("MainTree");
  auto& blackboard = tree.subtrees.front()->blackboard;

  const Point point = { 2, 7 };
  blackboard->set("first_point", point);
  blackboard->set("random_str", "Hello!");

  // First tick should succeed
  ASSERT_NO_THROW(tree.tickExactlyOnce());
  const auto entry_ptr = blackboard->getEntry("other_point");
  std::this_thread::sleep_for(std::chrono::milliseconds{ 5 });
  // Second tick should throw due to type mismatch
  EXPECT_THROW({ tree.tickWhileRunning(); }, BT::LogicError);
}

TEST(BlackboardTest, SetBlackboard_ChangeType2)
{
  BT::BehaviorTreeFactory factory;

  const std::string xml_text = R"(
  <root BTCPP_format="4">
    <BehaviorTree ID="MainTree">
      <Sequence>
        <SetBlackboard value="{first_point}" output_key="other_point" />
        <Sleep msec="5" />
        <SetBlackboard value="{random_num}" output_key="other_point" />
      </Sequence>
    </BehaviorTree>
  </root> )";

  factory.registerBehaviorTreeFromText(xml_text);
  auto tree = factory.createTree("MainTree");
  auto& blackboard = tree.subtrees.front()->blackboard;

  const Point point = { 2, 7 };
  blackboard->set("first_point", point);
  blackboard->set("random_num", 57);

  // First tick should succeed
  ASSERT_NO_THROW(tree.tickExactlyOnce());
  const auto entry_ptr = blackboard->getEntry("other_point");
  std::this_thread::sleep_for(std::chrono::milliseconds{ 5 });
  // Second tick should throw due to type mismatch
  EXPECT_THROW({ tree.tickWhileRunning(); }, BT::LogicError);
}

// Simple Action that updates an instance of Point in the blackboard
class UpdatePosition : public BT::SyncActionNode
{
public:
  UpdatePosition(const std::string& name, const BT::NodeConfig& config)
    : BT::SyncActionNode(name, config)
  {}

  BT::NodeStatus tick() override
  {
    const auto in_pos = getInput<Point>("pos_in");
    if(!in_pos.has_value())
      return BT::NodeStatus::FAILURE;
    Point _pos = in_pos.value();
    _pos.x += getInput<double>("x").value_or(0.0);
    _pos.y += getInput<double>("y").value_or(0.0);
    setOutput("pos_out", _pos);
    return BT::NodeStatus::SUCCESS;
  }

  static BT::PortsList providedPorts()
  {
    return { BT::InputPort<Point>("pos_in", { 0.0, 0.0 }, "Initial position"),
             BT::InputPort<double>("x"), BT::InputPort<double>("y"),
             BT::OutputPort<Point>("pos_out") };
  }

private:
};

TEST(BlackboardTest, SetBlackboard_WithPortRemapping)
{
  BT::BehaviorTreeFactory factory;

  const std::string xml_text = R"(
    <?xml version="1.0"?>
    <root BTCPP_format="4" main_tree_to_execute="MainTree">
      <BehaviorTree ID="MainTree">
          <Sequence>
              <SetBlackboard output_key="pos" value="0.0;0.0" />
              <Repeat num_cycles="3">
                  <Sequence>
                      <UpdatePosition pos_in="{pos}" x="0.1" y="0.2" pos_out="{pos}"/>
                      <SubTree ID="UpdPosPlus" _autoremap="true" new_pos="2.2;2.4" />
                      <Sleep msec="125"/>
                      <SetBlackboard output_key="pos" value="22.0;22.0" />
                  </Sequence>
              </Repeat>
          </Sequence>
      </BehaviorTree>
      <BehaviorTree ID="UpdPosPlus">
          <Sequence>
              <SetBlackboard output_key="pos" value="3.0;5.0" />
              <SetBlackboard output_key="pos" value="{new_pos}" />
          </Sequence>
      </BehaviorTree>
    </root>
  )";

  factory.registerNodeType<UpdatePosition>("UpdatePosition");
  factory.registerBehaviorTreeFromText(xml_text);
  auto tree = factory.createTree("MainTree");
  auto& blackboard = tree.subtrees.front()->blackboard;

  // First tick should succeed and update the value within the subtree
  ASSERT_NO_THROW(tree.tickExactlyOnce());

  const auto entry_ptr = blackboard->getEntry("pos");
  ASSERT_EQ(entry_ptr->value.type(), typeid(Point));

  const auto x = entry_ptr->value.cast<Point>().x;
  const auto y = entry_ptr->value.cast<Point>().y;
  ASSERT_EQ(x, 2.2);
  ASSERT_EQ(y, 2.4);

  // Tick till the end with no crashes
  ASSERT_NO_THROW(tree.tickWhileRunning(););
}
