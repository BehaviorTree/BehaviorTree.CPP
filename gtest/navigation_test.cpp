#include "behaviortree_cpp/xml_parsing.h"
#include "behaviortree_cpp/blackboard/blackboard_local.h"
#include <gtest/gtest.h>

using namespace BT;

// clang-format off
const std::string xml_text = R"(

<root main_tree_to_execute="BehaviorTree">
    <BehaviorTree ID="BehaviorTree">
        <FallbackStar name="root">
            <Sequence name="navigation_subtree">
                <Inverter>
                    <Condition ID="IsStuck"/>
                </Inverter>
                <SequenceStar name="navigate">
                    <Action ID="ComputePathToPose"/>
                    <Action ID="FollowPath"/>
                </SequenceStar>
            </Sequence>
            <SequenceStar name="stuck_recovery">
                <Condition ID="IsStuck"/>
                <Action ID="BackUpAndSpin"/>
            </SequenceStar>
        </FallbackStar>
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
    TestNode(const std::string& name):
      _expected_result(true),
      _tick_count(0),
      _name(name)
    { }

    void setExpectedResult(bool will_succeed)
    {
        _expected_result = will_succeed;
    }
    NodeStatus expectedResult() const
    {
        return _expected_result ? NodeStatus::SUCCESS : NodeStatus::FAILURE;
    }
    void resetTickCount() { _tick_count = 0; }
    int tickCount() const { return _tick_count;}

    NodeStatus tickImpl()
    {
        std::cout << _name << "::tick completed" << std::endl;
        _tick_count++;
        return expectedResult();
    }

  private:
    bool _expected_result;
    int _tick_count;
    std::string _name;
};

class IsStuck: public ConditionNode, public TestNode
{
  public:
    IsStuck(const std::string& name): ConditionNode(name), TestNode(name) {}

    NodeStatus tick() override
    {
        return tickImpl();
    }
};

class BackUpAndSpin: public ActionNodeBase, public TestNode
{
  public:
    BackUpAndSpin(const std::string& name): ActionNodeBase(name), TestNode(name){}

    NodeStatus tick() override
    {
        return tickImpl();
    }
    void halt() override {
        std::cout << "BackUpAndSpin::halt" << std::endl;
    }
};

class ComputePathToPose: public ActionNodeBase, public TestNode
{
  public:
    ComputePathToPose(const std::string& name): ActionNodeBase(name), TestNode(name){}

    NodeStatus tick() override
    {
        return tickImpl();
    }
    void halt() override {
        std::cout << "ComputePathToPose::halt" << std::endl;
    }
};

class FollowPath: public CoroActionNode, public TestNode
{
  public:
    FollowPath(const std::string& name): CoroActionNode(name), TestNode(name), _halted(false){}

    NodeStatus tick() override
    {
        _halted = false;
        std::cout << "FollowPath::started" << std::endl;
        auto initial_time = Now();

        // Yield for 1 second
        while( Now() < initial_time + Milliseconds(1000) )
        {
            setStatusRunningAndYield();

        }
        return tickImpl();
    }
    void halt() override {
        std::cout << "FollowPath::halt" << std::endl;
        setStatus( NodeStatus::FAILURE );
        _halted = true;
        CoroActionNode::halt();
    }

    bool wasHalted() const { return _halted; }

  private:
    bool _halted;
};

//-------------------------------------

/****************TESTS START HERE***************************/

