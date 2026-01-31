# FIXME - Bug Tracker

This file tracks reported bugs from GitHub issues. For each bug, the workflow is:
1. Write a unit test that reproduces the issue
2. Evaluate whether the behavior diverges from intended library semantics
3. Fix the issue (if confirmed as a bug)

---

## Scripting / Script Parser

### [x] #1029 - Mathematical order of operations in script is wrong
- **URL:** https://github.com/BehaviorTree/BehaviorTree.CPP/issues/1029
- **Summary:** `A - B + C` is evaluated as `A - (B + C)` instead of left-to-right. The expression parser applies incorrect associativity for addition and subtraction at the same precedence level.
- **Component:** `src/scripting/`
- **Test file:** `tests/gtest_scripting.cpp` (or new)

### [x] #923 - Out-of-bounds read in script validation
- **URL:** https://github.com/BehaviorTree/BehaviorTree.CPP/issues/923
- **Summary:** `ValidateScript` can create a `std::string` from a non-NULL-terminated fixed-size stack buffer when parsing invalid scripts that generate large error messages, causing `strlen` to read out of bounds.
- **Component:** `src/scripting/`
- **Test file:** `tests/gtest_scripting.cpp` (or new)

### [x] #832 - Script compare with negative number throws exception
- **URL:** https://github.com/BehaviorTree/BehaviorTree.CPP/issues/832
- **Summary:** Using a negative number in a script precondition (e.g., `_failureIf="A!=-1"`) throws a runtime exception. The scripting parser cannot handle negative number literals in comparisons.
- **Component:** `src/scripting/`
- **Test file:** `tests/gtest_scripting.cpp` (or new)

---

## Ports / Type Conversion

### [x] #1065 - Type conversion problem with subtree
- **URL:** https://github.com/BehaviorTree/BehaviorTree.CPP/issues/1065
- **Summary:** Passing a `std::deque<double>` through a SubTree port via a string literal (e.g., `"1;2;3"`) throws `std::runtime_error` about no safe conversion between `std::string` and `std::shared_ptr<std::deque<double>>`. String-to-deque conversion fails at subtree boundary.
- **Component:** `src/xml_parsing.cpp`, `include/behaviortree_cpp/basic_types.h`
- **Test file:** `tests/gtest_ports.cpp` (or new)

### [x] #982 - Vectors initialized with `json:[]` string instead of empty
- **URL:** https://github.com/BehaviorTree/BehaviorTree.CPP/issues/982
- **Summary:** When a port of type `std::vector<std::string>` has a default empty value `{}` and no input is specified in XML, the vector is initialized with the literal string `"json:[]"` instead of being an empty vector.
- **Component:** `include/behaviortree_cpp/basic_types.h`, `src/xml_parsing.cpp`
- **Test file:** `tests/gtest_ports.cpp`

### [x] #969 - LoopNode: std::vector<T> vs std::deque<T> type mismatch
- **URL:** https://github.com/BehaviorTree/BehaviorTree.CPP/issues/969
- **Summary:** `LoopNode<T>` uses `std::shared_ptr<std::deque<T>>` for its queue port, but upstream nodes often produce `std::vector<T>`. This creates a port type conflict during tree creation.
- **Component:** `include/behaviortree_cpp/decorators/loop_node.h`
- **Test file:** `tests/gtest_decorator.cpp` (or new)

### [x] #948 - ParseString errors with custom enum types
- **URL:** https://github.com/BehaviorTree/BehaviorTree.CPP/issues/948
- **Summary:** `parseString` in `tree_node.h` has an unsafe `find` on an unordered_map that may not be instantiated yet if `getInput` is called during construction. Also, custom enum types cannot be properly parsed via `convertFromString`.
- **Component:** `include/behaviortree_cpp/tree_node.h`, `include/behaviortree_cpp/basic_types.h`
- **Test file:** `tests/gtest_ports.cpp`

