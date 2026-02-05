#include "behaviortree_cpp/bt_factory.h"

#include <gtest/gtest.h>

#include "include/animal_hierarchy_test.h"

using namespace BT;

//-------------------------------------------------------------------
// Any-level polymorphic cast tests (registry)
//-------------------------------------------------------------------

TEST(PolymorphicPortTest, AnyCast_SameType)
{
  PolymorphicCastRegistry registry;
  registry.registerCast<Cat, Animal>();
  registry.registerCast<Dog, Animal>();
  registry.registerCast<Sphynx, Cat>();

  auto animal = std::make_shared<Animal>();
  Any any_animal(animal);
  EXPECT_NO_THROW(auto res = any_animal.cast<Animal::Ptr>());
  // Downcast should fail (returns error, doesn't throw)
  EXPECT_FALSE(any_animal.tryCastWithRegistry<Cat::Ptr>(registry).has_value());
  EXPECT_FALSE(any_animal.tryCastWithRegistry<Sphynx::Ptr>(registry).has_value());
}

TEST(PolymorphicPortTest, AnyCast_Upcast)
{
  PolymorphicCastRegistry registry;
  registry.registerCast<Cat, Animal>();
  registry.registerCast<Dog, Animal>();
  registry.registerCast<Sphynx, Cat>();

  auto cat = std::make_shared<Cat>();
  Any any_cat(cat);
  // Same type works
  EXPECT_NO_THROW(auto res = any_cat.cast<Cat::Ptr>());
  // Upcast via registry
  auto result = any_cat.tryCastWithRegistry<Animal::Ptr>(registry);
  EXPECT_TRUE(result.has_value());
  EXPECT_EQ(result.value()->name(), "Cat");
  // Downcast should fail
  EXPECT_FALSE(any_cat.tryCastWithRegistry<Sphynx::Ptr>(registry).has_value());
}

TEST(PolymorphicPortTest, AnyCast_TransitiveUpcast)
{
  PolymorphicCastRegistry registry;
  registry.registerCast<Cat, Animal>();
  registry.registerCast<Dog, Animal>();
  registry.registerCast<Sphynx, Cat>();

  auto sphynx = std::make_shared<Sphynx>();
  Any any_sphynx(sphynx);
  // Same type works
  EXPECT_NO_THROW(auto res = any_sphynx.cast<Sphynx::Ptr>());
  // Upcast to Cat
  auto as_cat = any_sphynx.tryCastWithRegistry<Cat::Ptr>(registry);
  EXPECT_TRUE(as_cat.has_value());
  EXPECT_EQ(as_cat.value()->name(), "Sphynx");
  // Transitive upcast to Animal
  auto as_animal = any_sphynx.tryCastWithRegistry<Animal::Ptr>(registry);
  EXPECT_TRUE(as_animal.has_value());
  EXPECT_EQ(as_animal.value()->name(), "Sphynx");
}

TEST(PolymorphicPortTest, AnyCast_DowncastWithRuntimeTypeCheck)
{
  PolymorphicCastRegistry registry;
  registry.registerCast<Cat, Animal>();
  registry.registerCast<Sphynx, Cat>();

  Cat::Ptr cat = std::make_shared<Sphynx>();  // Store Sphynx as Cat
  Any any_cat(cat);
  // Same type works
  EXPECT_NO_THROW(auto res = any_cat.cast<Cat::Ptr>());
  // Downcast should work because runtime type is Sphynx
  auto as_sphynx = any_cat.tryCastWithRegistry<Sphynx::Ptr>(registry);
  EXPECT_TRUE(as_sphynx.has_value());
  EXPECT_EQ(as_sphynx.value()->name(), "Sphynx");
}

TEST(PolymorphicPortTest, AnyCast_UnrelatedTypes)
{
  PolymorphicCastRegistry registry;
  registry.registerCast<Cat, Animal>();
  registry.registerCast<Dog, Animal>();

  auto cat = std::make_shared<Cat>();
  Any any_cat(cat);
  EXPECT_FALSE(any_cat.tryCastWithRegistry<Dog::Ptr>(registry).has_value());

  auto dog = std::make_shared<Dog>();
  Any any_dog(dog);
  EXPECT_FALSE(any_dog.tryCastWithRegistry<Cat::Ptr>(registry).has_value());
}

