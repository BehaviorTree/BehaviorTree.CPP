// bind_basic_types.cpp — enum bindings: NodeStatus, NodeType, PortDirection.

#include <nanobind/nanobind.h>

#include "behaviortree_cpp/basic_types.h"

namespace nb = nanobind;

namespace pybt {

void register_basic_types(nb::module_& m)
{
  nb::enum_<BT::NodeStatus>(m, "NodeStatus",
                            "Status returned by every tick. IDLE is the initial "
                            "state; user-defined nodes should never return IDLE.")
      .value("IDLE", BT::NodeStatus::IDLE, "Initial state; no tick has run yet.")
      .value("RUNNING", BT::NodeStatus::RUNNING,
             "Tick is still in progress; will be called again.")
      .value("SUCCESS", BT::NodeStatus::SUCCESS, "Tick completed successfully.")
      .value("FAILURE", BT::NodeStatus::FAILURE, "Tick completed unsuccessfully.")
      .value("SKIPPED", BT::NodeStatus::SKIPPED,
             "Tick was skipped (e.g. by a precondition).");

  nb::enum_<BT::NodeType>(m, "NodeType", "The kind of node in a behavior tree.")
      .value("UNDEFINED", BT::NodeType::UNDEFINED)
      .value("ACTION", BT::NodeType::ACTION, "Leaf node that performs work.")
      .value("CONDITION", BT::NodeType::CONDITION,
             "Leaf node that returns SUCCESS or FAILURE based on a check.")
      .value("CONTROL", BT::NodeType::CONTROL,
             "Internal node with multiple children (Sequence, Fallback, etc.).")
      .value("DECORATOR", BT::NodeType::DECORATOR,
             "Internal node with exactly one child that modifies its behavior.")
      .value("SUBTREE", BT::NodeType::SUBTREE,
             "Reference to a nested tree composed elsewhere.");

  nb::enum_<BT::PortDirection>(m, "PortDirection",
                               "Direction of a port: read-only, write-only, or both.")
      .value("INPUT", BT::PortDirection::INPUT, "Read-only port.")
      .value("OUTPUT", BT::PortDirection::OUTPUT, "Write-only port.")
      .value("INOUT", BT::PortDirection::INOUT, "Read-write port.");
}

}  // namespace pybt
