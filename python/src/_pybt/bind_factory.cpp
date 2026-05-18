// bind_factory.cpp — BehaviorTreeFactory binding.

#include <filesystem>
#include <functional>
#include <memory>
#include <string>
#include <utility>

#include <nanobind/nanobind.h>
#include <nanobind/stl/shared_ptr.h>
#include <nanobind/stl/string.h>

#include "behaviortree_cpp/basic_types.h"
#include "behaviortree_cpp/bt_factory.h"
#include "behaviortree_cpp/tree_node.h"

namespace nb = nanobind;
using namespace nb::literals;

namespace pybt {

// Defined in bind_tree_node.cpp — keeps the Python wrapper alive as long
// as the corresponding C++ TreeNode lives.
void park_python_instance(BT::TreeNode* node, nb::object instance);

// Defined in bind_tree.cpp — atexit halt walks this registry.
void register_live_tree(std::shared_ptr<BT::Tree> tree);

// Wrap a freshly-created Tree in shared_ptr and register it for atexit halt.
static std::shared_ptr<BT::Tree> wrap_and_track(BT::Tree&& tree)
{
  auto sp = std::make_shared<BT::Tree>(std::move(tree));
  register_live_tree(sp);
  return sp;
}

namespace {

// Build a PortsList from one of:
//   1. Explicit list of (name, PortInfo) tuples (e.g. [pybt.input_port("x")])
//   2. Class attributes set by @pybt.ports decorator (cls.input_ports / cls.output_ports — lists of names)
//   3. Nothing (empty PortsList)
BT::PortsList resolve_ports(nb::handle py_cls, nb::object ports_obj)
{
  BT::PortsList ports;

  if(!ports_obj.is_none())
  {
    // Expect an iterable of (name, PortInfo) pairs.
    for(nb::handle item : ports_obj)
    {
      auto pair = nb::cast<std::pair<std::string, BT::PortInfo>>(item);
      ports.insert(pair);
    }
    return ports;
  }

  // Fall back to @pybt.ports decorator attributes.
  if(nb::hasattr(py_cls, "input_ports"))
  {
    for(nb::handle name : py_cls.attr("input_ports"))
    {
      std::string n = nb::cast<std::string>(name);
      ports.insert(
          BT::CreatePort<BT::AnyTypeAllowed>(BT::PortDirection::INPUT, n));
    }
  }
  if(nb::hasattr(py_cls, "output_ports"))
  {
    for(nb::handle name : py_cls.attr("output_ports"))
    {
      std::string n = nb::cast<std::string>(name);
      ports.insert(
          BT::CreatePort<BT::AnyTypeAllowed>(BT::PortDirection::OUTPUT, n));
    }
  }

  return ports;
}

void register_node_type_impl(BT::BehaviorTreeFactory& self, nb::object py_cls,
                             const std::string& id, nb::object ports_obj)
{
  BT::PortsList ports = resolve_ports(py_cls, ports_obj);

  // For Phase 1 we only support action-flavored nodes (Sync + Stateful).
  // Condition / Control / Decorator come in later phases.
  BT::TreeNodeManifest manifest{ BT::NodeType::ACTION, id, ports, {} };

  BT::NodeBuilder builder =
      [py_cls](const std::string& name,
               const BT::NodeConfig& config) -> std::unique_ptr<BT::TreeNode> {
    nb::gil_scoped_acquire gil;
    nb::object py_inst = py_cls(name, config);
    BT::TreeNode* raw = nb::cast<BT::TreeNode*>(py_inst);
    park_python_instance(raw, std::move(py_inst));
    return std::unique_ptr<BT::TreeNode>(raw);
  };

  self.registerBuilder(manifest, builder);
}

void register_simple_action_impl(BT::BehaviorTreeFactory& self,
                                 const std::string& id, nb::object callable,
                                 nb::object ports_obj)
{
  BT::PortsList ports = resolve_ports(nb::none(), ports_obj);

  BT::SimpleActionNode::TickFunctor tick_functor =
      [callable](BT::TreeNode& node) -> BT::NodeStatus {
    nb::gil_scoped_acquire gil;
    nb::object result = callable(nb::cast(&node));
    return nb::cast<BT::NodeStatus>(result);
  };

  self.registerSimpleAction(id, tick_functor, ports);
}

}  // namespace

void register_factory(nb::module_& m)
{
  nb::class_<BT::BehaviorTreeFactory>(
      m, "BehaviorTreeFactory",
      "Registers node types and builds Tree instances from XML.")
      .def(nb::init<>(), "Construct an empty factory.")

      .def("register_node_type", &register_node_type_impl, "cls"_a, "id"_a,
           "ports"_a = nb::none(),
           "Register a Python class as a node type. The class must be a "
           "subclass of SyncActionNode or StatefulActionNode. Ports may be "
           "supplied as a list of `(name, PortInfo)` pairs, or omitted to use "
           "`cls.input_ports` / `cls.output_ports` (set by `@pybt.ports`).")

      .def("register_simple_action", &register_simple_action_impl, "id"_a,
           "tick_functor"_a, "ports"_a = nb::none(),
           "Register a callable as a synchronous action. The callable takes a "
           "TreeNode and returns a NodeStatus.")

      .def("register_behavior_tree_from_text",
           &BT::BehaviorTreeFactory::registerBehaviorTreeFromText, "xml_text"_a,
           "Pre-register one or more <BehaviorTree> definitions from an XML "
           "string. Instantiate them later with create_tree(name).")

      .def(
          "register_behavior_tree_from_file",
          [](BT::BehaviorTreeFactory& self, const std::string& path) {
            self.registerBehaviorTreeFromFile(std::filesystem::path(path));
          },
          "path"_a,
          "Pre-register the <BehaviorTree> definitions from a file on disk.")

      .def("registered_behavior_trees",
           &BT::BehaviorTreeFactory::registeredBehaviorTrees,
           "Names of every behavior tree currently registered with the factory.")

      .def("clear_registered_behavior_trees",
           &BT::BehaviorTreeFactory::clearRegisteredBehaviorTrees,
           "Forget all registered <BehaviorTree> definitions (registered node "
           "types remain).")

      .def(
          "create_tree_from_text",
          [](BT::BehaviorTreeFactory& self, const std::string& xml) {
            return wrap_and_track(self.createTreeFromText(xml));
          },
          "xml_text"_a,
          "Parse XML and instantiate a Tree in one shot. The XML must contain "
          "either a single <BehaviorTree> or set main_tree_to_execute.")

      .def(
          "create_tree_from_file",
          [](BT::BehaviorTreeFactory& self, const std::string& path) {
            return wrap_and_track(
                self.createTreeFromFile(std::filesystem::path(path)));
          },
          "path"_a, "Read XML from a file and instantiate the resulting Tree.")

      .def(
          "create_tree",
          [](BT::BehaviorTreeFactory& self, const std::string& tree_name) {
            return wrap_and_track(self.createTree(tree_name));
          },
          "tree_name"_a,
          "Instantiate a previously registered behavior tree by name.");
}

}  // namespace pybt
