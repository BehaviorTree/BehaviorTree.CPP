/* Copyright (C) 2024 Davide Faconti - All Rights Reserved
 *
 * Test for the rollback in XMLParser::loadFromFile(): a load call that is
 * rejected must not leave the trees registered by the documents it pulled in
 * through <include>.
 */

#include "behaviortree_cpp/bt_factory.h"
#include "behaviortree_cpp/xml_parsing.h"

#include <filesystem>
#include <fstream>

#include <gtest/gtest.h>

using namespace BT;

namespace
{
class TempDir
{
public:
  TempDir()
  {
    _path = std::filesystem::temp_directory_path() /
            ("bt_xmlparser_" + std::to_string(::getpid()) + "_" +
             std::to_string(reinterpret_cast<uintptr_t>(this)));
    std::filesystem::create_directories(_path);
  }

  ~TempDir()
  {
    std::error_code ec;
    std::filesystem::remove_all(_path, ec);
  }

  std::filesystem::path write(const std::string& name, const std::string& content) const
  {
    const auto path = _path / name;
    std::ofstream out(path);
    out << content;
    return path;
  }

  const std::filesystem::path& path() const
  {
    return _path;
  }

private:
  std::filesystem::path _path;
};

bool contains(const std::vector<std::string>& names, const std::string& name)
{
  return std::find(names.begin(), names.end(), name) != names.end();
}
}  // namespace

// The included document is valid and registers a tree; the including document
// is rejected afterwards. Before the fix only the rejected document was
// dropped, so the tree from the include stayed registered even though the
// load call as a whole failed.
TEST(XMLParserTest, RejectedIncludeKeepsNoTree)
{
  TempDir dir;
  dir.write("sub.xml", R"(<root BTCPP_format="4">
    <BehaviorTree ID="FromInclude">
      <AlwaysSuccess />
    </BehaviorTree>
  </root>)");

  const auto main_xml = dir.write("main.xml", R"(<root BTCPP_format="4">
    <include path="sub.xml"/>
    <BehaviorTree ID="MainTree">
      <NotARegisteredNode />
    </BehaviorTree>
  </root>)");

  BehaviorTreeFactory factory;
  XMLParser parser(factory);

  EXPECT_THROW(parser.loadFromFile(main_xml), std::exception);

  const auto registered = parser.registeredBehaviorTrees();
  EXPECT_FALSE(contains(registered, "FromInclude"))
      << "a rejected load call must not leave the included tree registered";
  EXPECT_FALSE(contains(registered, "MainTree"));
}

// The same for loadFromText(), which shares the rollback path.
TEST(XMLParserTest, RejectedInlineIncludesKeepNoTree)
{
  TempDir dir;
  dir.write("sub.xml", R"(<root BTCPP_format="4">
    <BehaviorTree ID="FromInclude">
      <AlwaysSuccess />
    </BehaviorTree>
  </root>)");

  BehaviorTreeFactory factory;
  XMLParser parser(factory);

  const std::string text = R"(<root BTCPP_format="4">
    <include path=")" + dir.path().string() +
                           R"(/sub.xml"/>
    <BehaviorTree ID="MainTree">
      <NotARegisteredNode />
    </BehaviorTree>
  </root>)";

  EXPECT_THROW(parser.loadFromText(text), std::exception);
  EXPECT_FALSE(contains(parser.registeredBehaviorTrees(), "FromInclude"));
}

// A load call that succeeds must still register everything, includes included.
TEST(XMLParserTest, AcceptedIncludeKeepsItsTree)
{
  TempDir dir;
  dir.write("sub.xml", R"(<root BTCPP_format="4">
    <BehaviorTree ID="FromInclude">
      <AlwaysSuccess />
    </BehaviorTree>
  </root>)");

  const auto main_xml = dir.write("main.xml", R"(<root BTCPP_format="4">
    <include path="sub.xml"/>
    <BehaviorTree ID="MainTree">
      <AlwaysSuccess />
    </BehaviorTree>
  </root>)");

  BehaviorTreeFactory factory;
  XMLParser parser(factory);
  parser.loadFromFile(main_xml);

  const auto registered = parser.registeredBehaviorTrees();
  EXPECT_TRUE(contains(registered, "FromInclude"));
  EXPECT_TRUE(contains(registered, "MainTree"));
}

// A rejected load that redefined a tree must put the previous definition back:
// the new element lives in the document that is about to be erased, so leaving
// it in place would leave tree_roots pointing into freed memory.
TEST(XMLParserTest, RejectedRedefinitionRestoresPreviousTree)
{
  TempDir dir;
  const auto good = dir.write("good.xml", R"(<root BTCPP_format="4">
    <BehaviorTree ID="Shared">
      <AlwaysSuccess/>
    </BehaviorTree>
  </root>)");

  const auto bad = dir.write("bad.xml", R"(<root BTCPP_format="4">
    <BehaviorTree ID="Shared">
      <NotARegisteredNode />
    </BehaviorTree>
  </root>)");

  // Use the factory's own parser: createTree() reads that one's tree_roots,
  // so this exercises the same state a user would hit.
  BehaviorTreeFactory factory;
  factory.registerBehaviorTreeFromFile(good);

  EXPECT_THROW(factory.registerBehaviorTreeFromFile(bad), std::exception);

  // The first definition must still be usable, not just present by name.
  EXPECT_NO_THROW({
    auto tree = factory.createTree("Shared");
    EXPECT_EQ(tree.tickOnce(), NodeStatus::SUCCESS);
  });
}

// Trees registered by an earlier, successful load must survive a later
// rejected one.
TEST(XMLParserTest, RollbackKeepsEarlierTrees)
{
  TempDir dir;
  const auto good = dir.write("good.xml", R"(<root BTCPP_format="4">
    <BehaviorTree ID="Good">
      <AlwaysSuccess />
    </BehaviorTree>
  </root>)");

  const auto bad = dir.write("bad.xml", R"(<root BTCPP_format="4">
    <BehaviorTree ID="Bad">
      <NotARegisteredNode />
    </BehaviorTree>
  </root>)");

  BehaviorTreeFactory factory;
  XMLParser parser(factory);
  parser.loadFromFile(good);

  EXPECT_THROW(parser.loadFromFile(bad), std::exception);

  const auto registered = parser.registeredBehaviorTrees();
  EXPECT_TRUE(contains(registered, "Good"));
  EXPECT_FALSE(contains(registered, "Bad"));
}
