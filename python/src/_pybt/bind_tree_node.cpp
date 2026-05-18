// bind_tree_node.cpp — TreeNode binding plus user-subclassable shell
// classes and the adapters that bridge Python to BT.CPP at tree-tick time.
//
// Design (adapter pattern, see Phase 1 Subagent B notes):
//
//   * `PySyncActionNode` / `PyStatefulActionNode` are nanobind trampoline
//     "shells". They exist so Python can `class Foo(pybt.SyncActionNode):`
//     and have a real, instantiable Python base. Their tick / on_* methods
//     are placeholders — they should never be invoked at tree-tick time.
//
//   * `PythonSyncActionAdapter` / `PythonStatefulActionAdapter` are the
//     actual C++ tree nodes. They hold a strong `nb::object` reference to
//     the user's Python instance and forward virtual calls into Python.
//     They are allocated via standard `new` (`std::make_unique`) so the
//     `std::unique_ptr<TreeNode>` BT.CPP holds can `delete` them safely.
//
// Why two classes per kind? Combining "Python wrapper trampoline" with
// "tree-owned C++ object" produced an allocator mismatch: nanobind
// allocates trampoline instances as part of the Python wrapper object,
// but `unique_ptr<TreeNode>::~unique_ptr` calls plain `delete` — which
// targets a different allocator and triggered `free(): invalid pointer`
// on tree teardown. Splitting the roles fixes the lifetime model.

#include "json_bridge.hpp"

#include "behaviortree_cpp/action_node.h"
#include "behaviortree_cpp/json_export.h"
#include "behaviortree_cpp/tree_node.h"

#include <nanobind/nanobind.h>
#include <nanobind/stl/string.h>
#include <nanobind/trampoline.h>

namespace nb = nanobind;
using namespace nb::literals;

namespace pybt
{

// --------------------------------------------------------------------------
// Trampoline shells
//
// These exist solely so Python can subclass them with a stable C++ base.
// The factory does NOT use these as tree nodes; the adapters below do.
// --------------------------------------------------------------------------

struct PySyncActionNode : public BT::SyncActionNode
{
  NB_TRAMPOLINE(BT::SyncActionNode, 1);

  PySyncActionNode(const std::string& name, const BT::NodeConfig& config)
    : BT::SyncActionNode(name, config)
  {}

  // Placeholder. The actual tick goes through PythonSyncActionAdapter.
  // If this fires, the factory wiring is broken.
  BT::NodeStatus tick() override
  {
    throw BT::LogicError("PySyncActionNode::tick() invoked directly — should go through "
                         "PythonSyncActionAdapter. This is a binding bug.");
  }
};

struct PyStatefulActionNode : public BT::StatefulActionNode
{
  NB_TRAMPOLINE(BT::StatefulActionNode, 3);

  PyStatefulActionNode(const std::string& name, const BT::NodeConfig& config)
    : BT::StatefulActionNode(name, config)
  {}

  BT::NodeStatus onStart() override
  {
    throw BT::LogicError("PyStatefulActionNode::onStart() invoked directly — "
                         "should go through PythonStatefulActionAdapter.");
  }
  BT::NodeStatus onRunning() override
  {
    throw BT::LogicError("PyStatefulActionNode::onRunning() invoked directly.");
  }
  void onHalted() override
  {
    // No-op default for the shell (never reached at tick time).
  }
};

// --------------------------------------------------------------------------
// Adapters
//
// Each adapter is allocated by us via `new` and owned by Tree's
// `unique_ptr<TreeNode>`. It holds the user's Python instance and forwards
// virtual calls into Python under `gil_scoped_acquire`.
// --------------------------------------------------------------------------

class PythonSyncActionAdapter : public BT::SyncActionNode
{
public:
  PythonSyncActionAdapter(const std::string& name, const BT::NodeConfig& config,
                          nb::object py_inst)
    : BT::SyncActionNode(name, config), py_inst_(std::move(py_inst))
  {}

  ~PythonSyncActionAdapter() override
  {
    nb::gil_scoped_acquire gil;
    py_inst_.reset();
  }

