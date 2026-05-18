// module.cpp — pybt entry point. Dispatches to each bind_* registration
// function. Order matters: types and exceptions first, then port/node
// machinery, then high-level factory and tree.

#include <nanobind/nanobind.h>

namespace nb = nanobind;

namespace pybt {
void register_exceptions(nb::module_& m);
void register_basic_types(nb::module_& m);
void register_ports(nb::module_& m);
void register_tree_node(nb::module_& m);
void register_factory(nb::module_& m);
void register_tree(nb::module_& m);
}  // namespace pybt

NB_MODULE(_pybt, m)
{
  m.doc() = "pybt — Python bindings for BehaviorTree.CPP.";
  m.attr("__phase__") = "1-foundation";

  pybt::register_exceptions(m);
  pybt::register_basic_types(m);
  pybt::register_ports(m);
  pybt::register_tree_node(m);
  pybt::register_tree(m);
  pybt::register_factory(m);
}
