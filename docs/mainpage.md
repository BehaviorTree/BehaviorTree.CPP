# BehaviorTree.CPP {#mainpage}

C++ library for behavior tree execution.

## Quick Start
- @ref BT::BehaviorTreeFactory - Create and register nodes
- @ref BT::Blackboard - Shared data storage
- @ref BT::TreeNode - Base class for all nodes

## Core Concepts
- @ref BT::PortInfo - Type-safe port system
- @ref BT::Expected - Result type for error handling
- @ref BT::NodeStatus - Node execution states

## Built-in Control Nodes

### Sequences
- @ref BT::SequenceNode
- @ref BT::ReactiveSequence
- @ref BT::SequenceWithMemory

### Fallbacks
- @ref BT::FallbackNode
- @ref BT::ReactiveFallback

### Parallels
- @ref BT::ParallelNode
- @ref BT::ParallelAllNode

### Conditional
- @ref BT::IfThenElseNode
- @ref BT::WhileDoElseNode
- @ref BT::SwitchNode
- @ref BT::ManualSelectorNode

## Built-in Decorators

### Repetition
- @ref BT::RetryNode
- @ref BT::RepeatNode
- @ref BT::LoopNode

### Timing
- @ref BT::TimeoutNode
- @ref BT::DelayNode

### Result Modification
- @ref BT::InverterNode
- @ref BT::ForceSuccessNode
- @ref BT::ForceFailureNode

### Execution Control
- @ref BT::RunOnceNode
- @ref BT::KeepRunningUntilFailureNode

### Subtrees
- @ref BT::SubTreeNode

### Preconditions
- @ref BT::PreconditionNode
- @ref BT::EntryUpdatedDecorator

## Built-in Actions

### Status
- @ref BT::AlwaysSuccessNode
- @ref BT::AlwaysFailureNode

### Blackboard
- @ref BT::SetBlackboardNode
- @ref BT::UnsetBlackboardNode

### Utility
- @ref BT::SleepNode
- @ref BT::TestNode

### Scripting
- @ref BT::ScriptNode
- @ref BT::ScriptCondition

### Entry Updated
- @ref BT::EntryUpdatedAction

### Queue
- @ref BT::PopFromQueue
- @ref BT::QueueSize


## Logging & Tools
- @ref BT::FileLogger2 - File logging
- @ref BT::StdCoutLogger - Console output
- @ref BT::Groot2Publisher - Groot2 editor integration

## Resources
- [GitHub Repository](https://github.com/BehaviorTree/BehaviorTree.CPP)
- [Groot2 Editor](https://www.behaviortree.dev/)
