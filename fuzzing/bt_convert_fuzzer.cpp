// Fuzz target: bt_convert_fuzzer
//
// Threat model: every XML attribute, script literal and blackboard value that
// enters a BehaviorTree.CPP application passes through BT::convertFromString<T>
// (port parsing), SafeAny/Any type-erased casts, or typed blackboard access.
// These string->type and type->type conversions are the numeric- and
// string-parsing funnel for untrusted data (attacker-supplied XML documents,
// Groot files, scripting expressions). polymorphic_cast_registry powers
// shared_ptr port compatibility (Issue #943) and is dark in production
// coverage (220 lines, 0.9%).
//
// Strategy (component-API harness, typed inputs):
//   1. convertFromString<T> over the full builtin type matrix with a fuzzed
//      string (numerics incl. int8..64/uint8..64/float/double overflow and
//      underflow, bool, enums NodeStatus/NodeType/PortDirection, split
//      containers vector<T>, SharedQueue<T>),
//   2. Any value storage + tryCast/cast cross-type matrix,
//   3. PolymorphicCastRegistry upcasts/downcasts/transitive chains over a
//      harness-defined class hierarchy (Animal/Cat/Dog/Sphynx), including via
//      a blackboard with the registry attached,
//   4. blackboard typed set/get mismatch matrix.
//
// All thrown BT exceptions (RuntimeError/LogicError on invalid input) are
// expected and caught; the point is exercising the conversion code paths, not
// crashing on them.
//
// Expected bug classes: integer/float overflow and truncation handling,
// out-of-bounds reads in split-string parsing, enum parsing mismatches,
// type-confusion in Any casts, registry lookup errors on shared_ptr ports.

#include "behaviortree_cpp/basic_types.h"
#include "behaviortree_cpp/blackboard.h"
#include "behaviortree_cpp/decorators/loop_node.h"
#include "behaviortree_cpp/utils/polymorphic_cast_registry.hpp"
#include "behaviortree_cpp/utils/safe_any.hpp"

#include <fuzzer/FuzzedDataProvider.h>

#include <cstdint>
#include <memory>
#include <string>