### [ ] #858 - getInput throws when default parameter is passed
- **URL:** https://github.com/BehaviorTree/BehaviorTree.CPP/issues/858
- **Summary:** Calling `getInput` with a default parameter value throws `std::out_of_range` (`_Map_base::at`). The internal map lookup fails when the port has a default value but the entry does not exist in the expected location.
- **Component:** `include/behaviortree_cpp/tree_node.h`
- **Test file:** `tests/gtest_ports.cpp`

---

## Blackboard

### [ ] #974 - Blackboard set() stores values as string type
- **URL:** https://github.com/BehaviorTree/BehaviorTree.CPP/issues/974
- **Summary:** When using `blackboard->set(key, value.dump())` to set numeric values from JSON, the blackboard stores them as strings. Subsequent scripting operations with numeric operators fail because string types cannot be compared numerically.
- **Component:** `src/blackboard.cpp`, `include/behaviortree_cpp/blackboard.h`
- **Test file:** `tests/gtest_blackboard.cpp`

### [ ] #942 - Default blackboard entry not created for locked content
- **URL:** https://github.com/BehaviorTree/BehaviorTree.CPP/issues/942
- **Summary:** When using `getLockedPortContent` on a port with a default blackboard entry, no blackboard entry is actually created on the first call. The returned `AnyPtrLocked` is a default/empty value not placed into the blackboard.
- **Component:** `src/blackboard.cpp`, `include/behaviortree_cpp/blackboard.h`
- **Test file:** `tests/gtest_blackboard.cpp`

### [ ] #408 - debugMessage does not print parent values in SubTree
- **URL:** https://github.com/BehaviorTree/BehaviorTree.CPP/issues/408
- **Summary:** `config().blackboard->debugMessage()` inside a SubTree node's constructor does not show remapped entries from the parent blackboard. The function only checks `internal_to_external_` after finding an entry in `storage_`.
- **Component:** `src/blackboard.cpp`
- **Test file:** `tests/gtest_blackboard.cpp`

---

## Control Nodes (Parallel)

### [ ] #1045 - Test PauseWithRetry is flaky
- **URL:** https://github.com/BehaviorTree/BehaviorTree.CPP/issues/1045
- **Summary:** The `Parallel.PauseWithRetry` test fails intermittently due to tight timing assertions. Measured time drifts beyond the allowed margin.
- **Component:** `tests/gtest_parallel.cpp`
- **Test file:** `tests/gtest_parallel.cpp`

### [ ] #1041 - Performance degradation after prolonged operation in parallel nodes
- **URL:** https://github.com/BehaviorTree/BehaviorTree.CPP/issues/1041
- **Summary:** After prolonged operation, when a parallel node's child detects an exception and exits, the interval before other nodes wait for execution becomes increasingly longer. Likely related to resource accumulation or thread management.
- **Component:** `src/controls/parallel_node.cpp`
- **Test file:** `tests/gtest_parallel.cpp` (or new)

### [ ] #966 - ParallelAll max_failures off-by-one semantics
- **URL:** https://github.com/BehaviorTree/BehaviorTree.CPP/issues/966
- **Summary:** `ParallelAll` documents `max_failures` as "if the number of children returning FAILURE *exceeds* this value" but implements `>=` (greater-than-or-equal). Documentation and implementation disagree.
- **Component:** `src/controls/parallel_all_node.cpp`
- **Test file:** `tests/gtest_parallel.cpp`

---

## Control Nodes (Reactive)

### [ ] #1031 - Race condition in ReactiveSequence/ReactiveFallback with async children
- **URL:** https://github.com/BehaviorTree/BehaviorTree.CPP/issues/1031
- **Summary:** With backchaining patterns, a previously `RUNNING` child is halted only *after* another child's `onStart` is executed. Two children are in `RUNNING` state simultaneously briefly, causing race conditions.
- **Component:** `src/controls/reactive_sequence.cpp`, `src/controls/reactive_fallback.cpp`
- **Test file:** `tests/gtest_reactive.cpp` (or new)

