// exceptions.cpp — Python exception hierarchy mirroring BT.CPP exceptions.
//
// Hierarchy:
//   pybt.BTError(Exception)
//     ├── pybt.BTRuntimeError       <- BT::RuntimeError
//     │     └── pybt.BTNodeExecutionError  <- BT::NodeExecutionError
//     └── pybt.BTLogicError         <- BT::LogicError
//
// Standard C++ exceptions (std::out_of_range, std::invalid_argument,
// std::runtime_error, etc.) are translated to Python equivalents
// (IndexError, ValueError, RuntimeError) by nanobind's defaults — we do
// not register translators for those here.

#include <nanobind/nanobind.h>

#include "behaviortree_cpp/exceptions.h"

namespace nb = nanobind;

namespace pybt {

void register_exceptions(nb::module_& m)
{
  // Register most-specific exception last so nanobind's LIFO translator
  // chain catches subclasses before their bases.

  nb::exception<BT::BehaviorTreeException>(m, "BTError");
  nb::handle base = m.attr("BTError");

  nb::exception<BT::RuntimeError>(m, "BTRuntimeError", base);
  nb::exception<BT::LogicError>(m, "BTLogicError", base);

  nb::handle runtime = m.attr("BTRuntimeError");
  nb::exception<BT::NodeExecutionError>(m, "BTNodeExecutionError", runtime);
}

}  // namespace pybt
