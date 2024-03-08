#include <gtest/gtest.h>
#include "behaviortree_cpp/bt_factory.h"

using namespace BT;

static const char* json_text = R"(
{
  "TestNodeConfigs": {
    "TestA": {
      "async_delay": 2000,
      "return_status": "SUCCESS",
      "post_script": "msg ='message SUBSTITUED'"
    },
    "TestB": {
      "return_status": "FAILURE"
    }
  },

  "SubstitutionRules": {
    "actionA": "TestA",
    "actionB": "TestB",
    "actionC": "NotAConfig"
  }
}
 )";

TEST(Substitution, Parser)
{
  BehaviorTreeFactory factory;

  factory.loadSubstitutionRuleFromJSON(json_text);

  const auto& rules = factory.substitutionRules();

  ASSERT_EQ(rules.size(), 3);
  ASSERT_EQ(rules.count("actionA"), 1);
  ASSERT_EQ(rules.count("actionB"), 1);
  ASSERT_EQ(rules.count("actionC"), 1);

  auto configA = std::get_if<TestNodeConfig>(&rules.at("actionA"));
  ASSERT_EQ(configA->return_status, NodeStatus::SUCCESS);
  ASSERT_EQ(configA->async_delay, std::chrono::milliseconds(2000));
  ASSERT_EQ(configA->post_script, "msg ='message SUBSTITUED'");

  auto configB = std::get_if<TestNodeConfig>(&rules.at("actionB"));
  ASSERT_EQ(configB->return_status, NodeStatus::FAILURE);
  ASSERT_EQ(configB->async_delay, std::chrono::milliseconds(0));
  ASSERT_TRUE(configB->post_script.empty());

  ASSERT_EQ(*std::get_if<std::string>(&rules.at("actionC")), "NotAConfig");
}
