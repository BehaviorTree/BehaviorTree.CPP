#pragma once

#include <memory>
#include <string>

// Animal hierarchy for testing polymorphic port connections
//
//         Animal
//        /      \
//      Cat      Dog
//       |
//    Sphynx

class Animal
{
public:
  using Ptr = std::shared_ptr<Animal>;

  Animal() = default;
  virtual ~Animal() = default;
  Animal(const Animal&) = default;
  Animal& operator=(const Animal&) = default;
  Animal(Animal&&) = default;
  Animal& operator=(Animal&&) = default;

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

  void meow() const
  {}
};

class Dog : public Animal
{
public:
  using Ptr = std::shared_ptr<Dog>;

  std::string name() const override
  {
    return "Dog";
  }

  void bark() const
  {}
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

// Helper to register the animal hierarchy with a factory
// Usage: RegisterAnimalHierarchy(factory);
#include "behaviortree_cpp/bt_factory.h"
inline void RegisterAnimalHierarchy(BT::BehaviorTreeFactory& factory)
{
  factory.registerPolymorphicCast<Cat, Animal>();
  factory.registerPolymorphicCast<Dog, Animal>();
  factory.registerPolymorphicCast<Sphynx, Cat>();
}
