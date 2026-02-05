/* Copyright (C) 2018-2024 Davide Faconti, Eurecat - All Rights Reserved
 *
 *   Permission is hereby granted, free of charge, to any person obtaining a copy
 *   of this software and associated documentation files (the "Software"), to deal
 *   in the Software without restriction, including without limitation the rights
 *   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *   copies of the Software, and to permit persons to whom the Software is
 *   furnished to do so, subject to the following conditions:
 *   The above copyright notice and this permission notice shall be included in
 *   all copies or substantial portions of the Software.
 *
 *   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *   SOFTWARE.
 */

/**
 * @file gtest_port_type_rules.cpp
 * @brief Comprehensive tests for port type connection and validation rules.
 *
 * This file tests the following rules documented in docs/PORT_CONNECTION_RULES.md:
 *
 * 1. Same type ports are always compatible
 * 2. Generic ports (AnyTypeAllowed, BT::Any) are compatible with any type
 * 3. String is a "universal donor" - can connect to any typed port via convertFromString
 * 4. String creation in blackboard creates AnyTypeAllowed entry
 * 5. Type locks after first strongly-typed write
 * 6. Safe numeric casting between arithmetic types
 * 7. BT::Any bypasses type checking
 * 8. Type mismatch between strongly typed ports causes error
 */

#include "behaviortree_cpp/blackboard.h"
#include "behaviortree_cpp/bt_factory.h"

#include <gtest/gtest.h>

using namespace BT;

//------------------------------------------------------------------------------
// Custom types for testing
//------------------------------------------------------------------------------

struct TestPoint
{
  double x = 0;
  double y = 0;
  bool operator==(const TestPoint& other) const
  {
    return x == other.x && y == other.y;
  }
  bool operator!=(const TestPoint& other) const
  {
    return !(*this == other);
  }
};

// Custom type without string conversion (for testing type mismatch)
struct CustomTypeNoConversion
{
  int value = 0;
};

namespace BT
{
// Provide string conversion for TestPoint (uses semicolon separator)
template <>
[[nodiscard]] inline TestPoint convertFromString(StringView str)
{
  auto parts = splitString(str, ';');
  if(parts.size() != 2)
  {
    throw RuntimeError("invalid TestPoint format, expected 'x;y'");
  }
  TestPoint output;
  output.x = convertFromString<double>(parts[0]);
  output.y = convertFromString<double>(parts[1]);
  return output;
}
}  // namespace BT

//------------------------------------------------------------------------------
// Test node classes
//------------------------------------------------------------------------------

// Node with strongly typed int ports
class NodeWithIntPorts : public SyncActionNode
{
public:
  NodeWithIntPorts(const std::string& name, const NodeConfig& config)
    : SyncActionNode(name, config)
  {}

  NodeStatus tick() override
  {
    auto input = getInput<int>("input");
    if(input)
    {
      setOutput("output", input.value() * 2);
      return NodeStatus::SUCCESS;
    }
    return NodeStatus::FAILURE;
  }

  static PortsList providedPorts()
  {
    return { InputPort<int>("input"), OutputPort<int>("output") };
  }
};

// Node with strongly typed string ports
class NodeWithStringPorts : public SyncActionNode
{
public:
  NodeWithStringPorts(const std::string& name, const NodeConfig& config)
    : SyncActionNode(name, config)
  {}

  NodeStatus tick() override
  {
    auto input = getInput<std::string>("input");
    if(input)
    {
      setOutput("output", input.value());
      return NodeStatus::SUCCESS;
    }
    return NodeStatus::FAILURE;
  }

  static PortsList providedPorts()
  {
    return { InputPort<std::string>("input"), OutputPort<std::string>("output") };
  }
};

// Node with strongly typed double ports
class NodeWithDoublePorts : public SyncActionNode
{
public:
  NodeWithDoublePorts(const std::string& name, const NodeConfig& config)
    : SyncActionNode(name, config)
  {}

  NodeStatus tick() override
  {
    auto input = getInput<double>("input");
    if(input)
    {
      setOutput("output", input.value());
      return NodeStatus::SUCCESS;
    }
    return NodeStatus::FAILURE;
  }

  static PortsList providedPorts()
  {
    return { InputPort<double>("input"), OutputPort<double>("output") };
  }
};

