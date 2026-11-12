/* A load call rejected after an <include> must leave the parser as it was
 * before the call (issue #1282). */

#include "test_helper.hpp"

#include "behaviortree_cpp/bt_factory.h"
#include "behaviortree_cpp/xml_parsing.h"

#include <string>
#include <tuple>
#include <vector>

#include <gtest/gtest.h>

using namespace BT;
using Names = std::vector<std::string>;

namespace
{
std::string treeXml(const std::string& body)
{
  return R"(<root BTCPP_format="4">)" + body + "</root>";
}

std::string tree(const std::string& tree_id, const std::string& node)
{
  return R"(<BehaviorTree ID=")" + tree_id + R"("><)" + node + "/></BehaviorTree>";
}

// main.xml includes a valid sub.xml, then defines MainTree with `main_node`
std::filesystem::path writeMainWithInclude(const TempSubdir& dir,
                                           const std::string& main_node)
{
  writeFile(dir.path / "sub.xml", treeXml(tree("FromInclude", "AlwaysSuccess")));
  const auto main_xml = dir.path / "main.xml";
  writeFile(main_xml,
            treeXml(R"(<include path="sub.xml"/>)" + tree("MainTree", main_node)));
  return main_xml;
}
}  // namespace

TEST(XMLParserTest, RejectedIncludeKeepsNoTree)
{
  TempSubdir dir;
  const auto main_xml = writeMainWithInclude(dir, "NotARegisteredNode");

  BehaviorTreeFactory factory;
  XMLParser parser(factory);
  EXPECT_THROW(parser.loadFromFile(main_xml), std::exception);
  EXPECT_EQ(parser.registeredBehaviorTrees(), Names{});
}

TEST(XMLParserTest, RejectedInlineIncludesKeepNoTree)
{
  TempSubdir dir;
  writeFile(dir.path / "sub.xml", treeXml(tree("FromInclude", "AlwaysSuccess")));
  const auto text = treeXml(R"(<include path=")" + (dir.path / "sub.xml").string() +
                            R"("/>)" + tree("MainTree", "NotARegisteredNode"));

  BehaviorTreeFactory factory;
  XMLParser parser(factory);
  EXPECT_THROW(parser.loadFromText(text), std::exception);
  EXPECT_EQ(parser.registeredBehaviorTrees(), Names{});
}

TEST(XMLParserTest, AcceptedIncludeKeepsItsTree)
{
  TempSubdir dir;
  const auto main_xml = writeMainWithInclude(dir, "AlwaysSuccess");

  BehaviorTreeFactory factory;
  XMLParser parser(factory);
  parser.loadFromFile(main_xml);
  EXPECT_EQ(parser.registeredBehaviorTrees(), (Names{ "FromInclude", "MainTree" }));
}

// The rejected load's documents must all be dropped: a leftover included
// document would become the first one, and its main_tree_to_execute would be
// picked for a later file that does not name its main tree.
TEST(XMLParserTest, RejectedIncludeIsNotTheFirstDocument)
{
  TempSubdir dir;
  writeFile(dir.path / "sub.xml",
            R"(<root BTCPP_format="4" main_tree_to_execute="FromInclude">)" +
                tree("FromInclude", "AlwaysSuccess") + "</root>");
  const auto main_xml = dir.path / "main.xml";
  writeFile(main_xml, treeXml(R"(<include path="sub.xml"/>)" +
                              tree("MainTree", "NotARegisteredNode")));

  BehaviorTreeFactory factory;
  EXPECT_THROW(factory.registerBehaviorTreeFromFile(main_xml), std::exception);

  // two trees and no main_tree_to_execute: ambiguous, must be rejected
  const auto ambiguous =
      treeXml(tree("A", "AlwaysSuccess") + tree("FromInclude", "AlwaysFailure"));
  EXPECT_THROW(std::ignore = factory.createTreeFromText(ambiguous), RuntimeError);
}

// A tree redefined by a valid include of a rejected file must get its
// previous definition back: the redefining document is erased.
TEST(XMLParserTest, RejectedRedefinitionRestoresPreviousTree)
{
  TempSubdir dir;
  writeFile(dir.path / "good.xml", treeXml(tree("Shared", "AlwaysSuccess")));
  writeFile(dir.path / "redefine.xml", treeXml(tree("Shared", "AlwaysFailure")));
  writeFile(dir.path / "main.xml", treeXml(R"(<include path="redefine.xml"/>)" +
                                           tree("MainTree", "NotARegisteredNode")));

  BehaviorTreeFactory factory;
  factory.registerBehaviorTreeFromFile(dir.path / "good.xml");
  EXPECT_THROW(factory.registerBehaviorTreeFromFile(dir.path / "main.xml"),
               std::exception);

  auto tree_instance = factory.createTree("Shared");
  EXPECT_EQ(tree_instance.tickOnce(), NodeStatus::SUCCESS);
}
