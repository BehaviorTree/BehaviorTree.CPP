#include <memory>
#include <stdexcept>

#include <pybind11/pybind11.h>
#include <pybind11/gil.h>
#include <pybind11/eval.h>
#include <pybind11/chrono.h>
#include <pybind11/pytypes.h>
#include <pybind11/stl.h>

#include "behaviortree_cpp/basic_types.h"
#include "behaviortree_cpp/bt_factory.h"
#include "behaviortree_cpp/action_node.h"
#include "behaviortree_cpp/tree_node.h"

namespace BT
{

namespace py = pybind11;

class Py_SyncActionNode : public SyncActionNode
{
public:
  Py_SyncActionNode(const std::string& name, const NodeConfig& config) :
    SyncActionNode(name, config)
  {}

  NodeStatus tick() override
  {
    PYBIND11_OVERRIDE_PURE_NAME(NodeStatus, Py_SyncActionNode, "tick", tick);
  }
};

class Py_StatefulActionNode final : public StatefulActionNode
{
public:
  Py_StatefulActionNode(const std::string& name, const NodeConfig& config) :
    StatefulActionNode(name, config)
  {}

  NodeStatus onStart() override
  {
    PYBIND11_OVERRIDE_PURE_NAME(NodeStatus, Py_StatefulActionNode, "on_start", onStart);
  }

  NodeStatus onRunning() override
  {
    PYBIND11_OVERRIDE_PURE_NAME(NodeStatus, Py_StatefulActionNode, "on_running",
                                onRunning);
  }

  void onHalted() override
  {
    PYBIND11_OVERRIDE_PURE_NAME(void, Py_StatefulActionNode, "on_running", onRunning);
  }
};

template <typename T>
py::object Py_getInput(const T& node, const std::string& name)
{
  py::object obj;
  node.getInput(name, obj);
  return obj;
}

template <typename T>
void Py_setOutput(T& node, const std::string& name, const py::object& value)
{
  node.setOutput(name, value);
}

// Add a conversion specialization from string values into general py::objects
// by evaluating as a Python expression.
template <>
inline py::object convertFromString(StringView str)
{
  try
  {
    // First, try evaluating the string as-is. Maybe it's a number, a list, a
    // dict, an object, etc.
    return py::eval(str);
  }
  catch (py::error_already_set& e)
  {
    // If that fails, then assume it's a string literal with quotation marks
    // omitted.
    return py::str(str);
  }
}

PYBIND11_MODULE(btpy_cpp, m)
{
  py::class_<PortInfo>(m, "PortInfo");
  m.def("input_port",
        [](const std::string& name) { return InputPort<py::object>(name); });
  m.def("output_port",
        [](const std::string& name) { return OutputPort<py::object>(name); });

  m.def(
      "ports2",
      [](const py::list& inputs, const py::list& outputs) -> auto {
        return [](py::type type) -> auto { return type; };
      },
      py::kw_only(), py::arg("inputs") = py::none(), py::arg("outputs") = py::none());

  py::class_<BehaviorTreeFactory>(m, "BehaviorTreeFactory")
      .def(py::init())
      .def("register",
           [](BehaviorTreeFactory& factory, const py::type type) {
             const std::string name = type.attr("__name__").cast<std::string>();

             TreeNodeManifest manifest;
             manifest.type = NodeType::ACTION;
             manifest.registration_ID = name;
             manifest.ports = {};
             manifest.description = "";

             const auto input_ports = type.attr("input_ports").cast<py::list>();
             for (const auto& name : input_ports)
             {
               manifest.ports.insert(InputPort<py::object>(name.cast<std::string>()));
             }

             const auto output_ports = type.attr("output_ports").cast<py::list>();
             for (const auto& name : output_ports)
             {
               manifest.ports.insert(OutputPort<py::object>(name.cast<std::string>()));
             }

             factory.registerBuilder(
                 manifest,
                 [type](const std::string& name,
                        const NodeConfig& config) -> std::unique_ptr<TreeNode> {
                   py::object obj = type(name, config);
                   // TODO: Increment the object's reference count or else it
                   // will be GC'd at the end of this scope. The downside is
                   // that, unless we can decrement the ref when the unique_ptr
                   // is destroyed, then the object will live forever.
                   obj.inc_ref();

                   if (py::isinstance<Py_SyncActionNode>(obj))
                   {
                     return std::unique_ptr<TreeNode>(obj.cast<Py_SyncActionNode*>());
                   }
                   else if (py::isinstance<Py_StatefulActionNode>(obj))
                   {
                     return std::unique_ptr<TreeNode>(obj.cast<Py_StatefulActionNode*>());
                   }
                   else
                   {
                     throw std::runtime_error("invalid node type of " + name);
                   }
                 });
           })
      .def("create_tree_from_text",
           [](BehaviorTreeFactory& factory, const std::string& text) -> Tree {
             return factory.createTreeFromText(text);
           });

  py::class_<Tree>(m, "Tree")
      .def("tick_once", &Tree::tickOnce)
      .def("tick_exactly_once", &Tree::tickExactlyOnce)
      .def("tick_while_running", &Tree::tickWhileRunning,
           py::arg("sleep_time") = std::chrono::milliseconds(10));

  py::enum_<NodeStatus>(m, "NodeStatus")
      .value("SUCCESS", NodeStatus::SUCCESS)
      .value("FAILURE", NodeStatus::FAILURE)
      .value("IDLE", NodeStatus::IDLE)
      .value("RUNNING", NodeStatus::RUNNING)
      .value("SKIPPED", NodeStatus::SKIPPED)
      .export_values();

  py::class_<NodeConfig>(m, "NodeConfig");

  py::class_<Py_SyncActionNode>(m, "SyncActionNode")
      .def(py::init<const std::string&, const NodeConfig&>())
      .def("get_input", &Py_getInput<Py_SyncActionNode>)
      .def("set_output", &Py_setOutput<Py_SyncActionNode>)
      .def("tick", &Py_SyncActionNode::tick);

  py::class_<Py_StatefulActionNode>(m, "StatefulActionNode")
      .def(py::init<const std::string&, const NodeConfig&>())
      .def("get_input", &Py_getInput<Py_StatefulActionNode>)
      .def("set_output", &Py_setOutput<Py_StatefulActionNode>)
      .def("on_start", &Py_StatefulActionNode::onStart)
      .def("on_running", &Py_StatefulActionNode::onRunning)
      .def("on_halted", &Py_StatefulActionNode::onHalted);
}

}   // namespace BT