// Node with generic (AnyTypeAllowed) ports
class NodeWithGenericPorts : public SyncActionNode
{
public:
  NodeWithGenericPorts(const std::string& name, const NodeConfig& config)
    : SyncActionNode(name, config)
  {}

  NodeStatus tick() override
  {
    return NodeStatus::SUCCESS;
  }

  static PortsList providedPorts()
  {
    // Ports without type parameter default to AnyTypeAllowed
    return { InputPort<>("input"), OutputPort<>("output") };
  }
};

// Node with BT::Any ports
class NodeWithAnyPorts : public SyncActionNode
{
public:
  NodeWithAnyPorts(const std::string& name, const NodeConfig& config)
    : SyncActionNode(name, config)
  {}

  NodeStatus tick() override
  {
    // Can write different types to BT::Any port
    setOutput("output", BT::Any(42));
    setOutput("output", BT::Any("hello"));
    setOutput("output", BT::Any(3.14));
    return NodeStatus::SUCCESS;
  }

  static PortsList providedPorts()
  {
    return { InputPort<BT::Any>("input"), OutputPort<BT::Any>("output") };
  }
};

// Node with TestPoint custom type ports
class NodeWithTestPointPorts : public SyncActionNode
{
public:
  NodeWithTestPointPorts(const std::string& name, const NodeConfig& config)
    : SyncActionNode(name, config)
  {}

  NodeStatus tick() override
  {
    auto input = getInput<TestPoint>("input");
    if(input)
    {
      setOutput("output", input.value());
      return NodeStatus::SUCCESS;
    }
    return NodeStatus::FAILURE;
  }

  static PortsList providedPorts()
  {
    return { InputPort<TestPoint>("input"), OutputPort<TestPoint>("output") };
  }
};

// Node with vector ports (for testing string to container conversion)
class NodeWithVectorPorts : public SyncActionNode
{
public:
  NodeWithVectorPorts(const std::string& name, const NodeConfig& config,
                      std::vector<double>* result)
    : SyncActionNode(name, config), result_(result)
  {}

  NodeStatus tick() override
  {
    auto input = getInput<std::vector<double>>("input");
    if(input && result_ != nullptr)
    {
      *result_ = input.value();
      return NodeStatus::SUCCESS;
    }
    return NodeStatus::FAILURE;
  }

  static PortsList providedPorts()
  {
    return { InputPort<std::vector<double>>("input") };
  }

private:
  std::vector<double>* result_;
};

//==============================================================================
// TEST SECTION 1: Same Type Ports (Rule 1)
//==============================================================================

TEST(PortTypeRules, SameType_IntToInt)
{
  BehaviorTreeFactory factory;
  factory.registerNodeType<NodeWithIntPorts>("NodeWithIntPorts");

  std::string xml = R"(
    <root BTCPP_format="4">
      <BehaviorTree>
        <Sequence>
          <NodeWithIntPorts input="21" output="{value}"/>
          <NodeWithIntPorts input="{value}" output="{result}"/>
        </Sequence>
      </BehaviorTree>
    </root>
  )";

  auto tree = factory.createTreeFromText(xml);
  auto status = tree.tickWhileRunning();

  ASSERT_EQ(status, NodeStatus::SUCCESS);
  ASSERT_EQ(tree.rootBlackboard()->get<int>("result"), 84);  // 21 * 2 * 2
}

TEST(PortTypeRules, SameType_StringToString)
{
  BehaviorTreeFactory factory;
  factory.registerNodeType<NodeWithStringPorts>("NodeWithStringPorts");

  std::string xml = R"(
    <root BTCPP_format="4">
      <BehaviorTree>
        <Sequence>
          <NodeWithStringPorts input="hello" output="{value}"/>
          <NodeWithStringPorts input="{value}" output="{result}"/>
        </Sequence>
      </BehaviorTree>
    </root>
  )";

  auto tree = factory.createTreeFromText(xml);
  auto status = tree.tickWhileRunning();

  ASSERT_EQ(status, NodeStatus::SUCCESS);
  ASSERT_EQ(tree.rootBlackboard()->get<std::string>("result"), "hello");
}