### [ ] #917 - Precondition not re-checked in ReactiveSequence
- **URL:** https://github.com/BehaviorTree/BehaviorTree.CPP/issues/917
- **Summary:** When a node has a `_successIf` precondition inside a `ReactiveSequence`, the precondition is never re-evaluated after the node enters `RUNNING` state. The node continues returning `RUNNING` instead of `SUCCESS`.
- **Component:** `src/controls/reactive_sequence.cpp`, `src/tree_node.cpp`
- **Test file:** `tests/gtest_reactive.cpp`

---

## XML Parsing

### [ ] #979 - Recursive behavior trees cause stack overflow
- **URL:** https://github.com/BehaviorTree/BehaviorTree.CPP/issues/979
- **Summary:** A behavior tree can reference its own ID as a SubTree, causing infinite recursion and stack overflow during XML parsing. The parser does not detect or prevent cyclic subtree references.
- **Component:** `src/xml_parsing.cpp`
- **Test file:** `tests/gtest_factory.cpp` (or new)

### [ ] #883 - JSON string in port value interpreted as blackboard reference
- **URL:** https://github.com/BehaviorTree/BehaviorTree.CPP/issues/883
- **Summary:** Setting a port value to a JSON string in XML (e.g., `settings='{{"a": 0.8}}'`) causes `{}` content to be interpreted as blackboard variable references instead of literal JSON.
- **Component:** `src/xml_parsing.cpp`, `src/tree_node.cpp`
- **Test file:** `tests/gtest_factory.cpp` (or new)

### [ ] #880 - External subtree from file not found when loaded via inline XML
- **URL:** https://github.com/BehaviorTree/BehaviorTree.CPP/issues/880
- **Summary:** When a subtree is registered via `registerBehaviorTreeFromFile()` and then referenced from inline XML via `createTreeFromText()`, the factory throws `"Can't find a tree with name: MyTree"`.
- **Component:** `src/xml_parsing.cpp`, `src/bt_factory.cpp`
- **Test file:** `tests/gtest_factory.cpp`

### [ ] #672 - Stack buffer overflow in xml_parsing.cpp
- **URL:** https://github.com/BehaviorTree/BehaviorTree.CPP/issues/672
- **Summary:** AddressSanitizer detects a stack-buffer-overflow in `xml_parsing.cpp` when parsing certain XML tree definitions. Memory safety issue in the XML parser.
- **Component:** `src/xml_parsing.cpp`
- **Test file:** `tests/gtest_factory.cpp` (or new)

---

## BehaviorTreeFactory

### [ ] #1046 - Heap use-after-free when factory is destroyed before ticking
- **URL:** https://github.com/BehaviorTree/BehaviorTree.CPP/issues/1046
- **Summary:** If `BehaviorTreeFactory` is destroyed before its created tree is ticked, `NodeConfig` holds a dangling pointer to `TreeNodeManifest` owned by the factory. Accessing it causes heap-use-after-free.
- **Component:** `src/bt_factory.cpp`, `include/behaviortree_cpp/tree_node.h`
- **Test file:** `tests/gtest_factory.cpp`

### [ ] #937 - BehaviorTreeFactory cannot be returned by value
- **URL:** https://github.com/BehaviorTree/BehaviorTree.CPP/issues/937
- **Summary:** Making `BehaviorTreeFactory` non-copyable also inadvertently prevents returning it by value from functions. Some compilers do not apply guaranteed copy elision in all contexts.
- **Component:** `include/behaviortree_cpp/bt_factory.h`
- **Test file:** `tests/gtest_factory.cpp` (or new)

### [ ] #934 - Segfault when substituting subtree
- **URL:** https://github.com/BehaviorTree/BehaviorTree.CPP/issues/934
- **Summary:** `factory.loadSubstitutionRuleFromJSON()` followed by `factory.createTree()` causes a segmentation fault when the substitution target is a subtree node.
- **Component:** `src/bt_factory.cpp`
- **Test file:** `tests/gtest_factory.cpp`

### [ ] #930 - Mock substitution with registerSimpleAction causes tree to hang
- **URL:** https://github.com/BehaviorTree/BehaviorTree.CPP/issues/930
- **Summary:** When using mock node substitution rules with `registerSimpleAction()`, the behavior tree gets stuck after an async delay finishes. The substituted node does not behave like the original.
- **Component:** `src/bt_factory.cpp`
- **Test file:** `tests/gtest_factory.cpp` (or new)

