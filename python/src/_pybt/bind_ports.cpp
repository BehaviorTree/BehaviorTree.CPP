// bind_ports.cpp — PortInfo binding plus input_port/output_port helpers.
//
// Ports declared from Python use AnyTypeAllowed; type checking happens at
// the JSON serialization layer in bind_tree_node.cpp. Custom strongly-typed
// ports are an advanced use case and not needed for Phase 1.

#include <nanobind/nanobind.h>
#include <nanobind/stl/pair.h>
#include <nanobind/stl/string.h>

#include "behaviortree_cpp/basic_types.h"

namespace nb = nanobind;
using namespace nb::literals;

namespace pybt {

void register_ports(nb::module_& m)
{
  nb::class_<BT::PortInfo>(m, "PortInfo",
                           "Describes one port: its direction, description, and "
                           "(for advanced use) its registered type and default value.")
      .def_prop_ro("direction", &BT::PortInfo::direction,
                   "Whether this port is INPUT, OUTPUT, or INOUT.")
      .def_prop_ro("description", &BT::PortInfo::description,
                   "Human-readable description, or empty string.")
      .def_prop_ro("default_value_string", &BT::PortInfo::defaultValueString,
                   "Default value as a string, or empty if no default was set.")
      .def("__repr__", [](const BT::PortInfo& self) {
        std::string dir;
        switch(self.direction())
        {
          case BT::PortDirection::INPUT:
            dir = "INPUT";
            break;
          case BT::PortDirection::OUTPUT:
            dir = "OUTPUT";
            break;
          case BT::PortDirection::INOUT:
            dir = "INOUT";
            break;
        }
        return "<pybt.PortInfo direction=" + dir + ">";
      });

  m.def(
      "input_port",
      [](const std::string& name, const std::string& description) {
        return BT::CreatePort<BT::AnyTypeAllowed>(BT::PortDirection::INPUT, name,
                                                  description);
      },
      "name"_a, "description"_a = "",
      "Build an input port. Returns a (name, PortInfo) pair suitable for the "
      "`ports` argument of `BehaviorTreeFactory.register_node_type`.");

  m.def(
      "output_port",
      [](const std::string& name, const std::string& description) {
        return BT::CreatePort<BT::AnyTypeAllowed>(BT::PortDirection::OUTPUT, name,
                                                  description);
      },
      "name"_a, "description"_a = "",
      "Build an output port. Returns a (name, PortInfo) pair suitable for the "
      "`ports` argument of `BehaviorTreeFactory.register_node_type`.");

  m.def(
      "bidirectional_port",
      [](const std::string& name, const std::string& description) {
        return BT::CreatePort<BT::AnyTypeAllowed>(BT::PortDirection::INOUT, name,
                                                  description);
      },
      "name"_a, "description"_a = "",
      "Build a read/write port. Returns a (name, PortInfo) pair.");
}

}  // namespace pybt