TEST(PortTypeRules, SameType_CustomTypeToCustomType)
{
  BehaviorTreeFactory factory;
  factory.registerNodeType<NodeWithTestPointPorts>("NodeWithTestPointPorts");

  std::string xml = R"(
    <root BTCPP_format="4">
      <BehaviorTree>
        <Sequence>
          <NodeWithTestPointPorts input="1.5;2.5" output="{point}"/>
          <NodeWithTestPointPorts input="{point}" output="{result}"/>
        </Sequence>
      </BehaviorTree>
    </root>
  )";

  auto tree = factory.createTreeFromText(xml);
  auto status = tree.tickWhileRunning();

  ASSERT_EQ(status, NodeStatus::SUCCESS);
  auto result = tree.rootBlackboard()->get<TestPoint>("result");
  ASSERT_EQ(result.x, 1.5);
  ASSERT_EQ(result.y, 2.5);
}

//==============================================================================
// TEST SECTION 2: Generic Ports (Rule 2)
//==============================================================================

TEST(PortTypeRules, GenericPort_AcceptsInt)
{
  BehaviorTreeFactory factory;
  factory.registerNodeType<NodeWithIntPorts>("NodeWithIntPorts");
  factory.registerNodeType<NodeWithGenericPorts>("NodeWithGenericPorts");

  std::string xml = R"(
    <root BTCPP_format="4">
      <BehaviorTree>
        <Sequence>
          <NodeWithIntPorts input="42" output="{value}"/>
          <NodeWithGenericPorts input="{value}"/>
        </Sequence>
      </BehaviorTree>
    </root>
  )";

  ASSERT_NO_THROW(auto tree = factory.createTreeFromText(xml));
}

TEST(PortTypeRules, GenericPort_AcceptsString)
{
  BehaviorTreeFactory factory;
  factory.registerNodeType<NodeWithStringPorts>("NodeWithStringPorts");
  factory.registerNodeType<NodeWithGenericPorts>("NodeWithGenericPorts");

  std::string xml = R"(
    <root BTCPP_format="4">
      <BehaviorTree>
        <Sequence>
          <NodeWithStringPorts input="hello" output="{value}"/>
          <NodeWithGenericPorts input="{value}"/>
        </Sequence>
      </BehaviorTree>
    </root>
  )";

  ASSERT_NO_THROW(auto tree = factory.createTreeFromText(xml));
}

TEST(PortTypeRules, GenericOutput_ToTypedInput)
{
  BehaviorTreeFactory factory;
  factory.registerNodeType<NodeWithIntPorts>("NodeWithIntPorts");
  factory.registerNodeType<NodeWithGenericPorts>("NodeWithGenericPorts");

  // Generic output connected to typed input via blackboard
  std::string xml = R"(
    <root BTCPP_format="4">
      <BehaviorTree>
        <Sequence>
          <NodeWithGenericPorts output="{value}"/>
          <NodeWithIntPorts input="{value}" output="{result}"/>
        </Sequence>
      </BehaviorTree>
    </root>
  )";

  // This should create the tree without error (types resolved at runtime)
  ASSERT_NO_THROW(auto tree = factory.createTreeFromText(xml));
}

//==============================================================================
// TEST SECTION 3: String as Universal Donor (Rule 3)
//==============================================================================

TEST(PortTypeRules, StringToInt_ViaConvertFromString)
{
  BehaviorTreeFactory factory;
  factory.registerNodeType<NodeWithIntPorts>("NodeWithIntPorts");

  // SetBlackboard creates a string entry, but NodeWithIntPorts expects int
  std::string xml = R"(
    <root BTCPP_format="4">
      <BehaviorTree>
        <Sequence>
          <SetBlackboard value="42" output_key="value"/>
          <NodeWithIntPorts input="{value}" output="{result}"/>
        </Sequence>
      </BehaviorTree>
    </root>
  )";

  auto tree = factory.createTreeFromText(xml);
  auto status = tree.tickWhileRunning();

  ASSERT_EQ(status, NodeStatus::SUCCESS);
  ASSERT_EQ(tree.rootBlackboard()->get<int>("result"), 84);  // 42 * 2
}

TEST(PortTypeRules, StringToDouble_ViaConvertFromString)
{
  BehaviorTreeFactory factory;
  factory.registerNodeType<NodeWithDoublePorts>("NodeWithDoublePorts");

  std::string xml = R"(
    <root BTCPP_format="4">
      <BehaviorTree>
        <Sequence>
          <SetBlackboard value="3.14" output_key="value"/>
          <NodeWithDoublePorts input="{value}" output="{result}"/>
        </Sequence>
      </BehaviorTree>
    </root>
  )";

  auto tree = factory.createTreeFromText(xml);
  auto status = tree.tickWhileRunning();

  ASSERT_EQ(status, NodeStatus::SUCCESS);
  ASSERT_DOUBLE_EQ(tree.rootBlackboard()->get<double>("result"), 3.14);
}

