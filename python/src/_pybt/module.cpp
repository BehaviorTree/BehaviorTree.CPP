#include <nanobind/nanobind.h>

namespace nb = nanobind;

NB_MODULE(_pybt, m)
{
  m.doc() = "pybt — Python bindings for BehaviorTree.CPP.";
  m.attr("__phase__") = "1-foundation";
}
