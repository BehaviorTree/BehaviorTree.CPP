#include "behaviortree_cpp/bt_factory.h"
#include "dummy_nodes.h"

using namespace BT;

// clang-format off
static const char* xml_text = R"(
 <root BTCPP_format="4">
     <BehaviorTree>
        <Sequence>
            <Script code=" msg:='hello world' " />
            <Script code=" A:=THE_ANSWER; B:=3.14; color:=RED " />
            <Precondition if="A>-B && color != BLUE" else="FAILURE">
                <Sequence>
                  <SaySomething message="{A}"/>
                  <SaySomething message="{B}"/>
                  <SaySomething message="{msg}"/>
                  <SaySomething message="{color}"/>
                </Sequence>
            </Precondition>
        </Sequence>
     </BehaviorTree>
 </root>
 )";

// clang-format on

int main()
{
  BehaviorTreeFactory factory;
  factory.registerNodeType<DummyNodes::SaySomething>("SaySomething");

  enum Color
  {
    RED = 1,
    BLUE = 2,
    GREEN = 3
  };
  // We can add these enums to the scripting language
  factory.registerScriptingEnums<Color>();

  // Or we can do it manually
  factory.registerScriptingEnum("THE_ANSWER", 42);

  auto tree = factory.createTreeFromText(xml_text);
  tree.tickWhileRunning();

  return 0;
}

/* Expected output:

Robot says: 42.000000
Robot says: 3.140000
Robot says: hello world
Robot says: 1.000000

*/