TEST(PortTypeRules, StringToCustomType_ViaConvertFromString)
{
  BehaviorTreeFactory factory;
  factory.registerNodeType<NodeWithTestPointPorts>("NodeWithTestPointPorts");

  // String "1.0;2.0" should convert to TestPoint via convertFromString
  std::string xml = R"(
    <root BTCPP_format="4">
      <BehaviorTree>
        <Sequence>
          <SetBlackboard value="1.0;2.0" output_key="point"/>
          <NodeWithTestPointPorts input="{point}" output="{result}"/>
        </Sequence>
      </BehaviorTree>
    </root>
  )";

  auto tree = factory.createTreeFromText(xml);
  auto status = tree.tickWhileRunning();

  ASSERT_EQ(status, NodeStatus::SUCCESS);
  auto result = tree.rootBlackboard()->get<TestPoint>("result");
  ASSERT_EQ(result.x, 1.0);
  ASSERT_EQ(result.y, 2.0);
}

TEST(PortTypeRules, StringToVector_ViaConvertFromString)
{
  BehaviorTreeFactory factory;
  std::vector<double> result;
  factory.registerNodeType<NodeWithVectorPorts>("NodeWithVectorPorts", &result);

  // Semicolon-separated string converts to vector
  std::string xml = R"(
    <root BTCPP_format="4">
      <BehaviorTree>
        <NodeWithVectorPorts input="1.0;2.0;3.0"/>
      </BehaviorTree>
    </root>
  )";

  auto tree = factory.createTreeFromText(xml);
  auto status = tree.tickWhileRunning();

  ASSERT_EQ(status, NodeStatus::SUCCESS);
  ASSERT_EQ(result.size(), 3u);
  ASSERT_EQ(result[0], 1.0);
  ASSERT_EQ(result[1], 2.0);
  ASSERT_EQ(result[2], 3.0);
}

TEST(PortTypeRules, SubtreeStringInput_ToTypedPort)
{
  BehaviorTreeFactory factory;
  std::vector<double> result;
  factory.registerNodeType<NodeWithVectorPorts>("NodeWithVectorPorts", &result);

  // String passed to subtree, then used by typed port
  std::string xml = R"(
    <root BTCPP_format="4">
      <BehaviorTree ID="Main">
        <SubTree ID="Sub" values="3;7"/>
      </BehaviorTree>
      <BehaviorTree ID="Sub">
        <NodeWithVectorPorts input="{values}"/>
      </BehaviorTree>
    </root>
  )";

  factory.registerBehaviorTreeFromText(xml);
  auto tree = factory.createTree("Main");
  auto status = tree.tickWhileRunning();

  ASSERT_EQ(status, NodeStatus::SUCCESS);
  ASSERT_EQ(result.size(), 2u);
  ASSERT_EQ(result[0], 3.0);
  ASSERT_EQ(result[1], 7.0);
}

//==============================================================================
// TEST SECTION 4: String Creates AnyTypeAllowed Entry (Rule 4)
//==============================================================================

TEST(PortTypeRules, BlackboardSetString_CreatesGenericEntry)
{
  auto bb = Blackboard::create();

  // Setting a string should create an AnyTypeAllowed entry
  bb->set("key", std::string("hello"));

  auto info = bb->entryInfo("key");
  ASSERT_NE(info, nullptr);

  // Entry should NOT be strongly typed (isStronglyTyped() == false)
  ASSERT_FALSE(info->isStronglyTyped());
}

TEST(PortTypeRules, BlackboardSetInt_CreatesStronglyTypedEntry)
{
  auto bb = Blackboard::create();

  bb->set("key", 42);

  auto info = bb->entryInfo("key");
  ASSERT_NE(info, nullptr);
  ASSERT_TRUE(info->isStronglyTyped());
  ASSERT_EQ(info->type(), typeid(int));
}

