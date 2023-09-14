#include "behaviortree_cpp/bt_factory.h"
#include "t13_custom_type.hpp"

static const char* xml_text = R"(

// clang-format off
 <root BTCPP_format="4" main_tree_to_execute="MainTree" >
  <BehaviorTree ID="MainTree">
    <Sequence>
        <Script   code="vect:='1,2,3,4'"/>
        <ShowVector value="{vect}"/>;
        <SubTree ID="MySub" v4="{vect}"/>
    </Sequence>
  </BehaviorTree>

  <BehaviorTree ID="MySub">
    <ShowVector value="{v4}"/>;
  </BehaviorTree>
 </root>
 )";
// clang-format on

int main(int argc, char** argv)
{
  using namespace BT;
  BehaviorTreeFactory factory;
  factory.registerFromPlugin("t13_plugin_action.so");

  // Not mandatory, since we don't have a Groot2 publisher
  RegisterJsonDefinition<Vector4D>(ToJson);

  auto tree = factory.createTreeFromText(xml_text);
  tree.tickWhileRunning();

  return 0;
}

