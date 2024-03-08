#include "behaviortree_cpp/bt_factory.h"

using namespace BT;

// To demonstrate how to pass arguments by reference, we
// use a simple non-copyable object
class NoCopyObj
{
public:
  NoCopyObj(int val) : _value(val)
  {}

  NoCopyObj(const NoCopyObj&) = delete;
  NoCopyObj& operator=(const NoCopyObj&) = delete;
  int value()
  {
    return _value;
  }

private:
  int _value = 0;
};

/*
 * Sometimes, it is convenient to pass additional (static) arguments to a Node.
 * If these parameters are known at compilation time or at deployment-time
 * and they don't change at run-time, input ports are probably overkill and cumbersome.
 *
 * This tutorial demonstrates two possible ways to initialize a custom node with
 * additional arguments.
 */

// Action_A has a different constructor than the default one.
class Action_A : public SyncActionNode
{
public:
  // additional arguments passed to the constructor
  Action_A(const std::string& name, const NodeConfig& config, int arg_int,
           std::string arg_str, NoCopyObj& nc)
    : SyncActionNode(name, config), _arg1(arg_int), _arg2(arg_str), _nc(nc)
  {}

  NodeStatus tick() override
  {
    std::cout << name() << ": " << _arg1 << " / " << _arg2 << " / " << _nc.value()
              << std::endl;
    return NodeStatus::SUCCESS;
  }
  static PortsList providedPorts()
  {
    return {};
  }

private:
  int _arg1;
  std::string _arg2;
  NoCopyObj& _nc;
};

// Action_B implements an init(...) method that must be called once at the beginning.
class Action_B : public SyncActionNode
{
public:
  Action_B(const std::string& name, const NodeConfig& config)
    : SyncActionNode(name, config)
  {}

  // we want this method to be called ONCE and BEFORE the first tick()
  void initialize(int arg_int, std::string arg_str)
  {
    _arg1 = (arg_int);
    _arg2 = (arg_str);
  }

  NodeStatus tick() override
  {
    std::cout << name() << ": " << _arg1 << " / " << _arg2 << std::endl;
    return NodeStatus::SUCCESS;
  }
  static PortsList providedPorts()
  {
    return {};
  }

private:
  int _arg1;
  std::string _arg2;
};

// Simple tree, used to execute once each action.
static const char* xml_text = R"(

 <root BTCPP_format="4">
     <BehaviorTree>
        <Sequence>
            <Action_A/>
            <Action_B/>
        </Sequence>
     </BehaviorTree>
 </root>
 )";

int main()
{
  BehaviorTreeFactory factory;

  NoCopyObj non_copyable(88);

  // Passing the extra parameters to the constructor of Action_A
  // note that if you want to pass an object by ref, instead of value
  // (copy), you must use std::ref wrapper.
  factory.registerNodeType<Action_A>("Action_A", 42, "hello world",
                                     std::ref(non_copyable));

  // Action_B will require initialization
  factory.registerNodeType<Action_B>("Action_B");

  auto tree = factory.createTreeFromText(xml_text);

  auto visitor = [](TreeNode* node) {
    if(auto action_B_node = dynamic_cast<Action_B*>(node))
    {
      action_B_node->initialize(69, "interesting_value");
    }
  };
  // apply the visitor to all the nodes of the tree
  tree.applyVisitor(visitor);

  tree.tickWhileRunning();

  /* Expected output:
        Action_A: 42 / hello world / 88
        Action_B: 69 / interesting_value
  */
  return 0;
}
