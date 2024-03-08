#include "behaviortree_cpp/xml_parsing.h"
#include "behaviortree_cpp/blackboard.h"
#include <gtest/gtest.h>

using namespace BT;

// clang-format off
static const char* xml_text = R"(

<root  BTCPP_format="4" main_tree_to_execute="BehaviorTree">
    <BehaviorTree ID="BehaviorTree">
        <Fallback name="root">

            <ReactiveSequence name="navigation_subtree">
                <Inverter>
                    <Condition ID="IsStuck"/>
                </Inverter>
                <SequenceWithMemory name="navigate">
                    <Action ID="ComputePathToPose"/>
                    <Action ID="FollowPath"/>
                </SequenceWithMemory>
            </ReactiveSequence>

            <SequenceWithMemory name="stuck_recovery">
                <Condition ID="IsStuck"/>
                <Action ID="BackUpAndSpin"/>
            </SequenceWithMemory>

        </Fallback>
    </BehaviorTree>
</root>
 )";

// clang-format on

using Milliseconds = std::chrono::milliseconds;

inline std::chrono::high_resolution_clock::time_point Now()
{
  return std::chrono::high_resolution_clock::now();
}

//--------------------------------------------

class TestNode
{
public:
  TestNode(const std::string& name) : _expected_result(true), _tick_count(0), _name(name)
  {}

  void setExpectedResult(bool will_succeed)
  {
    _expected_result = will_succeed;
  }
  NodeStatus expectedResult() const
  {
    return _expected_result ? NodeStatus::SUCCESS : NodeStatus::FAILURE;
  }
  void resetTickCount()
  {
    _tick_count = 0;
  }
  int tickCount() const
  {
    return _tick_count;
  }

  NodeStatus tickImpl()
  {
    std::cout << _name << ": " << (_expected_result ? "true" : "false") << std::endl;
    _tick_count++;
    return expectedResult();
  }

private:
  bool _expected_result;
  int _tick_count;
  std::string _name;
};

class IsStuck : public ConditionNode, public TestNode
{
public:
  IsStuck(const std::string& name) : ConditionNode(name, {}), TestNode(name)
  {}

  NodeStatus tick() override
  {
    return tickImpl();
  }
};

class BackUpAndSpin : public SyncActionNode, public TestNode
{
public:
  BackUpAndSpin(const std::string& name) : SyncActionNode(name, {}), TestNode(name)
  {}

  NodeStatus tick() override
  {
    return tickImpl();
  }
};

class ComputePathToPose : public SyncActionNode, public TestNode
{
public:
  ComputePathToPose(const std::string& name) : SyncActionNode(name, {}), TestNode(name)
  {}

  NodeStatus tick() override
  {
    return tickImpl();
  }
};

class FollowPath : public ActionNodeBase, public TestNode
{
  std::chrono::high_resolution_clock::time_point _initial_time;

public:
  FollowPath(const std::string& name)
    : ActionNodeBase(name, {}), TestNode(name), _halted(false)
  {}

  NodeStatus tick() override
  {
    if(status() == NodeStatus::IDLE)
    {
      setStatus(NodeStatus::RUNNING);
      _halted = false;
      std::cout << "FollowPath::started" << std::endl;
      _initial_time = Now();
    }

    // Yield for 1 second
    while(Now() < _initial_time + Milliseconds(600) || _halted)
    {
      return NodeStatus::RUNNING;
    }
    if(_halted)
    {
      return NodeStatus::IDLE;
    }
    return tickImpl();
  }
  void halt() override
  {
    std::cout << "FollowPath::halt" << std::endl;
    _halted = true;
  }

  bool wasHalted() const
  {
    return _halted;
  }

private:
  bool _halted;
};

//-------------------------------------

template <typename Original, typename Casted>
void TryDynamicCastPtr(Original* ptr, Casted*& destination)
{
  if(dynamic_cast<Casted*>(ptr))
  {
    destination = dynamic_cast<Casted*>(ptr);
  }
}

/****************TESTS START HERE***************************/

