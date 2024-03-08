#include <gtest/gtest.h>

#include "behaviortree_cpp/scripting/operators.hpp"
#include "behaviortree_cpp/bt_factory.h"
#include "../sample_nodes/dummy_nodes.h"
#include "test_helper.hpp"

#include <lexy/input/string_input.hpp>

BT::Any GetScriptResult(BT::Ast::Environment& environment, const char* text)
{
  auto input = lexy::zstring_input<lexy::utf8_encoding>(text);
  auto result = lexy::parse<BT::Grammar::stmt>(input, lexy_ext::report_error);

  if(result.has_value())
  {
    auto exprs = LEXY_MOV(result).value();
    for(auto i = 0u; i < exprs.size() - 1; ++i)
    {
      exprs[i]->evaluate(environment);
    }
    return exprs.back()->evaluate(environment);
  }
  else
  {
    return {};
  }
}

TEST(ParserTest, AnyTypes)
{
  auto Parse = [](const char* str) {
    return lexy::parse<BT::Grammar::AnyValue>(lexy::zstring_input(str),
                                              lexy_ext::report_error);
  };

  auto result = Parse("628");
  EXPECT_TRUE(result.is_success());
  EXPECT_EQ(result.value().cast<int>(), 628);

  result = Parse("-628");
  EXPECT_TRUE(result.is_success());
  EXPECT_EQ(result.value().cast<int>(), -628);

  result = Parse("0x100");
  EXPECT_TRUE(result.is_success());
  EXPECT_EQ(result.value().cast<int>(), 256);

  result = Parse("0X100");
  EXPECT_TRUE(result.is_success());
  EXPECT_EQ(result.value().cast<int>(), 256);

  result = Parse("3.14");
  EXPECT_TRUE(result.is_success());
  EXPECT_EQ(result.value().cast<double>(), 3.14);

  result = Parse("-3.14");
  EXPECT_TRUE(result.is_success());
  EXPECT_EQ(result.value().cast<double>(), -3.14);

  result = Parse("3.14e2");
  EXPECT_TRUE(result.is_success());
  EXPECT_EQ(result.value().cast<double>(), 314);

  result = Parse("3.14e-2");
  EXPECT_TRUE(result.is_success());
  EXPECT_EQ(result.value().cast<double>(), 0.0314);

  result = Parse("3e2");
  EXPECT_TRUE(result.is_success());
  EXPECT_EQ(result.value().cast<double>(), 300);

  result = Parse("3e-2");
  EXPECT_TRUE(result.is_success());
  EXPECT_EQ(result.value().cast<double>(), 0.03);

  result = Parse("'hello world '");
  EXPECT_TRUE(result.is_success());
  EXPECT_EQ(result.value().cast<std::string>(), "hello world ");

  result = Parse("true");
  EXPECT_TRUE(result.is_success());
  EXPECT_EQ(result.value().cast<int>(), 1);

  result = Parse("false");
  EXPECT_TRUE(result.is_success());
  EXPECT_EQ(result.value().cast<int>(), 0);
}

TEST(ParserTest, AnyTypes_Failing)
{
  std::istringstream null_sink;

  auto Parse = [](const char* str) {
    return lexy::parse<BT::Grammar::AnyValue>(lexy::zstring_input(str),
                                              lexy_ext::report_error);
  };

  EXPECT_TRUE(!Parse("0X100g").is_success());

  EXPECT_TRUE(!Parse("0X100.").is_success());

  EXPECT_TRUE(!Parse("3foo").is_success());

  EXPECT_TRUE(!Parse("65.").is_success());

  EXPECT_TRUE(!Parse("65.43foo").is_success());

  EXPECT_TRUE(!Parse("foo").is_success());
}

