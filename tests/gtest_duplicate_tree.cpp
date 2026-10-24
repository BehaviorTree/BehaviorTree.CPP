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

std::string minimalTreeXml(const std::string& tree_id, const std::string& node = "AlwaysS"
                                                                                 "uccess")
{
  return R"(
    <root BTCPP_format="4">
      <BehaviorTree ID=")" +
         tree_id + R"(">
        <)" +
         node +
         R"(/>
      </BehaviorTree>
    </root>
  )";
}

// The parser reports included files by their canonical path (symlinks
// resolved, e.g. /var -> /private/var on macOS).
std::string canonicalPath(const std::filesystem::path& p)
{
  return std::filesystem::weakly_canonical(p).string();
}

// Runs the function and returns what it printed on stdout.
template <typename Func>
std::string captureStdout(Func&& func)
{
  testing::internal::CaptureStdout();
  func();
  return testing::internal::GetCapturedStdout();
}

bool hasWarning(const std::string& output)
{
  return output.find("Warning: BehaviorTree ID") != std::string::npos;
}

}  // namespace

// A BehaviorTree ID registered again replaces the previous definition (last
// wins), with a warning when the two come from different sources.

TEST(DuplicateBehaviorTreeId, DuplicateIdAcrossFilesWarnsAndLastWins)
{
  TempSubdir dir;
  const auto path_a = dir.path / "a.xml";
  const auto path_b = dir.path / "b.xml";
  writeFile(path_a, minimalTreeXml("DupTree"));
  writeFile(path_b, minimalTreeXml("DupTree", "AlwaysFailure"));

  BehaviorTreeFactory factory;
  ASSERT_FALSE(
      hasWarning(captureStdout([&] { factory.registerBehaviorTreeFromFile(path_a); })));
  const auto output =
      captureStdout([&] { factory.registerBehaviorTreeFromFile(path_b); });
  EXPECT_TRUE(hasWarning(output)) << output;
  EXPECT_NE(output.find(canonicalPath(path_a)), std::string::npos) << output;
  EXPECT_NE(output.find(canonicalPath(path_b)), std::string::npos) << output;

  auto tree = factory.createTree("DupTree");
  EXPECT_EQ(tree.tickWhileRunning(), NodeStatus::FAILURE);
}

TEST(DuplicateBehaviorTreeId, DuplicateIdWithinSameTextWarns)
{
  const std::string xml = R"(
<root BTCPP_format="4">
  <BehaviorTree ID="foo"><AlwaysSuccess/></BehaviorTree>
  <BehaviorTree ID="foo"><AlwaysFailure/></BehaviorTree>
</root>
)";

  BehaviorTreeFactory factory;
  const auto output = captureStdout([&] { factory.registerBehaviorTreeFromText(xml); });
  EXPECT_TRUE(hasWarning(output)) << output;
  EXPECT_NE(output.find("<inline XML>"), std::string::npos) << output;

  auto tree = factory.createTree("foo");
  EXPECT_EQ(tree.tickWhileRunning(), NodeStatus::FAILURE);
}

TEST(DuplicateBehaviorTreeId, DuplicateIdAcrossFileAndTextWarns)
{
  TempSubdir dir;
  const auto path = dir.path / "one.xml";
  writeFile(path, minimalTreeXml("shared_id"));

  BehaviorTreeFactory factory;
  factory.registerBehaviorTreeFromFile(path);
  const auto output = captureStdout(
      [&] { factory.registerBehaviorTreeFromText(minimalTreeXml("shared_id")); });
  EXPECT_TRUE(hasWarning(output)) << output;
  EXPECT_NE(output.find(canonicalPath(path)), std::string::npos) << output;
  EXPECT_NE(output.find("<inline XML>"), std::string::npos) << output;
}

TEST(DuplicateBehaviorTreeId, ReloadingSameFileIsSilent)
{
  TempSubdir dir;
  const auto path = dir.path / "reload.xml";
  writeFile(path, minimalTreeXml("same"));

  BehaviorTreeFactory factory;
  factory.registerBehaviorTreeFromFile(path);
  const auto output = captureStdout([&] { factory.registerBehaviorTreeFromFile(path); });
  EXPECT_FALSE(hasWarning(output)) << output;
  ASSERT_NO_THROW(static_cast<void>(factory.createTree("same")));
}

TEST(DuplicateBehaviorTreeId, RepeatedInlineTextIsSilent)
{
  BehaviorTreeFactory factory;
  const std::string xml = minimalTreeXml("MainTree");
  const auto output = captureStdout([&] {
    factory.createTreeFromText(xml);
    factory.createTreeFromText(xml);
  });
  EXPECT_FALSE(hasWarning(output)) << output;
}

TEST(DuplicateBehaviorTreeId, DiamondIncludeIsSilent)
{
  // top includes left and right, which both include the same shared file
  TempSubdir dir;
  writeFile(dir.path / "shared.xml", minimalTreeXml("Shared"));
  for(const char* side : { "left", "right" })
  {
    writeFile(dir.path / (std::string(side) + ".xml"), std::string(R"(
<root BTCPP_format="4">
  <include path="shared.xml"/>
  <BehaviorTree ID=")") + side + R"("><SubTree ID="Shared"/></BehaviorTree>
</root>)");
  }
  writeFile(dir.path / "top.xml", R"(
<root BTCPP_format="4" main_tree_to_execute="Top">
  <include path="left.xml"/>
  <include path="right.xml"/>
  <BehaviorTree ID="Top">
    <Sequence><SubTree ID="left"/><SubTree ID="right"/></Sequence>
  </BehaviorTree>
</root>)");

  BehaviorTreeFactory factory;
  const auto output =
      captureStdout([&] { factory.registerBehaviorTreeFromFile(dir.path / "top.xml"); });
  EXPECT_FALSE(hasWarning(output)) << output;
  auto tree = factory.createTree("Top");
  EXPECT_EQ(tree.tickWhileRunning(), NodeStatus::SUCCESS);
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
  const auto output = captureStdout([&] {
    factory.registerBehaviorTreeFromFile(path_a);
    factory.registerBehaviorTreeFromFile(path_b);
  });
  EXPECT_FALSE(hasWarning(output)) << output;

  const auto ids = factory.registeredBehaviorTrees();
  ASSERT_EQ(ids.size(), 2);
  ASSERT_NO_THROW(static_cast<void>(factory.createTree("TreeA")));
  ASSERT_NO_THROW(static_cast<void>(factory.createTree("TreeB")));
}

TEST(DuplicateBehaviorTreeId, IncludeDuplicateWarns)
{
  TempSubdir dir;
  const auto inner_path = dir.path / "inner.xml";
  const auto outer_path = dir.path / "outer.xml";

  writeFile(inner_path, minimalTreeXml("foo"));
  writeFile(outer_path, R"(
<root BTCPP_format="4">
  <include path="inner.xml"/>
  <BehaviorTree ID="foo">
    <AlwaysFailure/>
  </BehaviorTree>
</root>
)");

  BehaviorTreeFactory factory;
  const auto output =
      captureStdout([&] { factory.registerBehaviorTreeFromFile(outer_path); });
  EXPECT_TRUE(hasWarning(output)) << output;
  EXPECT_NE(output.find(canonicalPath(inner_path)), std::string::npos) << output;

  // the including file is registered after its includes, so it wins
  auto tree = factory.createTree("foo");
  EXPECT_EQ(tree.tickWhileRunning(), NodeStatus::FAILURE);
}