TEST(PortTypeRules, StringEntry_CanBecomeTyped)
{
  auto bb = Blackboard::create();

  // First set as string (creates AnyTypeAllowed)
  bb->set("key", std::string("42"));
  ASSERT_FALSE(bb->entryInfo("key")->isStronglyTyped());

  // Now set as int - should lock the type
  bb->set("key", 42);
  ASSERT_TRUE(bb->entryInfo("key")->isStronglyTyped());
  ASSERT_EQ(bb->entryInfo("key")->type(), typeid(int));
}

//==============================================================================
// TEST SECTION 5: Type Locks After First Strongly-Typed Write (Rule 5)
//==============================================================================

TEST(PortTypeRules, TypeLock_CannotChangeAfterTypedWrite)
{
  auto bb = Blackboard::create();

  // First set as int (strongly typed)
  bb->set("key", 42);
  ASSERT_TRUE(bb->entryInfo("key")->isStronglyTyped());

  // Cannot change to different type - throws RuntimeError for string (tries to convert)
  // or LogicError for incompatible types
  EXPECT_ANY_THROW(bb->set("key", std::string("hello")));
  EXPECT_ANY_THROW(bb->set("key", 3.14));
}

TEST(PortTypeRules, TypeLock_XMLTreeCreation_TypeMismatch)
{
  BehaviorTreeFactory factory;
  factory.registerNodeType<NodeWithIntPorts>("NodeWithIntPorts");
  factory.registerNodeType<NodeWithStringPorts>("NodeWithStringPorts");

  // First node creates int entry, second expects string - should fail
  std::string xml = R"(
    <root BTCPP_format="4">
      <BehaviorTree>
        <Sequence>
          <NodeWithIntPorts input="42" output="{value}"/>
          <NodeWithStringPorts input="{value}" output="{result}"/>
        </Sequence>
      </BehaviorTree>
    </root>
  )";

  EXPECT_THROW(auto tree = factory.createTreeFromText(xml), RuntimeError);
}

TEST(PortTypeRules, TypeLock_IntToDouble_Fails)
{
  BehaviorTreeFactory factory;
  factory.registerNodeType<NodeWithIntPorts>("NodeWithIntPorts");
  factory.registerNodeType<NodeWithDoublePorts>("NodeWithDoublePorts");

  // int output to double input - type mismatch
  std::string xml = R"(
    <root BTCPP_format="4">
      <BehaviorTree>
        <Sequence>
          <NodeWithIntPorts input="42" output="{value}"/>
          <NodeWithDoublePorts input="{value}" output="{result}"/>
        </Sequence>
      </BehaviorTree>
    </root>
  )";

  EXPECT_THROW(auto tree = factory.createTreeFromText(xml), RuntimeError);
}

TEST(PortTypeRules, TypeLock_CustomTypeChange_Fails)
{
  BehaviorTreeFactory factory;
  factory.registerNodeType<NodeWithTestPointPorts>("NodeWithTestPointPorts");
  factory.registerNodeType<NodeWithIntPorts>("NodeWithIntPorts");

  // TestPoint output to int input - should fail at tree creation
  std::string xml = R"(
    <root BTCPP_format="4">
      <BehaviorTree>
        <Sequence>
          <NodeWithTestPointPorts input="1;2" output="{value}"/>
          <NodeWithIntPorts input="{value}" output="{result}"/>
        </Sequence>
      </BehaviorTree>
    </root>
  )";

  // Throws either RuntimeError or LogicError depending on validation stage
  EXPECT_ANY_THROW(auto tree = factory.createTreeFromText(xml));
}

TEST(PortTypeRules, TypeLock_RuntimeTypeChange_Fails)
{
  BehaviorTreeFactory factory;
  factory.registerNodeType<NodeWithTestPointPorts>("NodeWithTestPointPorts");
  factory.registerNodeType<NodeWithStringPorts>("NodeWithStringPorts");

  std::string xml = R"(
    <root BTCPP_format="4">
      <BehaviorTree>
        <Sequence>
          <SetBlackboard value="{point_value}" output_key="other_point"/>
          <Sleep msec="5"/>
          <SetBlackboard value="{string_value}" output_key="other_point"/>
        </Sequence>
      </BehaviorTree>
    </root>
  )";

  auto tree = factory.createTreeFromText(xml);
  auto& bb = tree.subtrees.front()->blackboard;

  TestPoint point = { 2, 7 };
  bb->set("point_value", point);
  bb->set("string_value", std::string("Hello!"));

  // First tick succeeds (creates entry as TestPoint)
  ASSERT_NO_THROW(tree.tickExactlyOnce());

  std::this_thread::sleep_for(std::chrono::milliseconds{ 5 });

  // Second tick fails (tries to change TestPoint to string)
  // Throws LogicError, wrapped in NodeExecutionError with backtrace
  EXPECT_THROW(tree.tickWhileRunning(), BehaviorTreeException);
}