TEST(ParserTest, Equations)
{
  BT::Ast::Environment environment = { BT::Blackboard::create(), {} };

  auto GetResult = [&environment](const char* text) -> BT::Any {
    return GetScriptResult(environment, text);
  };
  //-------------------
  const auto& variables = environment.vars;
  EXPECT_EQ(GetResult("x:= 3; y:=5; x+y").cast<double>(), 8.0);
  EXPECT_EQ(variables->getKeys().size(), 2);
  EXPECT_EQ(variables->get<double>("x"), 3.0);
  EXPECT_EQ(variables->get<double>("y"), 5.0);

  EXPECT_EQ(GetResult("x+=1").cast<double>(), 4.0);
  EXPECT_EQ(variables->get<double>("x"), 4.0);

  EXPECT_EQ(GetResult("x += 1").cast<double>(), 5.0);
  EXPECT_EQ(variables->get<double>("x"), 5.0);

  EXPECT_EQ(GetResult("x-=1").cast<double>(), 4.0);
  EXPECT_EQ(variables->get<double>("x"), 4.0);

  EXPECT_EQ(GetResult("x -= 1").cast<double>(), 3.0);
  EXPECT_EQ(variables->get<double>("x"), 3.0);

  EXPECT_EQ(GetResult("x*=2").cast<double>(), 6.0);
  EXPECT_EQ(variables->get<double>("x"), 6.0);

  EXPECT_EQ(GetResult("-x").cast<double>(), -6.0);

  EXPECT_EQ(GetResult("x/=2").cast<double>(), 3.0);
  EXPECT_EQ(variables->get<double>("x"), 3.0);

  EXPECT_EQ(GetResult("y").cast<double>(), 5.0);
  EXPECT_EQ(GetResult("y/2").cast<double>(), 2.5);
  EXPECT_EQ(GetResult("y*2").cast<double>(), 10.0);
  EXPECT_EQ(GetResult("y-x").cast<double>(), 2.0);

  EXPECT_EQ(GetResult("y & x").cast<double>(), (5 & 3));
  EXPECT_EQ(GetResult("y | x").cast<double>(), (5 | 3));
  EXPECT_EQ(GetResult("y ^ x").cast<double>(), (5 ^ 3));

  EXPECT_ANY_THROW(auto res = GetResult("y ^ 5.1").cast<double>());

  // test string variables
  EXPECT_EQ(GetResult("A:='hello'; B:=' '; C:='world'; A+B+C").cast<std::string>(), "hell"
                                                                                    "o "
                                                                                    "worl"
                                                                                    "d");
  EXPECT_EQ(variables->getKeys().size(), 5);
  EXPECT_EQ(variables->get<std::string>("A"), "hello");
  EXPECT_EQ(variables->get<std::string>("B"), " ");
  EXPECT_EQ(variables->get<std::string>("C"), "world");

  // check that whitespaces are handled correctly
  EXPECT_TRUE(!GetResult("A= '   right'; "
                         "B= ' center '; "
                         "C= 'left    '  ")
                   .empty());

  EXPECT_EQ(variables->getKeys().size(), 5);
  EXPECT_EQ(variables->get<std::string>("A"), "   right");
  EXPECT_EQ(variables->get<std::string>("B"), " center ");
  EXPECT_EQ(variables->get<std::string>("C"), "left    ");

  // can't change the type, once created
  EXPECT_ANY_THROW(GetResult("x=A"));
  EXPECT_ANY_THROW(GetResult("x='msg'"));
  EXPECT_ANY_THROW(GetResult("A=1.0"));

  // Invalid assignments
  EXPECT_ANY_THROW(GetResult(" 'hello' = 'world' "));
  EXPECT_ANY_THROW(GetResult(" 'hello' = 2.0 "));
  EXPECT_ANY_THROW(GetResult(" 3.0 = 2.0 "));

  size_t prev_size = variables->getKeys().size();
  EXPECT_ANY_THROW(GetResult("new_var=69"));
  EXPECT_EQ(variables->getKeys().size(), prev_size);  // shouldn't increase

  // check comparisons
  EXPECT_EQ(GetResult("x < y").cast<int>(), 1);
  EXPECT_EQ(GetResult("y > x").cast<int>(), 1);
  EXPECT_EQ(GetResult("y != x").cast<int>(), 1);
  EXPECT_EQ(GetResult("y == x").cast<int>(), 0);

  EXPECT_EQ(GetResult(" 'hello' == 'hello'").cast<int>(), 1);
  EXPECT_EQ(GetResult(" 'hello' != 'world'").cast<int>(), 1);
  EXPECT_EQ(GetResult(" 'hello' < 'world'").cast<int>(), 1);
  EXPECT_EQ(GetResult(" 'hello' > 'world'").cast<int>(), 0);

  EXPECT_NE(GetResult("x > y").cast<int>(), 1);
  EXPECT_NE(GetResult("y < x").cast<int>(), 1);
  EXPECT_NE(GetResult("y == x").cast<int>(), 1);

  EXPECT_EQ(GetResult("y == x ? 'T' : 'F'").cast<std::string>(), "F");
  EXPECT_EQ(GetResult("y != x ? 'T' : 'F'").cast<std::string>(), "T");

  EXPECT_EQ(GetResult("y == x").cast<int>(), 0);
  EXPECT_EQ(GetResult("y == 5").cast<int>(), 1);
  EXPECT_EQ(GetResult("x == 3").cast<int>(), 1);

  EXPECT_EQ(GetResult(" true ").cast<int>(), 1);
  EXPECT_EQ(GetResult(" 'true' ").cast<std::string>(), "true");

  GetResult("v1:= true; v2:= false");
  EXPECT_EQ(variables->get<int>("v1"), 1);
  EXPECT_EQ(variables->get<int>("v2"), 0);

  EXPECT_EQ(GetResult(" v2 = true ").cast<int>(), 1);
  EXPECT_EQ(GetResult(" v2 = !false ").cast<int>(), 1);
  EXPECT_EQ(GetResult(" v2 = !v2 ").cast<int>(), 0);

  EXPECT_EQ(GetResult("v1 && v2").cast<int>(), 0);
  EXPECT_EQ(GetResult("v1 || v2").cast<int>(), 1);

  EXPECT_EQ(GetResult("(y == x) && (x == 3)").cast<int>(), 0);
  EXPECT_EQ(GetResult("(y == x) || (x == 3)").cast<int>(), 1);

  EXPECT_EQ(GetResult(" y == x  &&  x == 3 ").cast<int>(), 0);
  EXPECT_EQ(GetResult(" y == x  ||  x == 3 ").cast<int>(), 1);

  // we expect string to be casted to number
  EXPECT_EQ(GetResult(" par1:='3'; par2:=3; par1==par2").cast<int>(), 1);
  EXPECT_EQ(GetResult(" par1:='3'; par2:=4; par1!=par2").cast<int>(), 1);
}

