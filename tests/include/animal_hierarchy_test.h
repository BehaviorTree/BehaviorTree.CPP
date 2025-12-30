#ifndef ANIMALHIERARCHY_H_
#define ANIMALHIERARCHY_H_

#include <behaviortree_cpp/utils/safe_any.hpp>

class Animal
{
public:
  using Ptr = std::shared_ptr<Animal>;

  virtual std::string name() const
  {
    return name_;
  }

  void setName(const std::string& name)
  {
    name_ = name;
  }

  virtual ~Animal() = default;

private:
  std::string name_ = "Unknown";
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

class Dog : public Animal
{
public:
  using Ptr = std::shared_ptr<Dog>;
  std::string name() const override
  {
    return "Dog";
  }
};

template <>
struct BT::any_cast_base<Animal>
{
  using type = Animal;
};

template <>
struct BT::any_cast_base<Cat>
{
  using type = Animal;
};

template <>
struct BT::any_cast_base<Sphynx>
{
  using type = Cat;
};

template <>
struct BT::any_cast_base<Dog>
{
  using type = Animal;
};

#endif  // ANIMALHIERARCHY_H_
