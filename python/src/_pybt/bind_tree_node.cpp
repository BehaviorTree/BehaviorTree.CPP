// bind_tree_node.cpp — TreeNode binding plus trampolines for the two
// user-subclassable kinds of action node.
//
// Trampoline pattern: each trampoline class inherits the BT.CPP type,
// declares NB_TRAMPOLINE for the methods Python may override, and forwards
// the virtual calls back to Python under nb::gil_scoped_acquire (the tick
// loop drops the GIL by default; trampolines reacquire to call into Python).
//
// Lifetime: Python users construct trampoline subclasses; the factory's
// NodeBuilder lambda calls the Python class with (name, config) to produce
// a fresh instance per tree. The Python instance is parked in
// PythonInstanceRegistry so it outlives the unique_ptr handed to the
// C++ tree; the trampoline destructor evicts itself from the registry.

#include <mutex>
#include <unordered_map>

#include <nanobind/nanobind.h>
#include <nanobind/stl/string.h>
#include <nanobind/trampoline.h>

#include "behaviortree_cpp/action_node.h"
#include "behaviortree_cpp/json_export.h"
#include "behaviortree_cpp/tree_node.h"

#include "json_bridge.hpp"

namespace nb = nanobind;
using namespace nb::literals;

namespace pybt {

// --------------------------------------------------------------------------
// Python instance registry
//
// Keeps the Python wrapper alive as long as the C++ trampoline lives.
// Without this, the user's Python instance can be garbage-collected the
// moment the factory builder returns — then the trampoline's NB_OVERRIDE_PURE
// would fail to find a Python tick() method.
// --------------------------------------------------------------------------

class PythonInstanceRegistry
{
public:
  static PythonInstanceRegistry& get()
  {
    static PythonInstanceRegistry r;
    return r;
  }

  void store(BT::TreeNode* node, nb::object instance)
  {
    std::lock_guard<std::mutex> lock(mu_);
    map_[node] = std::move(instance);
  }

  void remove(BT::TreeNode* node)
  {
    // Drop the nb::object with the GIL held (its destructor decrefs).
    nb::gil_scoped_acquire gil;
    std::lock_guard<std::mutex> lock(mu_);
    map_.erase(node);
  }

private:
  std::mutex mu_;
  std::unordered_map<BT::TreeNode*, nb::object> map_;
};

// --------------------------------------------------------------------------
// Trampolines
// --------------------------------------------------------------------------

struct PySyncActionNode : public BT::SyncActionNode
{
  NB_TRAMPOLINE(BT::SyncActionNode, 1);

  PySyncActionNode(const std::string& name, const BT::NodeConfig& config)
    : BT::SyncActionNode(name, config)
  {}

  ~PySyncActionNode() override
  {
    PythonInstanceRegistry::get().remove(this);
  }

  BT::NodeStatus tick() override
  {
    nb::gil_scoped_acquire gil;
    NB_OVERRIDE_PURE(tick);
  }
};

struct PyStatefulActionNode : public BT::StatefulActionNode
{
  NB_TRAMPOLINE(BT::StatefulActionNode, 3);

  PyStatefulActionNode(const std::string& name, const BT::NodeConfig& config)
    : BT::StatefulActionNode(name, config)
  {}

  ~PyStatefulActionNode() override
  {
    PythonInstanceRegistry::get().remove(this);
  }

  BT::NodeStatus onStart() override
  {
    nb::gil_scoped_acquire gil;
    NB_OVERRIDE_PURE_NAME("on_start", onStart);
  }

  BT::NodeStatus onRunning() override
  {
    nb::gil_scoped_acquire gil;
    NB_OVERRIDE_PURE_NAME("on_running", onRunning);
  }

  void onHalted() override
  {
    nb::gil_scoped_acquire gil;
    NB_OVERRIDE_PURE_NAME("on_halted", onHalted);
  }
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
                               "'): cannot convert value to a BT type: ",
                               entry.error());
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
  nb::class_<BT::TreeNode>(m, "TreeNode",
                           "Abstract base of every behavior-tree node. Cannot be "
                           "constructed directly; use SyncActionNode, "
                           "StatefulActionNode, or build a tree via the factory.")
      .def_prop_ro("name", &BT::TreeNode::name,
                   "The instance name assigned in the XML.")
      .def_prop_ro("status", &BT::TreeNode::status,
                   "Current NodeStatus.")
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

  nb::class_<BT::SyncActionNode, BT::TreeNode, PySyncActionNode>(
      m, "SyncActionNode",
      "Synchronous action. Subclass and implement `tick(self)` returning "
      "NodeStatus.SUCCESS or NodeStatus.FAILURE. Returning RUNNING is "
      "forbidden — use StatefulActionNode instead.")
      .def(nb::init<const std::string&, const BT::NodeConfig&>(), "name"_a,
           "config"_a, "Built by the factory; users rarely call this directly.");

  nb::class_<BT::StatefulActionNode, BT::TreeNode, PyStatefulActionNode>(
      m, "StatefulActionNode",
      "Stateful action for asynchronous work. Subclass and implement "
      "`on_start`, `on_running`, `on_halted`. The factory calls on_start once, "
      "then on_running until the node returns SUCCESS or FAILURE; on_halted "
      "runs if the parent halts the node while RUNNING.")
      .def(nb::init<const std::string&, const BT::NodeConfig&>(), "name"_a,
           "config"_a, "Built by the factory; users rarely call this directly.");
}

// Exposed for bind_factory.cpp: parks `instance` in the registry keyed by `node`.
void park_python_instance(BT::TreeNode* node, nb::object instance)
{
  PythonInstanceRegistry::get().store(node, std::move(instance));
}

}  // namespace pybt