//-------------------------------------------------------------------
// Test nodes for XML-level polymorphic port testing
//-------------------------------------------------------------------

class CreateAnimal : public SyncActionNode
{
public:
  CreateAnimal(const std::string& name, const NodeConfig& config)
    : SyncActionNode(name, config)
  {}

  NodeStatus tick() override
  {
    setOutput("out_animal", std::make_shared<Animal>());
    return NodeStatus::SUCCESS;
  }

  static PortsList providedPorts()
  {
    return { OutputPort<Animal::Ptr>("out_animal") };
  }
};

class CreateCat : public SyncActionNode
{
public:
  CreateCat(const std::string& name, const NodeConfig& config)
    : SyncActionNode(name, config)
  {}

  NodeStatus tick() override
  {
    setOutput("out_cat", std::make_shared<Cat>());
    return NodeStatus::SUCCESS;
  }

  static PortsList providedPorts()
  {
    return { OutputPort<Cat::Ptr>("out_cat") };
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
    setOutput("out_sphynx", std::make_shared<Sphynx>());
    return NodeStatus::SUCCESS;
  }

  static PortsList providedPorts()
  {
    return { OutputPort<Sphynx::Ptr>("out_sphynx") };
  }
};

class CreateDog : public SyncActionNode
{
public:
  CreateDog(const std::string& name, const NodeConfig& config)
    : SyncActionNode(name, config)
  {}

  NodeStatus tick() override
  {
    setOutput("out_dog", std::make_shared<Dog>());
    return NodeStatus::SUCCESS;
  }

  static PortsList providedPorts()
  {
    return { OutputPort<Dog::Ptr>("out_dog") };
  }
};

class CreateCatAsAnimal : public SyncActionNode
{
public:
  CreateCatAsAnimal(const std::string& name, const NodeConfig& config)
    : SyncActionNode(name, config)
  {}

  NodeStatus tick() override
  {
    setOutput("out_animal", Animal::Ptr(std::make_shared<Cat>()));
    return NodeStatus::SUCCESS;
  }

  static PortsList providedPorts()
  {
    return { OutputPort<Animal::Ptr>("out_animal") };
  }
};

class PrintAnimalName : public SyncActionNode
{
public:
  PrintAnimalName(const std::string& name, const NodeConfig& config)
    : SyncActionNode(name, config)
  {}

  NodeStatus tick() override
  {
    Animal::Ptr animal;
    if(!getInput("in_animal", animal) || !animal)
    {
      return NodeStatus::FAILURE;
    }
    last_name_ = animal->name();
    return NodeStatus::SUCCESS;
  }

  static PortsList providedPorts()
  {
    return { InputPort<Animal::Ptr>("in_animal") };
  }

  static std::string last_name_;
};
std::string PrintAnimalName::last_name_;

class PrintCatName : public SyncActionNode
{
public:
  PrintCatName(const std::string& name, const NodeConfig& config)
    : SyncActionNode(name, config)
  {}

  NodeStatus tick() override
  {
    Cat::Ptr cat;
    if(!getInput("in_cat", cat) || !cat)
    {
      return NodeStatus::FAILURE;
    }
    last_name_ = cat->name();
    return NodeStatus::SUCCESS;
  }

  static PortsList providedPorts()
  {
    return { InputPort<Cat::Ptr>("in_cat") };
  }

  static std::string last_name_;
};
std::string PrintCatName::last_name_;

class PrintDogName : public SyncActionNode
{
public:
  PrintDogName(const std::string& name, const NodeConfig& config)
    : SyncActionNode(name, config)
  {}

  NodeStatus tick() override
  {
    Dog::Ptr dog;
    if(!getInput("in_dog", dog) || !dog)
    {
      return NodeStatus::FAILURE;
    }
    last_name_ = dog->name();
    return NodeStatus::SUCCESS;
  }

