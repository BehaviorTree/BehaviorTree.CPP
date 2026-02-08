/* Copyright (C) 2024 Davide Faconti - All Rights Reserved
 *
 * Test for BUG-7: loadSubtreeModel crashes on null subtree_id
 * when a <SubTree> element in <TreeNodesModel> is missing the ID attribute.
 */

#include "behaviortree_cpp/bt_factory.h"

#include <gtest/gtest.h>

using namespace BT;

// BUG-7: If a <SubTree> element inside <TreeNodesModel> is missing the
// "ID" attribute, Attribute("ID") returns nullptr, which is used as a
// key in std::unordered_map — undefined behavior (null pointer dereference).
// After the fix, this should throw a descriptive RuntimeError.
TEST(XMLParserTest, SubTreeModelMissingID_Bug7)
{
  const std::string xml_text = R"(
  <root BTCPP_format="4">
    <BehaviorTree ID="MainTree">
      <AlwaysSuccess />
    </BehaviorTree>
    <TreeNodesModel>
      <SubTree>
        <input_port name="some_port"/>
      </SubTree>
    </TreeNodesModel>
  </root>
  )";

  BehaviorTreeFactory factory;

  // Before fix: crash (null pointer as map key)
  // After fix: should throw RuntimeError about missing ID
  EXPECT_THROW(factory.registerBehaviorTreeFromText(xml_text), RuntimeError);
}