TEST(ParserTest, NotInitializedComparison)
{
  BT::Ast::Environment environment = { BT::Blackboard::create(), {} };

  auto GetResult = [&environment](const char* text) -> BT::Any {
    return GetScriptResult(environment, text);
  };

  auto port_info = BT::PortInfo(BT::PortDirection::INOUT, typeid(uint8_t), {});
  environment.vars->createEntry("x", port_info);

  EXPECT_ANY_THROW(GetResult("x < 0"));
  EXPECT_ANY_THROW(GetResult("x == 0"));
  EXPECT_ANY_THROW(GetResult("x > 0"));

  EXPECT_ANY_THROW(GetResult("x + 1"));
  EXPECT_ANY_THROW(GetResult("x += 1"));
}

TEST(ParserTest, EnumsBasic)
{
  BT::Ast::Environment environment = { BT::Blackboard::create(), {} };

  auto GetResult = [&environment](const char* text) -> BT::Any {
    return GetScriptResult(environment, text);
  };

  enum Color
  {
    RED = 1,
    BLUE = 3,
    GREEN = 5
  };

  environment.enums = std::make_shared<BT::EnumsTable>();
  environment.enums->insert({ "RED", RED });
  environment.enums->insert({ "BLUE", BLUE });
  environment.enums->insert({ "GREEN", GREEN });
  GetResult("A:=RED");
  GetResult("B:=RED");
  GetResult("C:=BLUE");

  EXPECT_EQ(GetResult("A==B").cast<int>(), 1);
  EXPECT_EQ(GetResult("A!=C").cast<int>(), 1);

  EXPECT_EQ(GetResult("A").cast<Color>(), RED);
  EXPECT_EQ(GetResult("B").cast<Color>(), RED);
  EXPECT_EQ(GetResult("C").cast<Color>(), BLUE);
}

TEST(ParserTest, EnumsXML)
{
  BT::BehaviorTreeFactory factory;

  const std::string xml_text = R"(

    <root BTCPP_format="4" >
        <BehaviorTree ID="MainTree">
            <Script code = "A:=THE_ANSWER; color1:=RED; color2:=BLUE; color3:=GREEN" />
        </BehaviorTree>
    </root>)";

  enum Color
  {
    RED = 1,
    BLUE = 3,
    GREEN = 5
  };

  factory.registerScriptingEnum("THE_ANSWER", 42);
  factory.registerScriptingEnums<Color>();

  auto tree = factory.createTreeFromText(xml_text);
  const auto status = tree.tickWhileRunning();
  ASSERT_EQ(status, BT::NodeStatus::SUCCESS);

  const auto& blackboard = tree.subtrees.front()->blackboard;
  ASSERT_EQ(blackboard->get<int>("A"), 42);
  ASSERT_EQ(blackboard->get<int>("color1"), RED);
  ASSERT_EQ(blackboard->get<int>("color2"), BLUE);
  ASSERT_EQ(blackboard->get<int>("color3"), GREEN);
}

