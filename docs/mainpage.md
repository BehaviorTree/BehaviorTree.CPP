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

<table>
<tr><th>Category</th><th>Nodes</th></tr>
<tr><td>Sequences</td><td><ul>
  <li>@ref BT::SequenceNode</li>
  <li>@ref BT::ReactiveSequence</li>
  <li>@ref BT::SequenceWithMemory</li>
</ul></td></tr>
<tr><td>Fallbacks</td><td><ul>
  <li>@ref BT::FallbackNode</li>
  <li>@ref BT::ReactiveFallback</li>
</ul></td></tr>
<tr><td>Parallels</td><td><ul>
  <li>@ref BT::ParallelNode</li>
  <li>@ref BT::ParallelAllNode</li>
</ul></td></tr>
<tr><td>Conditional</td><td><ul>
  <li>@ref BT::IfThenElseNode</li>
  <li>@ref BT::WhileDoElseNode</li>
  <li>@ref BT::SwitchNode</li>
  <li>@ref BT::ManualSelectorNode</li>
</ul></td></tr>
</table>

## Built-in Decorators

<table>
<tr><th>Category</th><th>Nodes</th></tr>
<tr><td>Repetition</td><td><ul>
  <li>@ref BT::RetryNode</li>
  <li>@ref BT::RepeatNode</li>
  <li>@ref BT::LoopNode</li>
</ul></td></tr>
<tr><td>Timing</td><td><ul>
  <li>@ref BT::TimeoutNode</li>
  <li>@ref BT::DelayNode</li>
</ul></td></tr>
<tr><td>Result Modification</td><td><ul>
  <li>@ref BT::InverterNode</li>
  <li>@ref BT::ForceSuccessNode</li>
  <li>@ref BT::ForceFailureNode</li>
</ul></td></tr>
<tr><td>Execution Control</td><td><ul>
  <li>@ref BT::RunOnceNode</li>
  <li>@ref BT::KeepRunningUntilFailureNode</li>
</ul></td></tr>
<tr><td>Subtrees</td><td><ul>
  <li>@ref BT::SubTreeNode</li>
</ul></td></tr>
<tr><td>Preconditions</td><td><ul>
  <li>@ref BT::PreconditionNode</li>
  <li>@ref BT::EntryUpdatedDecorator</li>
</ul></td></tr>
</table>

## Built-in Actions

<table>
<tr><th>Category</th><th>Nodes</th></tr>
<tr><td>Status</td><td><ul>
  <li>@ref BT::AlwaysSuccessNode</li>
  <li>@ref BT::AlwaysFailureNode</li>
</ul></td></tr>
<tr><td>Blackboard</td><td><ul>
  <li>@ref BT::SetBlackboardNode</li>
  <li>@ref BT::UnsetBlackboardNode</li>
</ul></td></tr>
<tr><td>Utility</td><td><ul>
  <li>@ref BT::SleepNode</li>
  <li>@ref BT::TestNode</li>
</ul></td></tr>
<tr><td>Scripting</td><td><ul>
  <li>@ref BT::ScriptNode</li>
  <li>@ref BT::ScriptCondition</li>
</ul></td></tr>
<tr><td>Entry Updated</td><td><ul>
  <li>@ref BT::EntryUpdatedAction</li>
</ul></td></tr>
<tr><td>Queue</td><td><ul>
  <li>@ref BT::PopFromQueue</li>
  <li>@ref BT::QueueSize</li>
</ul></td></tr>
</table>


## Logging & Tools
- @ref BT::FileLogger2 - File logging
- @ref BT::StdCoutLogger - Console output
- @ref BT::Groot2Publisher - Groot2 editor integration

## Resources
- [GitHub Repository](https://github.com/BehaviorTree/BehaviorTree.CPP)
- [Groot2 Editor](https://www.behaviortree.dev/)
