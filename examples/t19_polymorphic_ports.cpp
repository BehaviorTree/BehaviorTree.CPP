#include "behaviortree_cpp/bt_factory.h"

using namespace BT;

/* This tutorial shows how to use polymorphic ports.
 *
 * When nodes produce and consume shared_ptr<T> via ports,
 * you may want a node that outputs shared_ptr<Derived> to feed
 * into a node that expects shared_ptr<Base>.
 *
 * By registering the inheritance relationship with
 * factory.registerPolymorphicCast<Derived, Base>(), the library
 * handles the upcast automatically — both at tree-creation time
 * (port type validation) and at runtime (getInput / get).
 *
 * Transitive casts are supported: if you register A->B and B->C,
 * then A->C works automatically.
 */

//--------------------------------------------------------------
// A simple class hierarchy
//--------------------------------------------------------------

class Animal
{
public:
  using Ptr = std::shared_ptr<Animal>;
  virtual ~Animal() = default;

  virtual std::string name() const
  {
    return "Animal";
  }
};

class Cat : public Animal
{
public:
  using Ptr = std::shared_ptr<Cat>;

  std::string name() const override
  {
    return "Cat";
  }
};

class Sphynx : public Cat
{
public:
  using Ptr = std::shared_ptr<Sphynx>;

  std::string name() const override
  {
    return "Sphynx";
  }
};

//--------------------------------------------------------------
// Nodes that produce derived types
//--------------------------------------------------------------

class CreateCat : public SyncActionNode
{
public:
  CreateCat(const std::string& name, const NodeConfig& config)
    : SyncActionNode(name, config)
  {}

  NodeStatus tick() override
  {
    setOutput("animal", std::make_shared<Cat>());
    return NodeStatus::SUCCESS;
  }

  static PortsList providedPorts()
  {
    return { OutputPort<Cat::Ptr>("animal") };
  }
};

class CreateSphynx : public SyncActionNode
{
public:
  CreateSphynx(const std::string& name, const NodeConfig& config)
    : SyncActionNode(name, config)
  {}

  NodeStatus tick() override
  {
    setOutput("animal", std::make_shared<Sphynx>());
    return NodeStatus::SUCCESS;
  }

  static PortsList providedPorts()
  {
    return { OutputPort<Sphynx::Ptr>("animal") };
  }
};

//--------------------------------------------------------------
// A node that consumes the base type
//--------------------------------------------------------------

class SayHi : public SyncActionNode
{
public:
  SayHi(const std::string& name, const NodeConfig& config) : SyncActionNode(name, config)
  {}

  NodeStatus tick() override
  {
    auto animal = getInput<Animal::Ptr>("animal").value();
    std::cout << "Hi! I am a " << animal->name() << std::endl;
    return NodeStatus::SUCCESS;
  }

  static PortsList providedPorts()
  {
    return { InputPort<Animal::Ptr>("animal") };
  }
};

//--------------------------------------------------------------

// clang-format off
static const char* xml_text = R"(
 <root BTCPP_format="4" >
   <BehaviorTree ID="MainTree">
     <Sequence>
       <CreateCat    animal="{pet}" />
       <SayHi        animal="{pet}" />
       <CreateSphynx animal="{pet2}" />
       <SayHi        animal="{pet2}" />
     </Sequence>
   </BehaviorTree>
 </root>
)";
// clang-format on

int main()
{
  BehaviorTreeFactory factory;

  // Register the inheritance relationships.
  // This is what makes Cat::Ptr and Sphynx::Ptr assignable to Animal::Ptr ports.
  factory.registerPolymorphicCast<Cat, Animal>();
  factory.registerPolymorphicCast<Sphynx, Cat>();

  factory.registerNodeType<CreateCat>("CreateCat");
  factory.registerNodeType<CreateSphynx>("CreateSphynx");
  factory.registerNodeType<SayHi>("SayHi");

  auto tree = factory.createTreeFromText(xml_text);
  tree.tickWhileRunning();

  /* Expected output:
   *
   *   Hi! I am a Cat
   *   Hi! I am a Sphynx
   */
  return 0;
}
