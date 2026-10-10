/* Copyright (C) 2026 - duplicate BehaviorTree ID registration tests */

#include "behaviortree_cpp/bt_factory.h"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>

#include <gtest/gtest.h>

using namespace BT;

namespace
{
class TempSubdir
{
public:
  TempSubdir()
  {
    const auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
    path = std::filesystem::temp_directory_path() /
           ("bt_duplicate_tree_" + std::to_string(stamp));
    std::filesystem::create_directories(path);
  }

  ~TempSubdir()
  {
    std::error_code ec;
    std::filesystem::remove_all(path, ec);
  }

  std::filesystem::path path;

  TempSubdir(const TempSubdir&) = delete;
  TempSubdir& operator=(const TempSubdir&) = delete;
};

void writeFile(const std::filesystem::path& p, const std::string& content)
{
  std::ofstream out(p.string());
  out << content;
}

std::string minimalTreeXml(const std::string& tree_id)
{
  return std::string(R"(
    <root BTCPP_format="4">
      <BehaviorTree ID=")" +
                     tree_id + R"(">
        <AlwaysSuccess/>
      </BehaviorTree>
    </root>
  )");
}

}  // namespace

TEST(DuplicateBehaviorTreeId, DuplicateIdAcrossFiles)
{
  TempSubdir dir;
  const auto path_a = dir.path / "a.xml";
  const auto path_b = dir.path / "b.xml";
  writeFile(path_a, minimalTreeXml("DupTree"));
  writeFile(path_b, minimalTreeXml("DupTree"));

  const std::string abs_a = std::filesystem::absolute(path_a).string();
  const std::string abs_b = std::filesystem::absolute(path_b).string();

  BehaviorTreeFactory factory;
  ASSERT_NO_THROW(factory.registerBehaviorTreeFromFile(path_a));

  try
  {
    factory.registerBehaviorTreeFromFile(path_b);
    FAIL() << "expected RuntimeError";
  }
  catch(const RuntimeError& e)
  {
    const std::string msg = e.what();
    EXPECT_NE(msg.find(abs_a), std::string::npos) << msg;
    EXPECT_NE(msg.find(abs_b), std::string::npos) << msg;
  }
}

TEST(DuplicateBehaviorTreeId, DuplicateIdWithinSameText)
{
  const std::string xml = R"(
<root BTCPP_format="4">
  <BehaviorTree ID="foo"><AlwaysSuccess/></BehaviorTree>
  <BehaviorTree ID="foo"><AlwaysFailure/></BehaviorTree>
</root>
)";

  BehaviorTreeFactory factory;
  try
  {
    factory.registerBehaviorTreeFromText(xml);
    FAIL() << "expected RuntimeError";
  }
  catch(const RuntimeError& e)
  {
    const std::string msg = e.what();
    EXPECT_NE(msg.find("<inline XML>"), std::string::npos) << msg;
  }
}

TEST(DuplicateBehaviorTreeId, DuplicateIdAcrossFileAndText)
{
  TempSubdir dir;
  const auto path = dir.path / "one.xml";
  writeFile(path, minimalTreeXml("shared_id"));

  const std::string abs_path = std::filesystem::absolute(path).string();

  BehaviorTreeFactory factory;
  ASSERT_NO_THROW(factory.registerBehaviorTreeFromFile(path));

  const std::string xml = minimalTreeXml("shared_id");
  try
  {
    factory.registerBehaviorTreeFromText(xml);
    FAIL() << "expected RuntimeError";
  }
  catch(const RuntimeError& e)
  {
    const std::string msg = e.what();
    EXPECT_NE(msg.find(abs_path), std::string::npos) << msg;
    EXPECT_NE(msg.find("<inline XML>"), std::string::npos) << msg;
  }
}

TEST(DuplicateBehaviorTreeId, DuplicateIdSamePathTwice)
{
  TempSubdir dir;
  const auto path = dir.path / "reload.xml";
  writeFile(path, minimalTreeXml("same"));

  const std::string abs_path = std::filesystem::absolute(path).string();

  BehaviorTreeFactory factory;
  ASSERT_NO_THROW(factory.registerBehaviorTreeFromFile(path));

  try
  {
    factory.registerBehaviorTreeFromFile(path);
    FAIL() << "expected RuntimeError";
  }
  catch(const RuntimeError& e)
  {
    const std::string msg = e.what();
    EXPECT_NE(msg.find(abs_path), std::string::npos) << msg;
  }
}

TEST(DuplicateBehaviorTreeId, ClearAllowsReload)
{
  TempSubdir dir;
  const auto path = dir.path / "clear.xml";
  writeFile(path, minimalTreeXml("cleared"));

  BehaviorTreeFactory factory;
  ASSERT_NO_THROW(factory.registerBehaviorTreeFromFile(path));
  factory.clearRegisteredBehaviorTrees();
  ASSERT_NO_THROW(factory.registerBehaviorTreeFromFile(path));
  ASSERT_NO_THROW(static_cast<void>(factory.createTree("cleared")));
}

TEST(DuplicateBehaviorTreeId, DifferentIdsCoexist)
{
  TempSubdir dir;
  const auto path_a = dir.path / "tree_a.xml";
  const auto path_b = dir.path / "tree_b.xml";
  writeFile(path_a, minimalTreeXml("TreeA"));
  writeFile(path_b, minimalTreeXml("TreeB"));

  BehaviorTreeFactory factory;
  ASSERT_NO_THROW(factory.registerBehaviorTreeFromFile(path_a));
  ASSERT_NO_THROW(factory.registerBehaviorTreeFromFile(path_b));

  const auto ids = factory.registeredBehaviorTrees();
  ASSERT_EQ(ids.size(), 2);
  ASSERT_NO_THROW(static_cast<void>(factory.createTree("TreeA")));
  ASSERT_NO_THROW(static_cast<void>(factory.createTree("TreeB")));
}

TEST(DuplicateBehaviorTreeId, IncludeDuplicateThrows)
{
  TempSubdir dir;
  const auto inner_path = dir.path / "inner.xml";
  const auto outer_path = dir.path / "outer.xml";

  writeFile(inner_path, minimalTreeXml("foo"));

  const std::string outer_xml = R"(
<root BTCPP_format="4">
  <include path="inner.xml"/>
  <BehaviorTree ID="foo">
    <AlwaysSuccess/>
  </BehaviorTree>
</root>
)";
  writeFile(outer_path, outer_xml);

  const std::string inner_abs = std::filesystem::absolute(inner_path).string();

  BehaviorTreeFactory factory;
  try
  {
    factory.registerBehaviorTreeFromFile(std::filesystem::absolute(outer_path));
    FAIL() << "expected RuntimeError";
  }
  catch(const RuntimeError& e)
  {
    const std::string msg = e.what();
    EXPECT_NE(msg.find(inner_abs), std::string::npos) << msg;
  }
}
