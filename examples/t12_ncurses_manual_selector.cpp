#include "behaviortree_cpp_v3/bt_factory.h"
#include "dummy_nodes.h"

using namespace BT;

// clang-format off
static const char* xml_text = R"(
 <root main_tree_to_execute = "MainTree" >
     <BehaviorTree ID="MainTree">
        <ManualSelector name="root">
            <SaySomething name="Option1"    message="Option1" />
            <SaySomething name="Option2"    message="Option2" />
            <SaySomething name="Option3"    message="Option3" />
            <SaySomething name="Option4"    message="Option4" />
            <ManualSelector name="YouChoose" />
        </ManualSelector>
     </BehaviorTree>
 </root>
 )";
// clang-format on

int main()
{
    BehaviorTreeFactory factory;
    factory.registerNodeType<DummyNodes::SaySomething>("SaySomething");

    auto tree = factory.createTreeFromText(xml_text);
    auto ret = tree.tickRoot();

    std::cout << "Result: " << ret << std::endl;

    return 0;
}


