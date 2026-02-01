# Name Validation Rules

This document describes the validation rules for names in Groot2 and BehaviorTree.CPP. These rules ensure XML compatibility while supporting Unicode characters (Chinese, Japanese, Korean, etc.).

## Overview

The validation uses a **blacklist approach**: all characters are allowed except those explicitly forbidden. This enables Unicode support while blocking characters that would break XML serialization or cause path/filesystem issues.

## Forbidden Characters (Model Names & Port Names)

The following ASCII characters are forbidden in **Model Names** and **Port Names**:

| Category | Characters | Reason |
|----------|------------|--------|
| Whitespace | `space`, `\t`, `\n`, `\r` | Breaks XML element/attribute names |
| XML special | `<`, `>`, `&`, `"`, `'` | Reserved in XML |
| Path separators | `/`, `\`, `:` | Filesystem conflicts |
| Wildcards | `*`, `?`, `\|` | Shell/glob conflicts |
| Period | `.` | Ambiguous in port names (e.g., `request.name`) |
| Control chars | ASCII 0-31, 127 | Non-printable |

## Allowed Characters

| Category | Examples |
|----------|----------|
| ASCII letters | `a-z`, `A-Z` |
| Digits | `0-9` |
| Underscore | `_` |
| Hyphen | `-` |
| Unicode letters | `中文`, `日本語`, `한국어`, `Ümlauts` |

## Validation Rules by Name Type

### Model Name (Node Type Name)
- **Cannot be empty**
- **Cannot be "Root"** (reserved)
- No forbidden characters (see table above)

### Port Name
- **Cannot be empty**
- **Cannot start with a digit**
- **Cannot be a reserved attribute**: `ID`, `name`, `_description`, `_skipIf`, `_successIf`, `_failureIf`, `_while`, `_onSuccess`, `_onFailure`, `_onHalted`, `_post`, `_autoremap`, `__shared_blackboard`
- No forbidden characters (see table above)

### Instance Name
- **Can be empty** (defaults to model name)
- Instance names are XML attribute **values** (not element/attribute names), so most characters are allowed including spaces, periods, etc.
- Only invalid XML control characters are forbidden (ASCII 0-8, 11-12, 14-31, 127)

## Implementation

### C++ Reference Implementation

```cpp
#include <algorithm>
#include <array>
#include <string>

// Returns the forbidden character if found, or '\0' if valid
static char findForbiddenChar(const std::string& name)
{
  static constexpr std::array<char, 16> forbidden = {
      ' ', '\t', '\n', '\r', '<', '>', '&', '"', '\'', '/', '\\', ':', '*', '?', '|', '.'};

  for (unsigned char c : name)
  {
    // Allow UTF-8 multibyte sequences (high bit set)
    if (c >= 0x80)
    {
      continue;
    }
    // Block control characters
    if (c < 32 || c == 127)
    {
      return static_cast<char>(c);
    }
    // Check forbidden list
    if (std::find(forbidden.begin(), forbidden.end(), c) != forbidden.end())
    {
      return static_cast<char>(c);
    }
  }
  return '\0';
}
```

## Examples

### Valid Model/Port Names
```
MyAction
my_action
My-Action
检查门状态      (Chinese)
ドアを開ける    (Japanese)
Tür_öffnen      (German)
```

### Invalid Model/Port Names
```
My Action       (contains space)
request.name    (contains period)
My<Node>        (contains XML chars)
path/to/node    (contains path separator)
Root            (reserved)
```

### Valid Instance Names
Instance names have relaxed rules since they are XML attribute values:
```
My Action           (spaces allowed)
node.name           (periods allowed)
Success 1           (spaces allowed)
检查门状态          (Unicode allowed)
```

### Invalid Instance Names
```
name_with_null\0    (null character)
name_with_bell\x07  (control character)
```

## Related Issues

- [#59](https://github.com/BehaviorTree/Groot2/issues/59) - Unicode support in node names
- [#60](https://github.com/BehaviorTree/Groot2/issues/60) - i18n support request
- [#64](https://github.com/BehaviorTree/Groot2/issues/64) - Clear error for forbidden characters in port names

## Files Modified in BehaviorTree.CPP

- `include/behaviortree_cpp/basic_types.h` - `findForbiddenChar()` declaration
- `src/basic_types.cpp` - `findForbiddenChar()` implementation, `IsAllowedPortName()` update
- `src/xml_parsing.cpp` - Validation functions and integration in XML parsing
- `tests/gtest_name_validation.cpp` - Comprehensive tests for validation
