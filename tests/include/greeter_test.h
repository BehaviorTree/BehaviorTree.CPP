#ifndef GREETER_TEST_H_
#define GREETER_TEST_H_

#include <behaviortree_cpp/utils/safe_any.hpp>

class Animal
{
public:
  using Ptr = std::shared_ptr<Animal>;
  virtual ~Animal() = default;
};

class Cat : public Animal
{
public:
  using Ptr = std::shared_ptr<Cat>;
};

class SphynxCat : public Cat
{
public:
  using Ptr = std::shared_ptr<SphynxCat>;
};

class Dog : public Animal
{
public:
  using Ptr = std::shared_ptr<Dog>;
};

// Not registered
class Plant
{
public:
  using Ptr = std::shared_ptr<Plant>;
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
struct BT::any_cast_base<SphynxCat>
{
  using type = Cat;
};

template <>
struct BT::any_cast_base<Dog>
{
  using type = Animal;
};

/**
 *    +-------------------+------------+-------------+-----------------------------+
 *    |       Class       | Base Class | Polymorphic | Type Trait Registered       |
 *    +-------------------+------------+-------------+-----------------------------+
 *    | Greeter           | -          | Yes         | Greeter                     |
 *    | HelloGreeter      | Greeter    | Yes         | Greeter                     |
 *    | FancyHelloGreeter | Greeter    | Yes         | Greeter                     |
 *    | Unwelcomer        | -          | Yes         | Greeter (Purposely invalid) |
 *    +-------------------+------------+-------------+-----------------------------+
 */

class Greeter
{
public:
  using Ptr = std::shared_ptr<Greeter>;
  virtual ~Greeter() = default;
  virtual std::string show_msg() const
  {
    return "";
  };
};

class HelloGreeter : public Greeter
{
public:
  using Ptr = std::shared_ptr<HelloGreeter>;
  std::string show_msg() const override
  {
    return "hello";
  }
  void setDerivedParameter(int n){};
};

class FancyHelloGreeter : public HelloGreeter
{
public:
  using Ptr = std::shared_ptr<FancyHelloGreeter>;
  std::string show_msg() const override
  {
    return "salutations";
  }
};

class Unwelcomer
{
public:
  using Ptr = std::shared_ptr<Unwelcomer>;
  virtual ~Unwelcomer() = default;
  virtual std::string show_msg() const
  {
    return "You’re not welcome here";
  };
};

// Register cast base type for self to allow direct cast
template <>
struct BT::any_cast_base<Greeter>
{
  using type = Greeter;
};

// Register cast base type for HelloGreeter
template <>
struct BT::any_cast_base<HelloGreeter>
{
  using type = Greeter;
};

// Register cast base type for FancyHelloGreeter
template <>
struct BT::any_cast_base<FancyHelloGreeter>
{
  using type = HelloGreeter;
};

// Register cast base type for Unwelcomer
// WARNING: intentionally incorrect
template <>
struct BT::any_cast_base<Unwelcomer>
{
  using type = Greeter;
};

/**
*    +-------------------+--------------+-------------+-----------------------+
*    |     Class         | Base Class   | Polymorphic | Type Trait Registered |
*    +-------------------+--------------+-------------+-----------------------+
*    | GreeterNoReg      | -            | Yes         | -                     |
*    | HelloGreeterNoReg | GreeterNoReg | Yes         | -                     |
*    +-------------------+--------------+-------------+-----------------------+
*/

class GreeterNoReg
{
public:
  using Ptr = std::shared_ptr<GreeterNoReg>;
  virtual ~GreeterNoReg() = default;
  virtual std::string show_msg() const
  {
    return "";
  };
};

class HelloGreeterNoReg : public GreeterNoReg
{
public:
  using Ptr = std::shared_ptr<HelloGreeterNoReg>;
  std::string show_msg() const override
  {
    return "hello";
  }
  void setDerivedParameter(int n){};
};

/**
*    +--------------------+---------------+-------------+-----------------------+
*    |     Class          | Base Class    | Polymorphic | Type Trait Registered |
*    +--------------------+---------------+-------------+-----------------------+
*    | GreeterNoPoly      | -             | No          | GreeterNoPoly         |
*    | HelloGreeterNoPoly | GreeterNoPoly | No          | GreeterNoPoly         |
*    +-----------------------+------------+-------------+-----------------------+
*/

// Correctly fails to compile:
//     static_assert(std::is_polymorphic_v<Base>, "Base must be polymorphic");

// class GreeterNoPoly
// {
// public:
//   using Ptr = std::shared_ptr<GreeterNoPoly>;
//   std::string greet() const
//   {
//     return "";
//   };
// };
//
// class HelloGreeterNoPoly : public GreeterNoPoly
// {
// public:
//   using Ptr = std::shared_ptr<HelloGreeterNoPoly>;
//   std::string hello_greet()
//   {
//     return "hello" + greet();
//   }
// };
//
// // Register cast base type for self to allow direct cast
// template <>
// struct BT::any_cast_base<GreeterNoPoly>
// {
//   using type = GreeterNoPoly;
// };
//
// // Register cast base type for HelloGreeter
// template <>
// struct BT::any_cast_base<HelloGreeterNoPoly>
// {
//   using type = GreeterNoPoly;
// };

/**
*    +-----------------------+------------+-------------+-----------------------+
*    |     Class             | Base Class | Polymorphic | Type Trait Registered |
*    +-----------------------+------------+-------------+-----------------------+
*    | GreeterNoPolyReg      | -          | No          | -                     |
*    | HelloGreeterNoPolyReg | NoGreeter  | No          | -                     |
*    +-----------------------+------------+-------------+-----------------------+
*/

class GreeterNoPolyReg
{
public:
  using Ptr = std::shared_ptr<GreeterNoPolyReg>;
  std::string greet() const
  {
    return "";
  };
};

class HelloGreeterNoPolyReg : public GreeterNoPolyReg
{
public:
  using Ptr = std::shared_ptr<HelloGreeterNoPolyReg>;
  std::string hello_greet()
  {
    return "hello" + greet();
  }
};

#endif  // GREETER_TEST_H_
