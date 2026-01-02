# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build Commands

```bash
# Plain CMake (recommended for development)
mkdir build && cmake -S . -B build && cmake --build build --parallel

# With Conan (CMake 3.23+ required)
conan install . -s build_type=Release --build=missing
cmake --preset conan-release
cmake --build --preset conan-release

# Pixi/Conda
pixi run build
```

Requires CMake 3.16.3+ and C++17 compiler. The project exports `CMAKE_EXPORT_COMPILE_COMMANDS=ON` by default.

## Testing

```bash
# Run all tests via CTest
ctest --test-dir build

# Run test executable directly
./build/tests/behaviortree_cpp_test

# Run specific test
./build/tests/behaviortree_cpp_test --gtest_filter="TestName*"

# Pixi
pixi run test
```

Test files are in `tests/` using Google Test. Key test categories: `gtest_blackboard.cpp`, `gtest_factory.cpp`, `gtest_tree.cpp`, `gtest_sequence.cpp`, `gtest_fallback.cpp`, `gtest_parallel.cpp`, `gtest_decorator.cpp`, `gtest_reactive.cpp`, `gtest_ports.cpp`, `gtest_port_type_rules.cpp`.

## Linting and Formatting

```bash
# Pre-commit hooks (clang-format, codespell)
pre-commit install
pre-commit run -a

# Clang-tidy (requires clangd-21, build must exist for compile_commands.json)
./run_clang_tidy.sh [build_path]
```

Install clang-tidy-21:
```bash
wget https://apt.llvm.org/llvm.sh && chmod +x llvm.sh
sudo ./llvm.sh 21
sudo apt install clangd-21 clang-tidy-21
```

Code style: Google C++ with 90-char line limit, 2-space indent. See `.clang-format` and `.clang-tidy` for details.

## Architecture

**Namespace:** `BT::`
**Library:** `behaviortree_cpp`

### Node Hierarchy

All behavior tree nodes inherit from `TreeNode` (`include/behaviortree_cpp/tree_node.h`):

- **LeafNode**: `ActionNode`, `ConditionNode` - user-defined tasks and checks
- **ControlNode**: `SequenceNode`, `FallbackNode`, `ParallelNode`, `ReactiveSequence`, `ReactiveFallback`, `SwitchNode`, `IfThenElseNode`, `WhileDoElseNode`
- **DecoratorNode**: `InverterNode`, `RetryNode`, `RepeatNode`, `TimeoutNode`, `DelayNode`, `SubtreeNode`

### Node Status

`TreeNodeStatus`: `IDLE`, `RUNNING`, `SUCCESS`, `FAILURE`, `SKIPPED`

### Key Components

| Component | Location | Purpose |
|-----------|----------|---------|
| `BehaviorTreeFactory` | `bt_factory.h` | Node registration, XML parsing, tree creation |
| `Blackboard` | `blackboard.h` | Shared typed key-value storage between nodes |
| Port System | `basic_types.h` | Type-safe dataflow: `InputPort<T>`, `OutputPort<T>`, `BidirectionalPort<T>` |
| XML Parser | `xml_parsing.cpp` | Loads trees from XML with type validation |
| Script Parser | `scripting/` | Embedded expression language for conditions |

### Port System Rules

Ports enable type-safe data passing between nodes via the Blackboard:
- Same-typed ports always connect
- Generic ports (`AnyTypeAllowed`, `BT::Any`) accept any type
- `std::string` output acts as "universal donor" (converts via `convertFromString<T>`)
- Type locks after first strongly-typed write
- Reserved names: `name`, `ID`, names starting with `_`

See `docs/PORT_CONNECTION_RULES.md` for detailed rules.

### Source Layout

```
src/
├── *.cpp              # Core: tree_node, blackboard, xml_parsing, bt_factory
├── actions/           # Built-in action nodes
├── controls/          # Control flow nodes (sequence, fallback, parallel, etc.)
├── decorators/        # Decorator nodes (retry, repeat, timeout, etc.)
└── loggers/           # Logging infrastructure (Groot2, SQLite, file)

include/behaviortree_cpp/
├── *.h                # Public API headers
├── controls/          # Control node headers
├── decorators/        # Decorator node headers
├── loggers/           # Logger headers
├── scripting/         # Script parser (lexy-based)
└── contrib/           # Third-party contributions
```

### Integration Points

- **Groot2**: Visual editor integration via ZeroMQ (`BTCPP_GROOT_INTERFACE` option)
- **ROS2**: Auto-detected via `ament_cmake`, uses colcon build
- **Conan**: Package manager support for non-ROS builds

### Vendored Dependencies

All in `3rdparty/`: TinyXML2, cppzmq, flatbuffers, lexy, minicoro, minitrace. Controlled via `USE_VENDORED_*` CMake options.

## Contributing

- Run `pre-commit run -a` and `./run_clang_tidy.sh` before PRs
- Bug fixes should include a failing test that passes after the fix
- Consider API/ABI compatibility implications
