# Port Connection and Validation Rules

This document describes the rules that govern how ports can be connected in BehaviorTree.CPP, including type checking, type conversion, and special cases.

## Overview

BehaviorTree.CPP uses a type system for ports that enforces type safety while providing flexibility through several special rules. Type checking occurs primarily at **tree creation time** (when parsing XML), not at runtime.

## Port Types Classification

### 1. Strongly Typed Ports

A port is **strongly typed** when declared with a specific type:

```cpp
InputPort<int>("my_port")
OutputPort<double>("result")
InputPort<Position2D>("goal")
```

### 2. Generic/Weakly Typed Ports (AnyTypeAllowed)

A port is **generic** (not strongly typed) when:
- Declared without a type parameter
- Declared with `AnyTypeAllowed`
- Declared with `BT::Any`
- Declared with `std::string` (**OoutputPort** only)

```cpp
// All of these create generic ports:
InputPort<>("value")                    // defaults to AnyTypeAllowed
InputPort<AnyTypeAllowed>("value")      // explicit AnyTypeAllowed
InputPort<BT::Any>("value")             // BT::Any type
OoutputPort<std::string>("value")       // Can be connected to strong typed input
```

The `isStronglyTyped()` method returns `false` for these ports:


## Port Connection Rules

### Rule 1: Same Type - Always Compatible

Ports of the **exact same type** can always be connected:

```cpp
// Connection allowed
OutputPort<int>("value")    // writes int
InputPort<int>("value")     // reads int
```

**Test reference:** `gtest_port_type_rules.cpp` - `SameType_IntToInt`, `SameType_StringToString`, `SameType_CustomTypeToCustomType` tests

### Rule 2: Generic Port - Compatible with Any Type

A **generic port** (`AnyTypeAllowed` or `BT::Any`) can connect to any other port:

```cpp
// Connection: OK - generic port accepts any type
OutputPort<>("output")           // generic, can write anything
InputPort<int>("input_int")      // expects int
```

**Test reference:** `gtest_port_type_rules.cpp` - `GenericPort_AcceptsInt`, `GenericPort_AcceptsString`, `GenericOutput_ToTypedInput` tests

### Rule 3: String is a "Universal Donor" (Generic Port)

When a blackboard entry is created as `std::string`, it can be connected to ports of **any type** that has a `convertFromString<T>()` specialization. This is the "string as generic port" rule.

Note that this may cause a run-time error if the string is not convertible.

**Example:**
```xml
<Sequence>
    <!-- Creates blackboard entry "value" as string with value "42" -->
    <SetBlackboard value="42" output_key="value" />
    <!-- Reads "value" as int - OK because string can convert to int -->
    <NodeExpectingInt input="{value}" />
</Sequence>
```

**Also applies to:**
- Subtree port passing (string values passed to typed subtree ports)
- Script node assignments

**Test reference:** `gtest_port_type_rules.cpp` - `StringToInt_ViaConvertFromString`, `StringToCustomType_ViaConvertFromString`, `SubtreeStringInput_ToTypedPort` tests

### Rule 4: String Creation in Blackboard

When using `Blackboard::set<std::string>()`, the entry is created with `AnyTypeAllowed` type, not `std::string`:

This allows subsequent writes of different types to the same entry.

**Test reference:** `gtest_port_type_rules.cpp` - `BlackboardSetString_CreatesGenericEntry`, `StringEntry_CanBecomeTyped` tests

### Rule 5: Type Lock After First Strongly-Typed Write

Once a blackboard entry receives a **strongly typed** value, its type is locked:

After this, writing a different type will fail (with exception).

**Test reference:** `gtest_port_type_rules.cpp` - `TypeLock_CannotChangeAfterTypedWrite`, `TypeLock_XMLTreeCreation_TypeMismatch`, `TypeLock_RuntimeTypeChange_Fails` tests

### Rule 6: BT::Any Bypasses Type Checking

When a blackboard entry is **created with type `BT::Any`**, it can store different types over time.