enum DeviceType
{
  BATT = 1,
  CONTROLLER = 2
};

BT::NodeStatus checkLevel(BT::TreeNode& self)
{
  double percent = self.getInput<double>("percentage").value();
  DeviceType devType;
  auto res = self.getInput("deviceType", devType);
  if(!res)
  {
    throw std::runtime_error(res.error());
  }

  if(devType == DeviceType::BATT)
  {
    self.setOutput("isLowBattery", (percent < 25));
  }
  std::cout << "Device: " << devType << " Level: " << percent << std::endl;
  return BT::NodeStatus::SUCCESS;
}

TEST(ParserTest, Enums_Issue_523)
{
  BT::BehaviorTreeFactory factory;

  const std::string xml_text = R"(
  <root BTCPP_format="4" >
    <BehaviorTree ID="PowerManagerT">
      <ReactiveSequence>
        <Script code=" deviceA:=BATT; deviceB:=CONTROLLER; battery_level:=30 "/>
        <CheckLevel deviceType="{deviceA}" percentage="{battery_level}" isLowBattery="{isLowBattery}"/>
        <SaySomething message="FIRST low batteries!" _skipIf="!isLowBattery" />

        <Script code=" battery_level:=20 "/>
        <CheckLevel deviceType="{deviceA}" percentage="{battery_level}" isLowBattery="{isLowBattery}"/>
        <SaySomething message="SECOND low batteries!" _skipIf="!isLowBattery" />
      </ReactiveSequence>
    </BehaviorTree>
  </root> )";

  factory.registerNodeType<DummyNodes::SaySomething>("SaySomething");
  factory.registerSimpleCondition(
      "CheckLevel", std::bind(checkLevel, std::placeholders::_1),
      { BT::InputPort("percentage"), BT::InputPort("deviceType"),
        BT::OutputPort("isLowBattery") });

  factory.registerScriptingEnums<DeviceType>();

  auto tree = factory.createTreeFromText(xml_text);
  const auto status = tree.tickWhileRunning();
  ASSERT_EQ(status, BT::NodeStatus::SUCCESS);

  const auto& blackboard = tree.subtrees.front()->blackboard;
  ASSERT_EQ(blackboard->get<int>("deviceA"), BATT);
  ASSERT_EQ(blackboard->get<int>("deviceB"), CONTROLLER);
  ASSERT_EQ(blackboard->get<bool>("isLowBattery"), true);
}

class SampleNode595 : public BT::SyncActionNode
{
public:
  SampleNode595(const std::string& name, const BT::NodeConfiguration& config)
    : BT::SyncActionNode(name, config)
  {}

  BT::NodeStatus tick() override
  {
    setOutput("find_enemy", 0);
    return BT::NodeStatus::SUCCESS;
  }
  static BT::PortsList providedPorts()
  {
    return { BT::OutputPort<uint8_t>("find_enemy") };
  }
};

TEST(ParserTest, Issue595)
{
  BT::BehaviorTreeFactory factory;

  const std::string xml_text = R"(
  <root BTCPP_format="4" >
    <BehaviorTree ID="PowerManagerT">
      <Sequence>
        <SampleNode595 find_enemy="{find_enemy}" />
        <TestA _skipIf="find_enemy==0"/>
      </Sequence>
    </BehaviorTree>
  </root> )";

  std::array<int, 1> counters;
  RegisterTestTick(factory, "Test", counters);
  factory.registerNodeType<SampleNode595>("SampleNode595");

  auto tree = factory.createTreeFromText(xml_text);
  const auto status = tree.tickWhileRunning();

  ASSERT_EQ(status, BT::NodeStatus::SUCCESS);
  ASSERT_EQ(0, counters[0]);
}

TEST(ParserTest, NewLine)
{
  BT::BehaviorTreeFactory factory;

  const std::string xml_text = R"(
  <root BTCPP_format="4" >
    <BehaviorTree ID="Main">
      <Script code="A:=5;&#10;B:=6"/>
    </BehaviorTree>
  </root> )";

  auto tree = factory.createTreeFromText(xml_text);
  const auto status = tree.tickWhileRunning();

  ASSERT_EQ(status, BT::NodeStatus::SUCCESS);
  ASSERT_EQ(tree.rootBlackboard()->get<int>("A"), 5);
  ASSERT_EQ(tree.rootBlackboard()->get<int>("B"), 6);
}