  static PortsList providedPorts()
  {
    return { InputPort<Dog::Ptr>("in_dog") };
  }

  static std::string last_name_;
};
std::string PrintDogName::last_name_;

//-------------------------------------------------------------------
// Blackboard-level polymorphic get/set tests
//-------------------------------------------------------------------

namespace
{
Blackboard::Ptr createBlackboardWithRegistry()
{
  auto bb = Blackboard::create();
  auto registry = std::make_shared<PolymorphicCastRegistry>();
  registry->registerCast<Cat, Animal>();
  registry->registerCast<Dog, Animal>();
  registry->registerCast<Sphynx, Cat>();
  bb->setPolymorphicCastRegistry(registry);
  return bb;
}
}  // namespace

TEST(PolymorphicPortTest, Blackboard_UpcastAndDowncast)
{
  auto bb = createBlackboardWithRegistry();

  // Store a Cat, retrieve as Animal (upcast)
  auto cat = std::make_shared<Cat>();
  bb->set("pet", cat);

  Animal::Ptr animal;
  ASSERT_TRUE(bb->get("pet", animal));
  ASSERT_EQ(animal->name(), "Cat");

  // Can still get as Cat
  Cat::Ptr retrieved_cat;
  ASSERT_TRUE(bb->get("pet", retrieved_cat));
  ASSERT_EQ(retrieved_cat->name(), "Cat");

  // Cannot get as Sphynx (invalid downcast)
  Sphynx::Ptr sphynx;
  ASSERT_ANY_THROW((void)bb->get("pet", sphynx));
}

TEST(PolymorphicPortTest, Blackboard_TransitiveUpcast)
{
  auto bb = createBlackboardWithRegistry();

  auto sphynx = std::make_shared<Sphynx>();
  bb->set("pet", sphynx);

  // Can get as Animal (transitive upcast through Cat)
  Animal::Ptr animal;
  ASSERT_TRUE(bb->get("pet", animal));
  ASSERT_EQ(animal->name(), "Sphynx");

  // Can get as Cat (direct upcast)
  Cat::Ptr cat;
  ASSERT_TRUE(bb->get("pet", cat));
  ASSERT_EQ(cat->name(), "Sphynx");

  // Can get as Sphynx (same type)
  Sphynx::Ptr retrieved_sphynx;
  ASSERT_TRUE(bb->get("pet", retrieved_sphynx));
  ASSERT_EQ(retrieved_sphynx->name(), "Sphynx");
}

//-------------------------------------------------------------------
// XML tree-level polymorphic port tests
//-------------------------------------------------------------------

TEST(PolymorphicPortTest, XML_ValidUpcast)
{
  std::string xml_txt = R"(
  <root BTCPP_format="4" >
    <BehaviorTree ID="Main">
      <Sequence>
        <CreateCat out_cat="{pet}" />
        <PrintCatName in_cat="{pet}" />
        <PrintAnimalName in_animal="{pet}" />
      </Sequence>
    </BehaviorTree>
  </root>)";

  BehaviorTreeFactory factory;
  RegisterAnimalHierarchy(factory);
  factory.registerNodeType<CreateCat>("CreateCat");
  factory.registerNodeType<PrintCatName>("PrintCatName");
  factory.registerNodeType<PrintAnimalName>("PrintAnimalName");

  auto tree = factory.createTreeFromText(xml_txt);
  NodeStatus status = tree.tickWhileRunning();

  ASSERT_EQ(status, NodeStatus::SUCCESS);
  ASSERT_EQ(PrintCatName::last_name_, "Cat");
  ASSERT_EQ(PrintAnimalName::last_name_, "Cat");
}