//==============================================================================
// TEST SECTION 6: Safe Numeric Casting (Rule 6)
//==============================================================================

TEST(PortTypeRules, SafeCast_IntToUint8_InRange)
{
  auto bb = Blackboard::create();

  // Create entry as uint8_t
  bb->set<uint8_t>("key", 100);
  ASSERT_TRUE(bb->entryInfo("key")->isStronglyTyped());

  // int(50) fits in uint8_t, should succeed
  ASSERT_NO_THROW(bb->set("key", int(50)));
  ASSERT_EQ(bb->get<uint8_t>("key"), 50);
}

TEST(PortTypeRules, SafeCast_IntToUint8_Overflow)
{
  auto bb = Blackboard::create();

  // Create entry as uint8_t
  bb->set<uint8_t>("key", 100);

  // int(300) > 255, should fail
  EXPECT_THROW(bb->set("key", int(300)), LogicError);
}

TEST(PortTypeRules, SafeCast_IntToUint8_Negative)
{
  auto bb = Blackboard::create();

  // Create entry as uint8_t
  bb->set<uint8_t>("key", 100);

  // Negative value cannot fit in unsigned type
  EXPECT_THROW(bb->set("key", int(-1)), LogicError);
}

TEST(PortTypeRules, SafeCast_DifferentIntTypes_NotAllowed)
{
  auto bb = Blackboard::create();

  // Create entry as int64_t
  bb->set<int64_t>("key", 100);

  // Even though int values fit in int64_t, different types are NOT allowed
  // Safe casting only works within the SAME conceptual type (e.g., int to uint8_t)
  EXPECT_THROW(bb->set("key", int(-1000000)), LogicError);

  // Setting same type works
  ASSERT_NO_THROW(bb->set("key", int64_t(1000000)));
}

//==============================================================================
// TEST SECTION 7: BT::Any Bypasses Type Checking (Rule 7)
//==============================================================================

TEST(PortTypeRules, BTAny_WrapperDoesNotBypassTypeCheck)
{
  auto bb = Blackboard::create();

  // Note: BT::Any(42) creates an entry of type int, NOT type BT::Any
  // The BT::Any wrapper is unwrapped when stored
  bb->set("key", BT::Any(42));

  // Cannot change to different type even with BT::Any wrapper
  // because the entry was created as int
  EXPECT_THROW(bb->set("key", BT::Any("hello")), LogicError);
}

TEST(PortTypeRules, BTAny_EntryType_AllowsDifferentTypes)
{
  auto bb = Blackboard::create();

  // Create entry explicitly as BT::Any type
  bb->createEntry("key", TypeInfo::Create<BT::Any>());

  // Now we can set different types because the entry type is BT::Any
  ASSERT_NO_THROW(bb->set("key", BT::Any(42)));
  ASSERT_NO_THROW(bb->set("key", BT::Any("hello")));
  ASSERT_NO_THROW(bb->set("key", BT::Any(3.14)));
}

TEST(PortTypeRules, BTAny_Port_AcceptsDifferentTypes)
{
  BehaviorTreeFactory factory;
  factory.registerNodeType<NodeWithAnyPorts>("NodeWithAnyPorts");
  factory.registerNodeType<NodeWithIntPorts>("NodeWithIntPorts");

  std::string xml = R"(
    <root BTCPP_format="4">
      <BehaviorTree>
        <Sequence>
          <NodeWithAnyPorts output="{value}"/>
          <NodeWithIntPorts input="{value}" output="{result}"/>
        </Sequence>
      </BehaviorTree>
    </root>
  )";

  // BT::Any output can connect to typed input
  ASSERT_NO_THROW(auto tree = factory.createTreeFromText(xml));
}

