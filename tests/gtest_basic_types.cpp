/* Copyright (C) 2018-2025 Davide Faconti -  All Rights Reserved
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

#include "behaviortree_cpp/basic_types.h"
#include "behaviortree_cpp/behavior_tree.h"
#include "behaviortree_cpp/bt_factory.h"

#include <sstream>

#include <gtest/gtest.h>

using namespace BT;

// ============ toStr tests ============

TEST(BasicTypes, ToStr_NodeStatus)
{
  ASSERT_EQ(toStr(NodeStatus::SUCCESS), "SUCCESS");
  ASSERT_EQ(toStr(NodeStatus::FAILURE), "FAILURE");
  ASSERT_EQ(toStr(NodeStatus::RUNNING), "RUNNING");
  ASSERT_EQ(toStr(NodeStatus::IDLE), "IDLE");
  ASSERT_EQ(toStr(NodeStatus::SKIPPED), "SKIPPED");
}

TEST(BasicTypes, ToStr_NodeStatus_Colored)
{
  // Colored versions should contain ANSI escape codes
  std::string success = toStr(NodeStatus::SUCCESS, true);
  std::string failure = toStr(NodeStatus::FAILURE, true);
  std::string running = toStr(NodeStatus::RUNNING, true);
  std::string idle = toStr(NodeStatus::IDLE, true);
  std::string skipped = toStr(NodeStatus::SKIPPED, true);

  // Check that they contain the status text and escape codes
  ASSERT_NE(success.find("SUCCESS"), std::string::npos);
  ASSERT_NE(success.find("\x1b["), std::string::npos);

  ASSERT_NE(failure.find("FAILURE"), std::string::npos);
  ASSERT_NE(failure.find("\x1b["), std::string::npos);

  ASSERT_NE(running.find("RUNNING"), std::string::npos);
  ASSERT_NE(running.find("\x1b["), std::string::npos);

  ASSERT_NE(idle.find("IDLE"), std::string::npos);
  ASSERT_NE(idle.find("\x1b["), std::string::npos);

  ASSERT_NE(skipped.find("SKIPPED"), std::string::npos);
  ASSERT_NE(skipped.find("\x1b["), std::string::npos);

  // Non-colored versions should not contain escape codes
  ASSERT_EQ(toStr(NodeStatus::SUCCESS, false).find("\x1b["), std::string::npos);
}

TEST(BasicTypes, ToStr_PortDirection)
{
  ASSERT_EQ(toStr(PortDirection::INPUT), "Input");
  ASSERT_EQ(toStr(PortDirection::OUTPUT), "Output");
  ASSERT_EQ(toStr(PortDirection::INOUT), "InOut");
}

TEST(BasicTypes, ToStr_NodeType)
{
  ASSERT_EQ(toStr(NodeType::ACTION), "Action");
  ASSERT_EQ(toStr(NodeType::CONDITION), "Condition");
  ASSERT_EQ(toStr(NodeType::DECORATOR), "Decorator");
  ASSERT_EQ(toStr(NodeType::CONTROL), "Control");
  ASSERT_EQ(toStr(NodeType::SUBTREE), "SubTree");
  ASSERT_EQ(toStr(NodeType::UNDEFINED), "Undefined");
}

TEST(BasicTypes, ToStr_Bool)
{
  ASSERT_EQ(toStr(true), "true");
  ASSERT_EQ(toStr(false), "false");
}

TEST(BasicTypes, ToStr_String)
{
  ASSERT_EQ(toStr(std::string("hello")), "hello");
  ASSERT_EQ(toStr(std::string("")), "");
}

// ============ convertFromString tests ============

TEST(BasicTypes, ConvertFromString_Int)
{
  ASSERT_EQ(convertFromString<int>("42"), 42);
  ASSERT_EQ(convertFromString<int>("-42"), -42);
  ASSERT_EQ(convertFromString<int>("0"), 0);

  ASSERT_THROW((void)convertFromString<int>("not_a_number"), RuntimeError);
  ASSERT_THROW((void)convertFromString<int>(""), RuntimeError);
}

TEST(BasicTypes, ConvertFromString_Int64)
{
  ASSERT_EQ(convertFromString<int64_t>("9223372036854775807"), INT64_MAX);
  ASSERT_EQ(convertFromString<int64_t>("-9223372036854775808"), INT64_MIN);
}

TEST(BasicTypes, ConvertFromString_UInt64)
{
  ASSERT_EQ(convertFromString<uint64_t>("18446744073709551615"), UINT64_MAX);
  ASSERT_EQ(convertFromString<uint64_t>("0"), 0ULL);
}

TEST(BasicTypes, ConvertFromString_Double)
{
  ASSERT_DOUBLE_EQ(convertFromString<double>("3.14159"), 3.14159);
  ASSERT_DOUBLE_EQ(convertFromString<double>("-2.5"), -2.5);
  ASSERT_DOUBLE_EQ(convertFromString<double>("0.0"), 0.0);

  // Invalid double throws RuntimeError
  ASSERT_THROW((void)convertFromString<double>("not_a_number"), RuntimeError);
}

TEST(BasicTypes, ConvertFromString_Bool)
{
  ASSERT_TRUE(convertFromString<bool>("true"));
  ASSERT_TRUE(convertFromString<bool>("True"));
  ASSERT_TRUE(convertFromString<bool>("TRUE"));
  ASSERT_TRUE(convertFromString<bool>("1"));

  ASSERT_FALSE(convertFromString<bool>("false"));
  ASSERT_FALSE(convertFromString<bool>("False"));
  ASSERT_FALSE(convertFromString<bool>("FALSE"));
  ASSERT_FALSE(convertFromString<bool>("0"));

  ASSERT_THROW((void)convertFromString<bool>("invalid"), RuntimeError);
}

TEST(BasicTypes, ConvertFromString_String)
{
  ASSERT_EQ(convertFromString<std::string>("hello"), "hello");
  ASSERT_EQ(convertFromString<std::string>(""), "");
  ASSERT_EQ(convertFromString<std::string>("with spaces"), "with spaces");
}

TEST(BasicTypes, ConvertFromString_NodeStatus)
{
  ASSERT_EQ(convertFromString<NodeStatus>("SUCCESS"), NodeStatus::SUCCESS);
  ASSERT_EQ(convertFromString<NodeStatus>("FAILURE"), NodeStatus::FAILURE);
  ASSERT_EQ(convertFromString<NodeStatus>("RUNNING"), NodeStatus::RUNNING);
  ASSERT_EQ(convertFromString<NodeStatus>("IDLE"), NodeStatus::IDLE);
  ASSERT_EQ(convertFromString<NodeStatus>("SKIPPED"), NodeStatus::SKIPPED);

  ASSERT_THROW((void)convertFromString<NodeStatus>("INVALID"), RuntimeError);
}

TEST(BasicTypes, ConvertFromString_NodeType)
{
  ASSERT_EQ(convertFromString<NodeType>("Action"), NodeType::ACTION);
  ASSERT_EQ(convertFromString<NodeType>("Condition"), NodeType::CONDITION);
  ASSERT_EQ(convertFromString<NodeType>("Control"), NodeType::CONTROL);
  ASSERT_EQ(convertFromString<NodeType>("Decorator"), NodeType::DECORATOR);
  ASSERT_EQ(convertFromString<NodeType>("SubTree"), NodeType::SUBTREE);
}

TEST(BasicTypes, ConvertFromString_PortDirection)
{
  ASSERT_EQ(convertFromString<PortDirection>("Input"), PortDirection::INPUT);
  ASSERT_EQ(convertFromString<PortDirection>("Output"), PortDirection::OUTPUT);
  ASSERT_EQ(convertFromString<PortDirection>("InOut"), PortDirection::INOUT);
}

// ============ splitString tests ============

TEST(BasicTypes, SplitString_Basic)
{
  auto parts = splitString("a,b,c", ',');
  ASSERT_EQ(parts.size(), 3u);
  ASSERT_EQ(parts[0], "a");
  ASSERT_EQ(parts[1], "b");
  ASSERT_EQ(parts[2], "c");
}

TEST(BasicTypes, SplitString_Empty)
{
  auto parts = splitString("", ',');
  // Empty string results in empty vector
  ASSERT_EQ(parts.size(), 0u);
}

TEST(BasicTypes, SplitString_NoDelimiter)
{
  auto parts = splitString("hello", ',');
  ASSERT_EQ(parts.size(), 1u);
  ASSERT_EQ(parts[0], "hello");
}

TEST(BasicTypes, SplitString_Whitespace)
{
  auto parts = splitString(" a , b , c ", ',');
  ASSERT_EQ(parts.size(), 3u);
  // Note: splitString does not trim whitespace
  ASSERT_EQ(parts[0], " a ");
  ASSERT_EQ(parts[1], " b ");
  ASSERT_EQ(parts[2], " c ");
}

// ============ Library version tests ============

TEST(BasicTypes, LibraryVersion)
{
  // LibraryVersionNumber should return a positive integer
  int version = LibraryVersionNumber();
  ASSERT_GT(version, 0);

  // LibraryVersionString should return a non-empty string
  const char* version_str = LibraryVersionString();
  ASSERT_NE(version_str, nullptr);
  ASSERT_GT(strlen(version_str), 0u);

  // Version string should contain dots (e.g., "4.5.0")
  std::string vs(version_str);
  ASSERT_NE(vs.find('.'), std::string::npos);
}

// ============ applyRecursiveVisitor tests ============

TEST(BehaviorTree, ApplyRecursiveVisitor)
{
  BehaviorTreeFactory factory;

  const std::string xml_text = R"(
    <root BTCPP_format="4">
       <BehaviorTree>
          <Sequence>
            <AlwaysSuccess/>
            <AlwaysFailure/>
          </Sequence>
       </BehaviorTree>
    </root>)";

  auto tree = factory.createTreeFromText(xml_text);

  // Count nodes using visitor
  int node_count = 0;
  applyRecursiveVisitor(tree.rootNode(),
                        [&node_count](const TreeNode*) { node_count++; });

  // Should have: Sequence + AlwaysSuccess + AlwaysFailure = 3 nodes
  ASSERT_EQ(node_count, 3);
}

TEST(BehaviorTree, ApplyRecursiveVisitor_MutableVersion)
{
  BehaviorTreeFactory factory;

  const std::string xml_text = R"(
    <root BTCPP_format="4">
       <BehaviorTree>
          <Sequence>
            <AlwaysSuccess/>
          </Sequence>
       </BehaviorTree>
    </root>)";

  auto tree = factory.createTreeFromText(xml_text);

  // Collect node names
  std::vector<std::string> names;
  applyRecursiveVisitor(tree.rootNode(),
                        [&names](TreeNode* node) { names.push_back(node->name()); });

  ASSERT_EQ(names.size(), 2u);
  ASSERT_EQ(names[0], "Sequence");
  ASSERT_EQ(names[1], "AlwaysSuccess");
}

TEST(BehaviorTree, ApplyRecursiveVisitor_NullNode)
{
  // Should throw when given nullptr
  ASSERT_THROW(applyRecursiveVisitor(static_cast<const TreeNode*>(nullptr),
                                     [](const TreeNode*) {}),
               LogicError);
}

// ============ printTreeRecursively tests ============

TEST(BehaviorTree, PrintTreeRecursively)
{
  BehaviorTreeFactory factory;

  const std::string xml_text = R"(
    <root BTCPP_format="4">
       <BehaviorTree>
          <Sequence name="MySequence">
            <AlwaysSuccess name="Success1"/>
          </Sequence>
       </BehaviorTree>
    </root>)";

  auto tree = factory.createTreeFromText(xml_text);

  std::stringstream ss;
  printTreeRecursively(tree.rootNode(), ss);

  std::string output = ss.str();

  // Should contain the node names
  ASSERT_NE(output.find("MySequence"), std::string::npos);
  ASSERT_NE(output.find("Success1"), std::string::npos);

  // Should have delimiters
  ASSERT_NE(output.find("----------------"), std::string::npos);
}

// ============ buildSerializedStatusSnapshot tests ============

TEST(BehaviorTree, BuildSerializedStatusSnapshot)
{
  BehaviorTreeFactory factory;

  const std::string xml_text = R"(
    <root BTCPP_format="4">
       <BehaviorTree>
          <Sequence>
            <AlwaysSuccess/>
            <AlwaysSuccess/>
          </Sequence>
       </BehaviorTree>
    </root>)";

  auto tree = factory.createTreeFromText(xml_text);

  // Tick once to set some statuses
  tree.tickOnce();

  SerializedTreeStatus snapshot;
  buildSerializedStatusSnapshot(tree.rootNode(), snapshot);

  // Should have entries for each node
  ASSERT_EQ(snapshot.size(), 3u);  // Sequence + 2 AlwaysSuccess
}

// ============ PortInfo tests ============

TEST(BasicTypes, PortInfo_Construction)
{
  // Test InputPort - returns pair<string, PortInfo>
  auto input = InputPort<int>("test_input", "description");
  ASSERT_EQ(input.first, "test_input");
  ASSERT_EQ(input.second.direction(), PortDirection::INPUT);
  ASSERT_EQ(input.second.description(), "description");

  // Test OutputPort
  auto output = OutputPort<double>("test_output", "out description");
  ASSERT_EQ(output.second.direction(), PortDirection::OUTPUT);

  // Test BidirectionalPort
  auto bidir = BidirectionalPort<std::string>("test_bidir");
  ASSERT_EQ(bidir.second.direction(), PortDirection::INOUT);
}

TEST(BasicTypes, PortInfo_DefaultValue)
{
  auto port = InputPort<int>("port_with_default", 42, "has default");
  // defaultValue() returns Any, check it's not empty
  ASSERT_FALSE(port.second.defaultValue().empty());
}

// ============ TreeNodeManifest tests ============

TEST(BasicTypes, TreeNodeManifest)
{
  TreeNodeManifest manifest;
  manifest.type = NodeType::ACTION;
  manifest.registration_ID = "TestAction";
  manifest.ports = { InputPort<int>("value"), OutputPort<std::string>("result") };

  ASSERT_EQ(manifest.type, NodeType::ACTION);
  ASSERT_EQ(manifest.registration_ID, "TestAction");
  ASSERT_EQ(manifest.ports.size(), 2u);
}

// ============ Result type tests ============

TEST(BasicTypes, Result_Success)
{
  Result result;  // Default is success
  ASSERT_TRUE(result.has_value());
}

TEST(BasicTypes, Result_Error)
{
  Result result = nonstd::make_unexpected(std::string("error message"));
  ASSERT_FALSE(result.has_value());
  ASSERT_EQ(result.error(), "error message");
}

// ============ StringView tests ============

TEST(BasicTypes, StringView_FromString)
{
  std::string str = "hello world";
  StringView sv(str);

  ASSERT_EQ(sv.size(), str.size());
  ASSERT_EQ(sv.data(), str.data());
}

TEST(BasicTypes, StringView_FromCharPtr)
{
  const char* str = "test string";
  StringView sv(str);

  ASSERT_EQ(sv.size(), strlen(str));
}