TEST(PolymorphicPortTest, XML_TransitiveUpcast)
{
  std::string xml_txt = R"(
  <root BTCPP_format="4" >
    <BehaviorTree ID="Main">
      <Sequence>
        <CreateSphynx out_sphynx="{pet}" />
        <PrintAnimalName in_animal="{pet}" />
      </Sequence>
    </BehaviorTree>
  </root>)";

  BehaviorTreeFactory factory;
  RegisterAnimalHierarchy(factory);
  factory.registerNodeType<CreateSphynx>("CreateSphynx");
  factory.registerNodeType<PrintAnimalName>("PrintAnimalName");

  auto tree = factory.createTreeFromText(xml_txt);
  NodeStatus status = tree.tickWhileRunning();

  ASSERT_EQ(status, NodeStatus::SUCCESS);
  ASSERT_EQ(PrintAnimalName::last_name_, "Sphynx");
}

TEST(PolymorphicPortTest, XML_InoutRejectsTypeMismatch)
{
  class UpdateAnimal : public SyncActionNode
  {
  public:
    UpdateAnimal(const std::string& name, const NodeConfig& config)
      : SyncActionNode(name, config)
    {}
    NodeStatus tick() override
    {
      return NodeStatus::SUCCESS;
    }
    static PortsList providedPorts()
    {
      return { BidirectionalPort<Animal::Ptr>("animal") };
    }
  };

  std::string xml_txt = R"(
  <root BTCPP_format="4" >
    <BehaviorTree ID="Main">
      <Sequence>
        <CreateCat out_cat="{pet}" />
        <UpdateAnimal animal="{pet}" />
      </Sequence>
    </BehaviorTree>
  </root>)";

  BehaviorTreeFactory factory;
  RegisterAnimalHierarchy(factory);
  factory.registerNodeType<CreateCat>("CreateCat");
  factory.registerNodeType<UpdateAnimal>("UpdateAnimal");

  ASSERT_ANY_THROW((void)factory.createTreeFromText(xml_txt));
}

TEST(PolymorphicPortTest, XML_InvalidConnection_UnrelatedTypes)
{
  std::string xml_txt = R"(
  <root BTCPP_format="4" >
    <BehaviorTree ID="Main">
      <Sequence>
        <CreateCat out_cat="{pet}" />
        <PrintDogName in_dog="{pet}" />
      </Sequence>
    </BehaviorTree>
  </root>)";

  BehaviorTreeFactory factory;
  factory.registerNodeType<CreateCat>("CreateCat");
  factory.registerNodeType<PrintDogName>("PrintDogName");

  ASSERT_ANY_THROW((void)factory.createTreeFromText(xml_txt));
}

TEST(PolymorphicPortTest, XML_DowncastSucceedsAtRuntime)
{
  std::string xml_txt = R"(
  <root BTCPP_format="4" >
    <BehaviorTree ID="Main">
      <Sequence>
        <CreateCatAsAnimal out_animal="{pet}" />
        <PrintCatName in_cat="{pet}" />
      </Sequence>
    </BehaviorTree>
  </root>)";

  BehaviorTreeFactory factory;
  RegisterAnimalHierarchy(factory);
  factory.registerNodeType<CreateCatAsAnimal>("CreateCatAsAnimal");
  factory.registerNodeType<PrintCatName>("PrintCatName");

  auto tree = factory.createTreeFromText(xml_txt);
  NodeStatus status = tree.tickWhileRunning();

  ASSERT_EQ(status, NodeStatus::SUCCESS);
  ASSERT_EQ(PrintCatName::last_name_, "Cat");
}

TEST(PolymorphicPortTest, XML_DowncastFailsAtRuntime)
{
  std::string xml_txt = R"(
  <root BTCPP_format="4" >
    <BehaviorTree ID="Main">
      <Sequence>
        <CreateAnimal out_animal="{pet}" />
        <PrintCatName in_cat="{pet}" />
      </Sequence>
    </BehaviorTree>
  </root>)";

  BehaviorTreeFactory factory;
  RegisterAnimalHierarchy(factory);
  factory.registerNodeType<CreateAnimal>("CreateAnimal");
  factory.registerNodeType<PrintCatName>("PrintCatName");

  auto tree = factory.createTreeFromText(xml_txt);
  // Runtime should fail (actual type is Animal, not Cat)
  ASSERT_EQ(tree.tickWhileRunning(), NodeStatus::FAILURE);
}
