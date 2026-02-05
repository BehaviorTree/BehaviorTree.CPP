#include "test_helper.hpp"

#include "behaviortree_cpp/bt_factory.h"
#include "behaviortree_cpp/scripting/operators.hpp"

#include <gtest/gtest.h>

#include "../sample_nodes/dummy_nodes.h"

namespace
{

BT::Any GetScriptResult(BT::Ast::Environment& environment, const char* text)
{
  auto exprs = BT::Scripting::parseStatements(text);
  if(exprs.empty())
  {
    return {};
  }
  for(size_t i = 0; i < exprs.size() - 1; ++i)
  {
    exprs[i]->evaluate(environment);
  }
  return exprs.back()->evaluate(environment);
}

}  // namespace

TEST(ParserTest, AnyTypes)
{
  BT::Ast::Environment env = { BT::Blackboard::create(), {} };

  auto Parse = [&env](const char* str) { return BT::ParseScriptAndExecute(env, str); };

  auto result = Parse("628");
  EXPECT_TRUE(result.has_value());
  EXPECT_EQ(result.value().cast<int>(), 628);

  result = Parse("-628");
  EXPECT_TRUE(result.has_value());
  EXPECT_EQ(result.value().cast<int>(), -628);

  result = Parse("0x100");
  EXPECT_TRUE(result.has_value());
  EXPECT_EQ(result.value().cast<int>(), 256);

  result = Parse("0X100");
  EXPECT_TRUE(result.has_value());
  EXPECT_EQ(result.value().cast<int>(), 256);

  result = Parse("3.14");
  EXPECT_TRUE(result.has_value());
  EXPECT_EQ(result.value().cast<double>(), 3.14);

  result = Parse("-3.14");
  EXPECT_TRUE(result.has_value());
  EXPECT_EQ(result.value().cast<double>(), -3.14);

  result = Parse("3.14e2");
  EXPECT_TRUE(result.has_value());
  EXPECT_EQ(result.value().cast<double>(), 314);

  result = Parse("3.14e-2");
  EXPECT_TRUE(result.has_value());
  EXPECT_EQ(result.value().cast<double>(), 0.0314);

  result = Parse("3e2");
  EXPECT_TRUE(result.has_value());
  EXPECT_EQ(result.value().cast<double>(), 300);

  result = Parse("3e-2");
  EXPECT_TRUE(result.has_value());
  EXPECT_EQ(result.value().cast<double>(), 0.03);

  result = Parse("'hello world '");
  EXPECT_TRUE(result.has_value());
  EXPECT_EQ(result.value().cast<std::string>(), "hello world ");

  result = Parse("true");
  EXPECT_TRUE(result.has_value());
  EXPECT_EQ(result.value().cast<int>(), 1);

  result = Parse("false");
  EXPECT_TRUE(result.has_value());
  EXPECT_EQ(result.value().cast<int>(), 0);
}

