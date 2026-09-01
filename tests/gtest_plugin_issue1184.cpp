#include "behaviortree_cpp/bt_factory.h"

#include <filesystem>

#include <gtest/gtest.h>

using namespace BT;

#ifndef BT_PLUGIN_ISSUE1184_PATH
#define BT_PLUGIN_ISSUE1184_PATH "plugin_issue1184.so"
#endif

class PluginIssue1184Test : public testing::Test
{
protected:
  void SetUp() override
  {
    plugin_path_ = BT_PLUGIN_ISSUE1184_PATH;

    if(!std::filesystem::exists(plugin_path_))
    {
      GTEST_SKIP() << "Plugin not found at: " << plugin_path_ << ". "
                   << "Make sure it's built before running this test.";
    }
  }

  std::string plugin_path_;
};

TEST_F(PluginIssue1184Test, VerifyXMLRejectsPluginAsyncControlInReactiveSequence)
{
  const char* xml_text = R"(
    <root BTCPP_format="4">
      <BehaviorTree ID="MainTree">
        <ReactiveSequence>
          <PluginAsyncFallback>
            <AlwaysFailure/>
            <AlwaysSuccess/>
          </PluginAsyncFallback>
          <AsyncSequence>
            <AlwaysSuccess/>
          </AsyncSequence>
        </ReactiveSequence>
      </BehaviorTree>
    </root>
  )";

  BehaviorTreeFactory factory;
  factory.registerFromPlugin(plugin_path_);

  EXPECT_THROW((void)factory.createTreeFromText(xml_text), RuntimeError);
}