This requires the entry to be explicitly created as `BT::Any` type.

```cpp
// Create entry explicitly as BT::Any type
bb->createEntry("key", TypeInfo::Create<BT::Any>());

// Now different types are allowed
bb->set("key", BT::Any(42));       // OK
bb->set("key", BT::Any("hello"));  // OK
bb->set("key", BT::Any(3.14));     // OK
```

**Test reference:** `gtest_port_type_rules.cpp` - `BTAny_WrapperDoesNotBypassTypeCheck`, `BTAny_EntryType_AllowsDifferentTypes`, `BTAny_Port_AcceptsDifferentTypes` tests

### Rule 7: Type Mismatch Between Strongly Typed Ports - Error

If two **strongly typed** ports with **different types** try to use the same blackboard entry, an error is thrown at tree creation:

```xml
<!-- This will FAIL at tree creation -->
<Sequence>
    <NodeA output_int="{value}" />      <!-- Creates entry as int -->
    <NodeB input_string="{value}" />    <!-- Tries to read as string - ERROR -->
</Sequence>
```

**Test reference:** `gtest_port_type_rules.cpp` - `TypeLock_XMLTreeCreation_TypeMismatch`, `TypeLock_IntToDouble_Fails`, `TypeLock_CustomTypeChange_Fails` tests

## Type Conversion via convertFromString

### Built-in Conversions

The library provides `convertFromString<T>()` for:
- `int`, `long`, `long long`, and unsigned variants
- `float`, `double`
- `bool` (accepts "true"/"false", "1"/"0")
- `std::string`
- `std::vector<T>` (semicolon-separated values)
- Enums (when registered)

### Custom Type Conversion

To make a custom type compatible with string ports, specialize `convertFromString`:

```cpp
namespace BT
{
template <>
inline Position2D convertFromString(StringView str)
{
  auto parts = splitString(str, ';');
  if(parts.size() != 2)
    throw RuntimeError("invalid input");

  Position2D output;
  output.x = convertFromString<double>(parts[0]);
  output.y = convertFromString<double>(parts[1]);
  return output;
}
}
```

**Test reference:** `gtest_ports.cpp`, `t03_generic_ports.cpp`

### JSON Format Support

Custom types can also use JSON format with "json:" prefix:

```cpp
InputPort<Point2D>("pointE", R"(json:{"x":9,"y":10})", "description")
```

**Test reference:** `gtest_ports.cpp`

## Validation Timeline

### At Tree Creation (XML Parsing)

1. **Port name validation** - Checks port exists in node manifest
2. **Literal value validation** - If port value is not a blackboard reference, validates conversion
3. **Blackboard entry type check** - If entry exists, checks type compatibility

**Source:** `xml_parsing.cpp`
```cpp
if(!is_blackboard && port_model.converter() && port_model.isStronglyTyped())
{
  try
  {
    port_model.converter()(port_value);  // Validate conversion
  }
  catch(std::exception& ex)
  {
    throw LogicError("The port... can not be converted to " + port_model.typeName());
  }
}
```

### At Runtime (Blackboard::set)

1. **Type match check** - Compares new type with entry's declared type
2. **String conversion attempt** - If mismatch, tries `parseString()`

## Summary Table

| Scenario | Compatible? | Notes |
|----------|-------------|-------|
| Same types | Yes | Always works |
| Generic port (either side) | Yes | `AnyTypeAllowed` or `BT::Any` |
| String → Typed port | Yes | Via `convertFromString<T>()` |
| Typed → String port | No | Type mismatch error |
| int → double | No | Different strongly-typed |
| Point2D → std::string | No | Unless entry was string first |
| BT::Any entry to anything | Yes | Entry must be created as BT::Any type |

## Reserved Port Names

The following names **cannot** be used for ports:
- `name` - Reserved for node instance name
- `ID` - Reserved for node type ID
- Names starting with `_` - Reserved for internal use

**Test reference:** `gtest_port_type_rules.cpp` - `ReservedPortName_ThrowsOnRegistration` test