namespace
{

// Harness-defined hierarchy for the polymorphic cast registry. No upstream
// product code is involved: the registry API is public and standalone.
struct Animal
{
  virtual ~Animal() = default;
  virtual const char* species() const
  {
    return "Animal";
  }
};
struct Cat : Animal
{
  const char* species() const override
  {
    return "Cat";
  }
};
struct Dog : Animal
{
  const char* species() const override
  {
    return "Dog";
  }
};
struct Sphynx : Cat
{
  const char* species() const override
  {
    return "Sphynx";
  }
};

using AnimalPtr = std::shared_ptr<Animal>;
using CatPtr = std::shared_ptr<Cat>;
using DogPtr = std::shared_ptr<Dog>;
using SphynxPtr = std::shared_ptr<Sphynx>;

}  // namespace

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
  if(size < 8)
  {
    return 0;
  }
  FuzzedDataProvider fdp(data, size);

  const std::string text = fdp.ConsumeRandomLengthString();
  const BT::StringView sv(text);

  // ---- Stage 1: convertFromString<T> over the builtin type matrix
  try
  {
    switch(fdp.ConsumeIntegralInRange<uint8_t>(0, 19))
    {
      case 0:
        (void)BT::convertFromString<std::string>(sv);
        break;
      case 1:
        (void)BT::convertFromString<int8_t>(sv);
        break;
      case 2:
        (void)BT::convertFromString<int16_t>(sv);
        break;
      case 3:
        (void)BT::convertFromString<int32_t>(sv);
        break;
      case 4:
        (void)BT::convertFromString<int64_t>(sv);
        break;
      case 5:
        (void)BT::convertFromString<uint8_t>(sv);
        break;
      case 6:
        (void)BT::convertFromString<uint16_t>(sv);
        break;
      case 7:
        (void)BT::convertFromString<uint32_t>(sv);
        break;
      case 8:
        (void)BT::convertFromString<uint64_t>(sv);
        break;
      case 9:
        (void)BT::convertFromString<float>(sv);
        break;
      case 10:
        (void)BT::convertFromString<double>(sv);
        break;
      case 11:
        (void)BT::convertFromString<bool>(sv);
        break;
      case 12:
        (void)BT::convertFromString<BT::NodeStatus>(sv);
        break;
      case 13:
        (void)BT::convertFromString<BT::NodeType>(sv);
        break;
      case 14:
        (void)BT::convertFromString<BT::PortDirection>(sv);
        break;
      case 15:
        (void)BT::convertFromString<std::vector<int>>(sv);
        break;
      case 16:
        (void)BT::convertFromString<std::vector<double>>(sv);
        break;
      case 17:
        (void)BT::convertFromString<std::vector<bool>>(sv);
        break;
      case 18:
        (void)BT::convertFromString<std::vector<std::string>>(sv);
        break;
      default:
        (void)BT::convertFromString<BT::SharedQueue<int>>(sv);
        break;
    }
  }
  catch(const std::exception&)
  {
    // invalid numeric/enum input raises RuntimeError: expected
  }

  // Stages 2-4 are wrapped so that BT exceptions raised by the conversion
  // APIs are contained per-iteration and never escape LLVMFuzzerTestOneInput.
  try
  {
  // ---- Stage 2: Any storage + cross-type cast matrix
  {
    BT::Any any;
    switch(fdp.ConsumeIntegralInRange<uint8_t>(0, 2))
    {
      case 0:
        any = BT::Any(text);
        break;
      case 1:
        any = BT::Any(static_cast<int64_t>(text.size()) * 1000000);
        break;
      default:
        any = BT::Any(static_cast<double>(text.size()) * 1.5);
        break;
    }

    (void)any.isNumber();
    (void)any.isIntegral();
    (void)any.isString();

    switch(fdp.ConsumeIntegralInRange<uint8_t>(0, 5))
    {
      case 0:
      {
        auto res = any.tryCast<int64_t>();
        (void)res.has_value();
        break;
      }
      case 1:
      {
        auto res = any.tryCast<uint64_t>();
        (void)res.has_value();
        break;
      }
      case 2:
      {
        auto res = any.tryCast<double>();
        (void)res.has_value();
        break;
      }
      case 3:
      {
        auto res = any.tryCast<std::string>();
        (void)res.has_value();
        break;
      }
      case 4:
      {
        auto res = any.tryCast<int>();
        (void)res.has_value();
        break;
      }
      default:
      {
        // cast<T>() throws RuntimeError on a lossy/invalid cast
        try
        {
          (void)any.cast<uint64_t>();
        }
        catch(const std::exception&)
        {
          // expected on invalid casts
        }
        break;
      }
    }
  }

  // ---- Stage 3: polymorphic cast registry
  {
    BT::PolymorphicCastRegistry registry;
    registry.registerCast<Cat, Animal>();
    registry.registerCast<Dog, Animal>();
    registry.registerCast<Sphynx, Cat>();

    BT::Any animal_any;
    switch(fdp.ConsumeIntegralInRange<uint8_t>(0, 3))
    {
      case 0:
        animal_any = BT::Any(AnimalPtr(std::make_shared<Animal>()));
        break;
      case 1:
        animal_any = BT::Any(CatPtr(std::make_shared<Cat>()));
        break;
      case 2:
        animal_any = BT::Any(DogPtr(std::make_shared<Dog>()));
        break;
      default:
        animal_any = BT::Any(SphynxPtr(std::make_shared<Sphynx>()));
        break;
    }

    switch(fdp.ConsumeIntegralInRange<uint8_t>(0, 4))
    {
      case 0:
      {
        auto res = animal_any.tryCastWithRegistry<AnimalPtr>(registry);
        (void)res.has_value();
        break;
      }
      case 1:
      {
        auto res = animal_any.tryCastWithRegistry<CatPtr>(registry);
        (void)res.has_value();
        break;
      }
      case 2:
      {
        auto res = animal_any.tryCastWithRegistry<DogPtr>(registry);
        (void)res.has_value();
        break;
      }
      case 3:
      {
        auto res = animal_any.tryCastWithRegistry<SphynxPtr>(registry);
        (void)res.has_value();
        break;
      }
      default:
      {
        try
        {
          auto p = animal_any.cast<CatPtr>();
          (void)p;
        }
        catch(const std::exception&)
        {
          // expected when the stored type is not Cat-compatible
        }
        break;
      }
    }

    // Blackboard with an attached registry: typed set/get through the
    // polymorphic path (Issue #943 usage pattern).
    auto reg2 = std::make_shared<BT::PolymorphicCastRegistry>();
    reg2->registerCast<Cat, Animal>();
    reg2->registerCast<Sphynx, Cat>();
    auto bb_poly = BT::Blackboard::create();
    bb_poly->setPolymorphicCastRegistry(reg2);
    bb_poly->set("obj", CatPtr(std::make_shared<Sphynx>()));
    try
    {
      auto up = bb_poly->get<AnimalPtr>("obj");
      (void)up;
    }
    catch(const std::exception&)
    {
      // expected when the registry cannot satisfy the cast
    }
    try
    {
      auto down = bb_poly->get<SphynxPtr>("obj");
      (void)down;
    }
    catch(const std::exception&)
    {
      // expected on unsupported downcasts
    }
  }

  // ---- Stage 4: blackboard typed set/get mismatch matrix
  {
    auto bb = BT::Blackboard::create();
    switch(fdp.ConsumeIntegralInRange<uint8_t>(0, 3))
    {
      case 0:
        bb->set("k", static_cast<int64_t>(text.size()));
        break;
      case 1:
        bb->set("k", static_cast<double>(text.size()));
        break;
      case 2:
        bb->set("k", text);
        break;
      default:
        bb->set("k", !text.empty());
        break;
    }
    const auto fetch = [&bb](auto type_tag) {
      using T = decltype(type_tag);
      try
      {
        // typed get: mismatched entry type raises RuntimeError (expected)
        (void)bb->get<T>("k");
      }
      catch(const std::exception&)
      {
        // expected: type mismatch or missing key
      }
    };
    switch(fdp.ConsumeIntegralInRange<uint8_t>(0, 4))
    {
      case 0:
        fetch(int64_t{});
        break;
      case 1:
        fetch(double{});
        break;
      case 2:
        fetch(std::string{});
        break;
      case 3:
        fetch(bool{});
        break;
      default:
        fetch(int64_t{});  // on a missing key: RuntimeError path
        break;
    }
  }
  }
  catch(const std::exception&)
  {
    // expected: Any/registry/blackboard APIs raise RuntimeError on
    // lossy or mismatched conversions
  }

  return 0;
}

