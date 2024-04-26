#include "behaviortree_cpp/bt_factory.h"

using namespace BT;

/**
 * @brief Show how to access an entry in the blackboard by pointer.
 * This approach is more verbose, but thread-safe
 *
 */
class PushIntoVector : public SyncActionNode
{
public:
  PushIntoVector(const std::string& name, const NodeConfig& config)
    : SyncActionNode(name, config)
  {}

  NodeStatus tick() override
  {
    const int number = getInput<int>("value").value();
    if(auto any_ptr = getLockedPortContent("vector"))
    {
      // inside this scope (as long as any_ptr exists) the access to
      // the entry in the blackboard is mutex-protected and thread-safe.

      // check if the entry contains a valid element
      if(any_ptr->empty())
      {
        // The entry hasn't been initialized by any other node, yet.
        // Let's initialize it ourselves
        std::vector<int> vect = { number };
        any_ptr.assign(vect);
        std::cout << "We created a new vector, containing value [" << number << "]\n";
      }
      else if(auto* vect_ptr = any_ptr->castPtr<std::vector<int>>())
      {
        // NOTE: vect_ptr would be nullptr, if we try to cast it to the wrong type
        vect_ptr->push_back(number);
        std::cout << "Value [" << number
                  << "] pushed into the vector. New size: " << vect_ptr->size() << "\n";
      }
      return NodeStatus::SUCCESS;
    }
    else
    {
      return NodeStatus::FAILURE;
    }
  }

  static PortsList providedPorts()
  {
    return { BT::BidirectionalPort<std::vector<int>>("vector"), BT::InputPort<int>("valu"
                                                                                   "e") };
  }
};

//--------------------------------------------------------------

// clang-format off
static const char* xml_tree = R"(
  <root BTCPP_format="4" >
    <BehaviorTree ID="TreeA">
      <Sequence>
        <PushIntoVector vector="{vect}" value="3"/>
        <PushIntoVector vector="{vect}" value="5"/>
        <PushIntoVector vector="{vect}" value="7"/>
      </Sequence>
    </BehaviorTree>
 </root>
 )";

// clang-format on

int main()
{
  BehaviorTreeFactory factory;
  factory.registerNodeType<PushIntoVector>("PushIntoVector");

  auto tree = factory.createTreeFromText(xml_tree);
  tree.tickWhileRunning();
  return 0;
}
