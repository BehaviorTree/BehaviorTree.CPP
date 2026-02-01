#include "behaviortree_cpp/bt_factory.h"
#include "behaviortree_cpp/xml_parsing.h"

static const char* xml_text = R"(

// clang-format off
 <root BTCPP_format="4" main_tree_to_execute="MainTree" >
  <BehaviorTree ID="MainTree">
    <Sequence>
        <Script   code="vect:='1,2,3,4'"/>
        <PrintVector value="{vect}"/>;
        <SubTree ID="MySub" v4="{vect}"/>
    </Sequence>
  </BehaviorTree>

  <BehaviorTree ID="MySub">
    <PrintVector value="{v4}"/>;
  </BehaviorTree>
 </root>
 )";
// clang-format on

int main(int argc, char** argv)
{
  using namespace BT;
  BehaviorTreeFactory factory;

  std::string plugin_path = "test_plugin_action.so";

  // if you don't want to use the hardcoded path, pass it as an argument
  if(argc == 2)
  {
    plugin_path = argv[1];
  }

  // load the plugin. This will register the action "PrintVector"
  factory.registerFromPlugin(plugin_path);

  // print the registered model of PrintVector
  std::cout << writeTreeNodesModelXML(factory, false) << std::endl;

  auto tree = factory.createTreeFromText(xml_text);
  tree.tickWhileRunning();

  return 0;
}