TEST(Navigationtest, MoveBaseRecovery)
{
  BehaviorTreeFactory factory;

  factory.registerNodeType<IsStuck>("IsStuck");
  factory.registerNodeType<BackUpAndSpin>("BackUpAndSpin");
  factory.registerNodeType<ComputePathToPose>("ComputePathToPose");
  factory.registerNodeType<FollowPath>("FollowPath");

  auto tree = factory.createTreeFromText(xml_text);

  // Need to retrieve the node pointers with dynamic cast
  // In a normal application you would NEVER want to do such a thing.
  IsStuck* first_stuck_node = nullptr;
  IsStuck* second_stuck_node = nullptr;
  BackUpAndSpin* back_spin_node = nullptr;
  ComputePathToPose* compute_node = nullptr;
  FollowPath* follow_node = nullptr;

  for(auto& subtree : tree.subtrees)
  {
    for(auto& node : subtree->nodes)
    {
      auto ptr = node.get();

      if(!first_stuck_node)
      {
        TryDynamicCastPtr(ptr, first_stuck_node);
      }
      else
      {
        TryDynamicCastPtr(ptr, second_stuck_node);
      }
      TryDynamicCastPtr(ptr, back_spin_node);
      TryDynamicCastPtr(ptr, follow_node);
      TryDynamicCastPtr(ptr, compute_node);
    }
  }

  ASSERT_TRUE(first_stuck_node);
  ASSERT_TRUE(second_stuck_node);
  ASSERT_TRUE(back_spin_node);
  ASSERT_TRUE(compute_node);
  ASSERT_TRUE(follow_node);

  std::cout << "-----------------------" << std::endl;

  // First case: not stuck, everything fine.
  NodeStatus status = NodeStatus::IDLE;

  first_stuck_node->setExpectedResult(false);

  while(status == NodeStatus::IDLE || status == NodeStatus::RUNNING)
  {
    status = tree.tickWhileRunning();
    std::this_thread::sleep_for(Milliseconds(100));
  }

  // SUCCESS expected
  ASSERT_EQ(status, NodeStatus::SUCCESS);
  // IsStuck on the left branch must run several times
  ASSERT_GE(first_stuck_node->tickCount(), 6);
  // Never take the right branch (recovery)
  ASSERT_EQ(second_stuck_node->tickCount(), 0);
  ASSERT_EQ(back_spin_node->tickCount(), 0);

  ASSERT_EQ(compute_node->tickCount(), 1);
  ASSERT_EQ(follow_node->tickCount(), 1);
  ASSERT_FALSE(follow_node->wasHalted());

  std::cout << "-----------------------" << std::endl;

  // Second case: get stuck after a while

  // Initialize everything first
  first_stuck_node->resetTickCount();
  second_stuck_node->resetTickCount();
  compute_node->resetTickCount();
  follow_node->resetTickCount();
  back_spin_node->resetTickCount();
  status = NodeStatus::IDLE;
  int cycle = 0;

  while(status == NodeStatus::IDLE || status == NodeStatus::RUNNING)
  {
    // At the fifth cycle get stuck
    if(++cycle == 2)
    {
      first_stuck_node->setExpectedResult(true);
      second_stuck_node->setExpectedResult(true);
    }
    status = tree.tickWhileRunning();
    std::this_thread::sleep_for(Milliseconds(100));
  }

  // SUCCESS expected
  ASSERT_EQ(status, NodeStatus::SUCCESS);

  // First IsStuck must run several times
  ASSERT_GE(first_stuck_node->tickCount(), 2);
  // Second IsStuck probably only once
  ASSERT_EQ(second_stuck_node->tickCount(), 1);
  ASSERT_EQ(back_spin_node->tickCount(), 1);

  // compute done once and follow started but halted
  ASSERT_EQ(compute_node->tickCount(), 1);

  ASSERT_EQ(follow_node->tickCount(), 0);  // started but never completed
  ASSERT_TRUE(follow_node->wasHalted());

  ASSERT_EQ(compute_node->status(), NodeStatus::IDLE);
  ASSERT_EQ(follow_node->status(), NodeStatus::IDLE);
  ASSERT_EQ(back_spin_node->status(), NodeStatus::IDLE);

  std::cout << "-----------------------" << std::endl;

  // Third case: execute again

  // Initialize everything first
  first_stuck_node->resetTickCount();
  second_stuck_node->resetTickCount();
  compute_node->resetTickCount();
  follow_node->resetTickCount();
  back_spin_node->resetTickCount();
  status = NodeStatus::IDLE;
  first_stuck_node->setExpectedResult(false);
  second_stuck_node->setExpectedResult(false);

  while(status == NodeStatus::IDLE || status == NodeStatus::RUNNING)
  {
    status = tree.tickWhileRunning();
    std::this_thread::sleep_for(Milliseconds(100));
  }

  // SUCCESS expected
  ASSERT_EQ(status, NodeStatus::SUCCESS);

  ASSERT_GE(first_stuck_node->tickCount(), 6);
  ASSERT_EQ(second_stuck_node->tickCount(), 0);
  ASSERT_EQ(back_spin_node->tickCount(), 0);

  ASSERT_EQ(compute_node->status(), NodeStatus::IDLE);
  ASSERT_EQ(follow_node->status(), NodeStatus::IDLE);
  ASSERT_EQ(back_spin_node->status(), NodeStatus::IDLE);
  ASSERT_FALSE(follow_node->wasHalted());
}