  BT::NodeStatus tick() override
  {
    nb::gil_scoped_acquire gil;
    return nb::cast<BT::NodeStatus>(py_inst_.attr("tick")());
  }

private:
  nb::object py_inst_;
};

class PythonStatefulActionAdapter : public BT::StatefulActionNode
{
public:
  PythonStatefulActionAdapter(const std::string& name, const BT::NodeConfig& config,
                              nb::object py_inst)
    : BT::StatefulActionNode(name, config), py_inst_(std::move(py_inst))
  {}

  ~PythonStatefulActionAdapter() override
  {
    nb::gil_scoped_acquire gil;
    py_inst_.reset();
  }

  BT::NodeStatus onStart() override
  {
    nb::gil_scoped_acquire gil;
    return nb::cast<BT::NodeStatus>(py_inst_.attr("on_start")());
  }

  BT::NodeStatus onRunning() override
  {
    nb::gil_scoped_acquire gil;
    return nb::cast<BT::NodeStatus>(py_inst_.attr("on_running")());
  }

  void onHalted() override
  {
    nb::gil_scoped_acquire gil;
    // `on_halted` is optional — user may skip it when they have no cleanup.
    if(nb::hasattr(py_inst_, "on_halted"))
    {
      py_inst_.attr("on_halted")();
    }
  }

private:
  nb::object py_inst_;
};

// --------------------------------------------------------------------------
// get_input / set_output bridges
// --------------------------------------------------------------------------

static nb::object get_input_impl(BT::TreeNode& self, const std::string& name)
{
  // Case 1: port is remapped to a blackboard entry — read the BT::Any and
  // route through JsonExporter for typed conversion.
  if(auto locked = self.getLockedPortContent(name))
  {
    const BT::Any* any = locked.get();
    if(any && !any->empty())
    {
      nlohmann::json j;
      if(BT::JsonExporter::get().toJson(*any, j))
      {
        return json_to_python(j);
      }
      // No JSON converter for this type — try a string cast as a last resort.
      try
      {
        return nb::cast(const_cast<BT::Any*>(any)->cast<std::string>());
      }
      catch(...)
      {
        throw BT::RuntimeError("get_input('", name,
                               "'): value has no JSON converter registered. "
                               "Register one with JsonExporter::addConverter "
                               "or pass a JSON-native type.");
      }
    }
  }

  // Case 2: port is a raw string literal from XML (e.g. message="hello").
  try
  {
    auto raw = self.getRawPortValue(name);
    return nb::cast(std::string(raw));
  }
  catch(const std::exception& e)
  {
    throw BT::RuntimeError("get_input('", name, "'): ", e.what());
  }
}

static void set_output_impl(BT::TreeNode& self, const std::string& name, nb::object value)
{
  // Honor BT.CPP's port-declaration contract: fail if the output port was
  // never declared. setOutput<Any> performs this check internally and
  // additionally validates that the declared port type is Any or
  // AnyTypeAllowed.
  nlohmann::json j = python_to_json(value);

  // Convert JSON to BT::Any. For primitive JSON values we don't need
  // JsonExporter — we can build BT::Any directly. For complex/registered
  // types, defer to JsonExporter::fromJson which knows about user converters.
  BT::Any any;
  switch(j.type())
  {
    case nlohmann::json::value_t::null:
      any = BT::Any();
      break;
    case nlohmann::json::value_t::boolean:
      any = BT::Any(j.get<bool>());
      break;
    case nlohmann::json::value_t::number_integer:
      any = BT::Any(j.get<int64_t>());
      break;
    case nlohmann::json::value_t::number_unsigned:
      any = BT::Any(j.get<uint64_t>());
      break;
    case nlohmann::json::value_t::number_float:
      any = BT::Any(j.get<double>());
      break;
    case nlohmann::json::value_t::string:
      any = BT::Any(j.get<std::string>());
      break;
    default: {
      auto entry = BT::JsonExporter::get().fromJson(j);
      if(!entry)
      {
        throw BT::RuntimeError("set_output('", name,
                               "'): cannot convert value to a BT type: ", entry.error());
      }
      any = entry->first;
      break;
    }
  }

  auto result = self.setOutput(name, any);
  if(!result)
  {
    throw BT::RuntimeError("set_output('", name, "'): ", result.error());
  }
}

// --------------------------------------------------------------------------
// Binding registration
// --------------------------------------------------------------------------

void register_tree_node(nb::module_& m)
{
  // Opaque NodeConfig binding — never constructed or inspected from Python,
  // but must be visible so the factory's builder can pass it through
  // `py_cls(name, config)` to the user's class's super().__init__.
  nb::class_<BT::NodeConfig>(m, "NodeConfig",
                             "Opaque per-node configuration handed in by the "
                             "factory. Pass through to super().__init__; do "
                             "not construct or inspect.");

  nb::class_<BT::TreeNode>(m, "TreeNode",
                           "Abstract base of every behavior-tree node. Cannot be "
                           "constructed directly; use SyncActionNode, "
                           "StatefulActionNode, or build a tree via the factory.")
      .def_prop_ro("name", &BT::TreeNode::name, "The instance name assigned in the XML.")
      .def_prop_ro("status", &BT::TreeNode::status, "Current NodeStatus.")
      .def_prop_ro("uid", &BT::TreeNode::UID,
                   "Numeric unique identifier assigned by the factory.")
      .def_prop_ro("full_path", &BT::TreeNode::fullPath,
                   "Hierarchical path including all subtrees.")
      .def_prop_ro("registration_name", &BT::TreeNode::registrationName,
                   "The registration ID this node was created from.")
      .def("get_input", &get_input_impl, "name"_a,
           "Read a typed input port by name. Returns the JSON-converted value, "
           "or the raw string for XML literals. Raises BTRuntimeError if the "
           "port is missing or unconvertible.")
      .def("set_output", &set_output_impl, "name"_a, "value"_a,
           "Write a value to a declared output port. Raises BTRuntimeError if "
           "the port was not declared.");

  nb::class_<BT::SyncActionNode, BT::TreeNode, PySyncActionNode>(m, "SyncActionNode",
                                                                 "Synchronous action. "
                                                                 "Subclass and implement "
                                                                 "`tick(self)` returning "
                                                                 "NodeStatus.SUCCESS or "
                                                                 "NodeStatus.FAILURE. "
                                                                 "Returning RUNNING is "
                                                                 "forbidden — use "
                                                                 "StatefulActionNode "
                                                                 "instead.")
      .def(nb::init<const std::string&, const BT::NodeConfig&>(), "name"_a, "config"_a,
           "Built by the factory; users rarely call this directly.");

  nb::class_<BT::StatefulActionNode, BT::TreeNode, PyStatefulActionNode>(m,
                                                                         "StatefulActionN"
                                                                         "ode",
                                                                         "Stateful "
                                                                         "action for "
                                                                         "asynchronous "
                                                                         "work. Subclass "
                                                                         "and implement "
                                                                         "`on_start`, "
                                                                         "`on_running`, "
                                                                         "`on_halted`. "
                                                                         "The factory "
                                                                         "calls on_start "
                                                                         "once, "
                                                                         "then "
                                                                         "on_running "
                                                                         "until the node "
                                                                         "returns "
                                                                         "SUCCESS or "
                                                                         "FAILURE; "
                                                                         "on_halted "
                                                                         "runs if the "
                                                                         "parent halts "
                                                                         "the node while "
                                                                         "RUNNING.")
      .def(nb::init<const std::string&, const BT::NodeConfig&>(), "name"_a, "config"_a,
           "Built by the factory; users rarely call this directly.");
}

// --------------------------------------------------------------------------
// Adapter factories — called from bind_factory.cpp's NodeBuilder lambda.
// --------------------------------------------------------------------------

std::unique_ptr<BT::TreeNode> make_sync_action_adapter(const std::string& name,
                                                       const BT::NodeConfig& config,
                                                       nb::object py_inst)
{
  return std::make_unique<PythonSyncActionAdapter>(name, config, std::move(py_inst));
}

std::unique_ptr<BT::TreeNode> make_stateful_action_adapter(const std::string& name,
                                                           const BT::NodeConfig& config,
                                                           nb::object py_inst)
{
  return std::make_unique<PythonStatefulActionAdapter>(name, config, std::move(py_inst));
}

}  // namespace pybt