---

## Loggers

### [ ] #1057 - Groot2Publisher enters infinite loop on exception
- **URL:** https://github.com/BehaviorTree/BehaviorTree.CPP/issues/1057
- **Summary:** `Groot2Publisher` can enter an infinite loop during destruction. When `active_server` is set to `false`, it can be reset to `true` depending on timing, causing `server_thread.join()` to hang indefinitely.
- **Component:** `src/loggers/groot2_publisher.cpp`
- **Test file:** New test needed

---

## Tree Execution

### [ ] #686 - haltTree doesn't effectively halt tickWhileRunning
- **URL:** https://github.com/BehaviorTree/BehaviorTree.CPP/issues/686
- **Summary:** `haltTree()` resets tree status to `IDLE`, which only breaks the inner loop of `tickRoot`. The outer loop then restarts execution because `IDLE` satisfies its condition, so the tree is never truly halted with `tickWhileRunning()`.
- **Component:** `src/bt_factory.cpp`, `include/behaviortree_cpp/bt_factory.h`
- **Test file:** `tests/gtest_tree.cpp` (or new)

---

## TreeNode

### [ ] #861 - Tick time monitoring can measure 0 due to instruction reordering
- **URL:** https://github.com/BehaviorTree/BehaviorTree.CPP/issues/861
- **Summary:** The `std::chrono` calls around `executeTick()` in `tree_node.cpp` can be reordered by the compiler, resulting in a measured tick duration of 0.
- **Component:** `src/tree_node.cpp`
- **Test file:** `tests/gtest_tree.cpp` (or new)

---

## JSON Exporter

### [ ] #989 - JsonExporter throws during vector conversion
- **URL:** https://github.com/BehaviorTree/BehaviorTree.CPP/issues/989
- **Summary:** A `bad_function_call` exception is thrown when converting `std::vector` types to JSON via `JsonExporter`. The conversion function is not properly registered for vector types.
- **Component:** `include/behaviortree_cpp/json_export.h`
- **Test file:** New test needed

---

## Build / Dependencies

### [ ] #959 - Version conflict with embedded nlohmann::json causes UB
- **URL:** https://github.com/BehaviorTree/BehaviorTree.CPP/issues/959
- **Summary:** The library embeds `nlohmann::json` v3.11.3 in its headers. If the user's codebase uses a different version, ODR violations occur depending on header include order, potentially causing undefined behavior.
- **Component:** `3rdparty/`, `CMakeLists.txt`
- **Test file:** Build system test / CMake configuration

---

## Platform-Specific

### [ ] #869 - Read access violation on Windows debug build
- **URL:** https://github.com/BehaviorTree/BehaviorTree.CPP/issues/869
- **Summary:** Running on VS 2022 in Debug mode causes a read access violation in string hashing within the library DLL. Release mode works fine, suggesting a debug-mode-specific memory issue.
- **Component:** Core library (platform-specific)
- **Test file:** Requires Windows environment

---

## Summary

| Category | Count | Issue Numbers |
|----------|-------|---------------|
| Scripting / Script Parser | 3 | #1029, #923, #832 |
| Ports / Type Conversion | 5 | #1065, #982, #969, #948, #858 |
| Blackboard | 3 | #974, #942, #408 |
| Control Nodes (Parallel) | 3 | #1045, #1041, #966 |
| Control Nodes (Reactive) | 2 | #1031, #917 |
| XML Parsing | 4 | #979, #883, #880, #672 |
| BehaviorTreeFactory | 4 | #1046, #937, #934, #930 |
| Loggers | 1 | #1057 |
| Tree Execution | 1 | #686 |
| TreeNode | 1 | #861 |
| JSON Exporter | 1 | #989 |
| Build / Dependencies | 1 | #959 |
| Platform-Specific | 1 | #869 |
| **Total** | **30** | |