TEST(ParserTest, AnyTypes_Failing)
{
  EXPECT_FALSE(BT::ValidateScript("0X100g"));
  EXPECT_FALSE(BT::ValidateScript("0X100."));
  EXPECT_FALSE(BT::ValidateScript("3foo"));
  EXPECT_FALSE(BT::ValidateScript("65."));
  EXPECT_FALSE(BT::ValidateScript("65.43foo"));
  // "foo" is a valid identifier (parses as ExprName), only fails at
  // evaluation when the variable doesn't exist.
  BT::Ast::Environment env = { BT::Blackboard::create(), {} };
  EXPECT_FALSE(BT::ParseScriptAndExecute(env, "foo").has_value());
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

// NOLINTNEXTLINE(misc-use-anonymous-namespace,misc-use-internal-linkage)
BT::NodeStatus checkLevel(BT::TreeNode& self)
{
  double percent = self.getInput<double>("percentage").value();
  DeviceType devType{};
  auto res = self.getInput("deviceType", devType);
  if(!res)
  {
    throw std::runtime_error(res.error());
  }

  if(devType == DeviceType::BATT)
  {
    self.setOutput("isLowBattery", (percent < 25));
  }
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

  std::array<int, 1> counters{};
  RegisterTestTick(factory, "Test", counters);
  factory.registerNodeType<SampleNode595>("SampleNode595");

  auto tree = factory.createTreeFromText(xml_text);
  const auto status = tree.tickWhileRunning();

  ASSERT_EQ(status, BT::NodeStatus::SUCCESS);
  ASSERT_EQ(0, counters[0]);
}

// https://github.com/BehaviorTree/BehaviorTree.CPP/issues/1029
TEST(ParserTest, OperatorAssociativity_Issue1029)
{
  BT::Ast::Environment environment = { BT::Blackboard::create(), {} };

  auto GetResult = [&environment](const char* text) -> BT::Any {
    return GetScriptResult(environment, text);
  };

  // Addition and subtraction are left-associative:
  // "5 - 2 + 1" should be (5-2)+1 = 4, NOT 5-(2+1) = 2
  EXPECT_EQ(GetResult("5 - 2 + 1").cast<double>(), 4.0);

  // "10 - 3 - 2" should be (10-3)-2 = 5, NOT 10-(3-2) = 9
  EXPECT_EQ(GetResult("10 - 3 - 2").cast<double>(), 5.0);

  // "2 + 3 - 1" should be (2+3)-1 = 4
  EXPECT_EQ(GetResult("2 + 3 - 1").cast<double>(), 4.0);

  // Multiplication and division are also left-associative:
  // "12 / 3 / 2" should be (12/3)/2 = 2, NOT 12/(3/2) = 8
  EXPECT_EQ(GetResult("12 / 3 / 2").cast<double>(), 2.0);

  // "12 / 3 * 2" should be (12/3)*2 = 8, NOT 12/(3*2) = 2
  EXPECT_EQ(GetResult("12 / 3 * 2").cast<double>(), 8.0);

  // Mixed precedence: "2 + 3 * 4 - 1" should be 2+(3*4)-1 = 13
  EXPECT_EQ(GetResult("2 + 3 * 4 - 1").cast<double>(), 13.0);

  // Verify string concatenation operator (..) still works after operand fix
  EXPECT_EQ(GetResult("A:='hello'; B:=' world'; A .. B").cast<std::string>(), "hello "
                                                                              "world");
  // Chained concatenation (left-associative)
  EXPECT_EQ(GetResult("A .. ' ' .. B").cast<std::string>(), "hello  world");
}

// https://github.com/BehaviorTree/BehaviorTree.CPP/issues/923
// Regression test: ValidateScript must not crash on large invalid scripts
// that produce error messages exceeding any fixed-size buffer.
TEST(ParserTest, ValidateScriptLargeError_Issue923)
{
  // Build an invalid script large enough to overflow the old 2048-byte buffer
  std::string script;
  for(int i = 0; i < 10; i++)
  {
    script += "+6e66>6666.6+66\r6>6;6e62=6+6e66>66666'; en';o';o'; en'; ";
    script += "\x7f"
              "n"
              "\x7f"
              "r;6.6+66.H>6+6"
              "\x80"
              "6"
              "\x1e"
              ";@e66";
  }
  // Must not crash (old code used a fixed char[2048] buffer causing OOB read)
  auto result = BT::ValidateScript(script);
  EXPECT_FALSE(result);  // invalid script, but no crash
}

// https://github.com/BehaviorTree/BehaviorTree.CPP/issues/832
TEST(ParserTest, CompareWithNegativeNumber_Issue832)
{
  BT::Ast::Environment environment = { BT::Blackboard::create(), {} };

  auto GetResult = [&environment](const char* text) -> BT::Any {
    return GetScriptResult(environment, text);
  };

  // "A != -1" should parse and evaluate correctly
  EXPECT_EQ(GetResult("A:=0; A!=-1").cast<int>(), 1);   // 0 != -1 is true
  EXPECT_EQ(GetResult("A:=-1; A!=-1").cast<int>(), 0);  // -1 != -1 is false
  EXPECT_EQ(GetResult("A:=0; A==-1").cast<int>(), 0);   // 0 == -1 is false
  EXPECT_EQ(GetResult("A:=0; A>-1").cast<int>(), 1);    // 0 > -1 is true
  EXPECT_EQ(GetResult("A:=0; A<-1").cast<int>(), 0);    // 0 < -1 is false

  // Also test that ValidateScript accepts these expressions
  EXPECT_TRUE(BT::ValidateScript("A:=0; A!=-1"));
  EXPECT_TRUE(BT::ValidateScript("A:=0; A>-1"));

  // Reproducer from the issue: precondition with negative literal
  BT::BehaviorTreeFactory factory;
  const std::string xml_text = R"(
  <root BTCPP_format="4">
      <BehaviorTree>
         <Sequence>
             <Script code=" A:=0 " />
             <AlwaysSuccess _failureIf="A!=-1"/>
         </Sequence>
      </BehaviorTree>
  </root>
  )";
  auto tree = factory.createTreeFromText(xml_text);
  // A==0, so A!=-1 is true, meaning _failureIf triggers => FAILURE
  auto status = tree.tickWhileRunning();
  EXPECT_EQ(status, BT::NodeStatus::FAILURE);
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

TEST(ParserTest, TokenizerEdgeCases)
{
  // Unterminated string
  EXPECT_FALSE(BT::ValidateScript("'hello"));

  // Hex edge cases
  EXPECT_FALSE(BT::ValidateScript("0x"));
  EXPECT_FALSE(BT::ValidateScript("0xG"));

  // Exponent without digits
  EXPECT_FALSE(BT::ValidateScript("3e"));
  EXPECT_FALSE(BT::ValidateScript("3e+"));

  // DotDot adjacent to integer: "65..66" should parse as 65 .. 66
  BT::Ast::Environment env = { BT::Blackboard::create(), {} };
  auto result = BT::ParseScriptAndExecute(env, "A:='65'; B:='66'; A..B");
  EXPECT_TRUE(result.has_value());
  EXPECT_EQ(result.value().cast<std::string>(), "6566");

  // Empty and whitespace-only scripts
  EXPECT_FALSE(BT::ValidateScript(""));
  EXPECT_FALSE(BT::ValidateScript("   "));
  EXPECT_FALSE(BT::ValidateScript("\t\n\r"));
}

TEST(ParserTest, ChainedComparisons)
{
  BT::Ast::Environment env = { BT::Blackboard::create(), {} };
  auto Parse = [&env](const char* str) { return BT::ParseScriptAndExecute(env, str); };

  // 1 < 2 < 3 should be true (chained: 1<2 AND 2<3)
  EXPECT_EQ(Parse("1 < 2 < 3").value().cast<int>(), 1);

  // 3 > 2 > 1 should be true
  EXPECT_EQ(Parse("3 > 2 > 1").value().cast<int>(), 1);

  // 1 < 2 > 3 should be false (1<2 is true, but 2>3 is false)
  EXPECT_EQ(Parse("1 < 2 > 3").value().cast<int>(), 0);

  // Chained equality
  EXPECT_EQ(Parse("5 == 5 == 5").value().cast<int>(), 1);
  EXPECT_EQ(Parse("5 == 5 != 3").value().cast<int>(), 1);

  // 1 <= 2 <= 3
  EXPECT_EQ(Parse("1 <= 2 <= 3").value().cast<int>(), 1);

  // 3 >= 2 >= 1
  EXPECT_EQ(Parse("3 >= 2 >= 1").value().cast<int>(), 1);
}

TEST(ParserTest, OperatorPrecedence)
{
  BT::Ast::Environment env = { BT::Blackboard::create(), {} };
  auto Parse = [&env](const char* str) { return BT::ParseScriptAndExecute(env, str); };

  // Bitwise AND binds tighter than bitwise OR
  // 6 | 3 & 5 should be 6 | (3 & 5) = 6 | 1 = 7
  EXPECT_EQ(Parse("6 | 3 & 5").value().cast<int>(), 7);

  // Bitwise OR binds tighter than logical AND
  // true && (6 | 0) should be true
  EXPECT_EQ(Parse("true && (6 | 0)").value().cast<int>(), 1);

  // Logical AND binds tighter than logical OR
  // false || true && true should be false || (true && true) = true
  EXPECT_EQ(Parse("false || true && true").value().cast<int>(), 1);

  // false && true || true should be (false && true) || true = true
  EXPECT_EQ(Parse("false && true || true").value().cast<int>(), 1);

  // Parentheses override precedence
  EXPECT_EQ(Parse("(2 + 3) * 4").value().cast<double>(), 20.0);
  EXPECT_EQ(Parse("2 * (3 + 4)").value().cast<double>(), 14.0);
}

TEST(ParserTest, UnaryOperators)
{
  BT::Ast::Environment env = { BT::Blackboard::create(), {} };
  auto Parse = [&env](const char* str) { return BT::ParseScriptAndExecute(env, str); };

  // Logical NOT
  EXPECT_EQ(Parse("!true").value().cast<int>(), 0);
  EXPECT_EQ(Parse("!false").value().cast<int>(), 1);
  EXPECT_EQ(Parse("!!true").value().cast<int>(), 1);

  // Bitwise complement
  auto result = Parse("~0");
  EXPECT_TRUE(result.has_value());
  EXPECT_EQ(result.value().cast<int64_t>(), ~int64_t(0));

  // Unary minus
  EXPECT_EQ(Parse("-(3 + 2)").value().cast<double>(), -5.0);

  // Unary minus in expressions
  EXPECT_EQ(Parse("10 + -3").value().cast<double>(), 7.0);
}

TEST(ParserTest, TernaryExpressions)
{
  BT::Ast::Environment env = { BT::Blackboard::create(), {} };
  auto Parse = [&env](const char* str) { return BT::ParseScriptAndExecute(env, str); };

  EXPECT_EQ(Parse("true ? 1 : 2").value().cast<int>(), 1);
  EXPECT_EQ(Parse("false ? 1 : 2").value().cast<int>(), 2);

  // Ternary with expressions in branches
  EXPECT_EQ(Parse("true ? 2 + 3 : 10").value().cast<double>(), 5.0);
  EXPECT_EQ(Parse("false ? 10 : 2 + 3").value().cast<double>(), 5.0);

  // Ternary with comparison as condition
  EXPECT_EQ(Parse("3 > 2 ? 'yes' : 'no'").value().cast<std::string>(), "yes");
  EXPECT_EQ(Parse("3 < 2 ? 'yes' : 'no'").value().cast<std::string>(), "no");
}

TEST(ParserTest, MultipleStatements)
{
  BT::Ast::Environment env = { BT::Blackboard::create(), {} };
  auto Parse = [&env](const char* str) { return BT::ParseScriptAndExecute(env, str); };

  // Multiple semicolons
  Parse("a:=1;;; b:=2;;");
  EXPECT_EQ(env.vars->get<double>("a"), 1.0);
  EXPECT_EQ(env.vars->get<double>("b"), 2.0);

  // Last expression is the return value
  auto result = Parse("a:=10; b:=20; a+b");
  EXPECT_EQ(result.value().cast<double>(), 30.0);
}