TEST(PortTypeRules, BTAny_InputPort_ReadsAsString)
{
  BehaviorTreeFactory factory;

  // Create a node that reads BT::Any as string
  class GetAnyAsString : public SyncActionNode
  {
  public:
    GetAnyAsString(const std::string& name, const NodeConfig& config, std::string* result)
      : SyncActionNode(name, config), result_(result)
    {}

    NodeStatus tick() override
    {
      auto res = getInput<std::string>("input");
      if(res)
      {
        *result_ = res.value();
        return NodeStatus::SUCCESS;
      }
      return NodeStatus::FAILURE;
    }

    static PortsList providedPorts()
    {
      return { InputPort<BT::Any>("input") };
    }

  private:
    std::string* result_;
  };

  std::string result;
  factory.registerNodeType<GetAnyAsString>("GetAnyAsString", &result);
  factory.registerNodeType<NodeWithIntPorts>("NodeWithIntPorts");

  std::string xml = R"(
    <root BTCPP_format="4">
      <BehaviorTree>
        <Sequence>
          <NodeWithIntPorts input="21" output="{value}"/>
          <GetAnyAsString input="{value}"/>
        </Sequence>
      </BehaviorTree>
    </root>
  )";

  auto tree = factory.createTreeFromText(xml);
  auto status = tree.tickWhileRunning();

  ASSERT_EQ(status, NodeStatus::SUCCESS);
  ASSERT_EQ(result, "42");  // 21 * 2 = 42, converted to string
}

//==============================================================================
// TEST SECTION 8: isStronglyTyped() Behavior
//==============================================================================

TEST(PortTypeRules, IsStronglyTyped_TypeInfo)
{
  // AnyTypeAllowed is NOT strongly typed
  TypeInfo anyType;
  ASSERT_FALSE(anyType.isStronglyTyped());

  // Specific types ARE strongly typed
  TypeInfo intType = TypeInfo::Create<int>();
  ASSERT_TRUE(intType.isStronglyTyped());

  TypeInfo stringType = TypeInfo::Create<std::string>();
  ASSERT_TRUE(stringType.isStronglyTyped());

  // BT::Any is NOT strongly typed
  TypeInfo btAnyType = TypeInfo::Create<BT::Any>();
  ASSERT_FALSE(btAnyType.isStronglyTyped());
}

TEST(PortTypeRules, GenericPortDeclaration_DefaultsToAnyTypeAllowed)
{
  // Port<>() without type should be AnyTypeAllowed
  auto [name, portInfo] = InputPort<>("test_port");

  ASSERT_FALSE(portInfo.isStronglyTyped());
  ASSERT_EQ(portInfo.type(), typeid(AnyTypeAllowed));
}

//==============================================================================
// TEST SECTION 9: Edge Cases and Complex Scenarios
//==============================================================================

TEST(PortTypeRules, GenericToTyped_ChainThroughBlackboard)
{
  BehaviorTreeFactory factory;
  factory.registerNodeType<NodeWithGenericPorts>("NodeWithGenericPorts");
  factory.registerNodeType<NodeWithIntPorts>("NodeWithIntPorts");

  // Generic port writes, then two typed ports use it
  std::string xml = R"(
    <root BTCPP_format="4">
      <BehaviorTree>
        <Sequence>
          <NodeWithIntPorts input="10" output="{value}"/>
          <NodeWithGenericPorts input="{value}" output="{generic}"/>
          <NodeWithIntPorts input="{value}" output="{result}"/>
        </Sequence>
      </BehaviorTree>
    </root>
  )";

  auto tree = factory.createTreeFromText(xml);
  auto status = tree.tickWhileRunning();

  ASSERT_EQ(status, NodeStatus::SUCCESS);
  ASSERT_EQ(tree.rootBlackboard()->get<int>("result"), 40);  // 10 * 2 * 2
}

TEST(PortTypeRules, MixedTypesWithGenericIntermediate)
{
  BehaviorTreeFactory factory;
  factory.registerNodeType<NodeWithIntPorts>("NodeWithIntPorts");
  factory.registerNodeType<NodeWithGenericPorts>("NodeWithGenericPorts");

  std::string xml = R"(
    <root BTCPP_format="4">
      <BehaviorTree>
        <Sequence>
          <NodeWithIntPorts input="42" output="{matching}"/>
          <NodeWithGenericPorts input="{matching}" output="{generic_out}"/>
          <NodeWithIntPorts input="{matching}" output="{result}"/>
        </Sequence>
      </BehaviorTree>
    </root>
  )";

  // This tests the pattern: typed -> generic -> typed should work
  ASSERT_NO_THROW(auto tree = factory.createTreeFromText(xml));
}