TEST(Navigationtest, MoveBaseReocvery)
{
    BehaviorTreeFactory factory;

    factory.registerNodeType<IsStuck>("IsStuck");
    factory.registerNodeType<BackUpAndSpin>("BackUpAndSpin");
    factory.registerNodeType<ComputePathToPose>("ComputePathToPose");
    factory.registerNodeType<FollowPath>("FollowPath");

    auto tree = buildTreeFromText(factory, xml_text);

    IsStuck *first_stuck_node = nullptr;
    IsStuck *second_stuck_node = nullptr;
    BackUpAndSpin* back_spin_node = nullptr;
    ComputePathToPose* compute_node = nullptr;
    FollowPath* follow_node = nullptr;

    for (auto& node: tree.nodes)
    {
        auto ptr = node.get();
        if( dynamic_cast<IsStuck*>(ptr) )
        {
            if( !first_stuck_node )
            {
                first_stuck_node = dynamic_cast<IsStuck*>(ptr);
            }
            else{
                second_stuck_node = dynamic_cast<IsStuck*>(ptr);
            }
        }
        else if( dynamic_cast<BackUpAndSpin*>(ptr) ){
            back_spin_node = dynamic_cast<BackUpAndSpin*>(ptr);
        }
        else if( dynamic_cast<ComputePathToPose*>(ptr) ){
            compute_node = dynamic_cast<ComputePathToPose*>(ptr);
        }
        else if( dynamic_cast<FollowPath*>(ptr) ){
            follow_node = dynamic_cast<FollowPath*>(ptr);
        }
    }

    ASSERT_TRUE( first_stuck_node );
    ASSERT_TRUE( second_stuck_node );
    ASSERT_TRUE( back_spin_node );
    ASSERT_TRUE( compute_node );
    ASSERT_TRUE( follow_node );

    NodeStatus status = NodeStatus::IDLE;

    auto deadline = Now() + Milliseconds(100);

    first_stuck_node->setExpectedResult(false);

    std::cout << "-----------------------" << std::endl;
    // First case: not stuck, everything fine.

    while( status == NodeStatus::IDLE || status == NodeStatus::RUNNING )
    {
        status = tree.root_node->executeTick();
        std::this_thread::sleep_until(deadline);
        deadline = Now() + Milliseconds(100);
    }

    // SUCCESS expected
    ASSERT_EQ( status,  NodeStatus::SUCCESS );
    // IsStuck on the left branch must run several times
    ASSERT_GE( first_stuck_node->tickCount(), 9);
    // Never take the right branch (recovery)
    ASSERT_EQ( second_stuck_node->tickCount(), 0 );
    ASSERT_EQ( back_spin_node->tickCount(), 0 );

    ASSERT_EQ( compute_node->tickCount(), 1 );
    ASSERT_EQ( follow_node->tickCount(), 1 );
    ASSERT_FALSE( follow_node->wasHalted() );

    std::cout << "-----------------------" << std::endl;
    first_stuck_node->resetTickCount();
    second_stuck_node->resetTickCount();
    compute_node->resetTickCount();
    follow_node->resetTickCount();
    back_spin_node->resetTickCount();

    status = NodeStatus::IDLE;

    int cycle = 0;
    deadline = Now() + Milliseconds(100);
    while( status == NodeStatus::IDLE || status == NodeStatus::RUNNING )
    {
        if( cycle++ == 5 )
        {
            first_stuck_node->setExpectedResult(true);
            second_stuck_node->setExpectedResult(true);
        }
        status = tree.root_node->executeTick();
        std::this_thread::sleep_until(deadline);
        deadline = Now() + Milliseconds(100);
    }

    // SUCCESS expected
    ASSERT_EQ( status,  NodeStatus::SUCCESS );

    // First IsStuck must run several times
    ASSERT_GE( first_stuck_node->tickCount(), 5);
    // Second IsStuck probably only once
    ASSERT_EQ( second_stuck_node->tickCount(), 1 );
    ASSERT_EQ( back_spin_node->tickCount(), 1 );

    // compute done once and follow started but halted
    ASSERT_EQ( compute_node->tickCount(), 1 );

    ASSERT_EQ( follow_node->tickCount(), 0 ); // started but never completed
    ASSERT_TRUE( follow_node->wasHalted() );
    ASSERT_EQ( follow_node->status(),  NodeStatus::FAILURE );

}