TEST(PortTypeRules, StringLiteralValidation_InvalidFormat)
{
  BehaviorTreeFactory factory;
  factory.registerNodeType<NodeWithIntPorts>("NodeWithIntPorts");

  // "not_a_number" cannot be converted to int
  std::string xml = R"(
    <root BTCPP_format="4">
      <BehaviorTree>
        <NodeWithIntPorts input="not_a_number" output="{result}"/>
      </BehaviorTree>
    </root>
  )";

  EXPECT_THROW(auto tree = factory.createTreeFromText(xml), LogicError);
}

TEST(PortTypeRules, StringLiteralValidation_ValidFormat)
{
  BehaviorTreeFactory factory;
  factory.registerNodeType<NodeWithIntPorts>("NodeWithIntPorts");

  // "42" can be converted to int
  std::string xml = R"(
    <root BTCPP_format="4">
      <BehaviorTree>
        <NodeWithIntPorts input="42" output="{result}"/>
      </BehaviorTree>
    </root>
  )";

  ASSERT_NO_THROW(auto tree = factory.createTreeFromText(xml));
}

TEST(PortTypeRules, CustomTypeStringLiteral_ValidFormat)
{
  BehaviorTreeFactory factory;
  factory.registerNodeType<NodeWithTestPointPorts>("NodeWithTestPointPorts");

  std::string xml = R"(
    <root BTCPP_format="4">
      <BehaviorTree>
        <NodeWithTestPointPorts input="1.5;2.5" output="{result}"/>
      </BehaviorTree>
    </root>
  )";

  auto tree = factory.createTreeFromText(xml);
  auto status = tree.tickWhileRunning();

  ASSERT_EQ(status, NodeStatus::SUCCESS);
}

TEST(PortTypeRules, CustomTypeStringLiteral_InvalidFormat)
{
  BehaviorTreeFactory factory;
  factory.registerNodeType<NodeWithTestPointPorts>("NodeWithTestPointPorts");

  // Missing second coordinate
  std::string xml = R"(
    <root BTCPP_format="4">
      <BehaviorTree>
        <NodeWithTestPointPorts input="1.5" output="{result}"/>
      </BehaviorTree>
    </root>
  )";

  EXPECT_THROW(auto tree = factory.createTreeFromText(xml), LogicError);
}

TEST(PortTypeRules, StringToDifferentTypes)
{
  BehaviorTreeFactory factory;
  factory.registerNodeType<NodeWithStringPorts>("NodeWithStringPorts");
  factory.registerNodeType<NodeWithIntPorts>("NodeWithIntPorts");
  factory.registerNodeType<NodeWithDoublePorts>("NodeWithDoublePorts");

  // Missing second coordinate
  std::string xml = R"(
    <root BTCPP_format="4">
      <BehaviorTree>
        <Sequence>
          <NodeWithStringPorts input="42" output="{value}"/>
          <NodeWithIntPorts input="{value}" output="{test_int}"/>
          <NodeWithDoublePorts input="{value}" output="{test_double}"/>
        </Sequence>
      </BehaviorTree>
    </root>
  )";

  auto tree = factory.createTreeFromText(xml);
  tree.tickWhileRunning();
  ASSERT_EQ(tree.rootBlackboard()->get<int>("test_int"), 84);
  ASSERT_DOUBLE_EQ(tree.rootBlackboard()->get<double>("test_double"), 42.0);
}

//==============================================================================
// TEST SECTION 10: Reserved Port Names
//==============================================================================

class IllegalPortNameNode : public SyncActionNode
{
public:
  IllegalPortNameNode(const std::string& name, const NodeConfig& config)
    : SyncActionNode(name, config)
  {}

  NodeStatus tick() override
  {
    return NodeStatus::SUCCESS;
  }

  static PortsList providedPorts()
  {
    // "name" is reserved and should not be allowed
    return { InputPort<std::string>("name") };
  }
};

TEST(PortTypeRules, ReservedPortName_ThrowsOnRegistration)
{
  BehaviorTreeFactory factory;

  // Should throw because "name" is a reserved port name
  EXPECT_THROW(factory.registerNodeType<IllegalPortNameNode>("IllegalPortNameNode"),
               RuntimeError);
}
