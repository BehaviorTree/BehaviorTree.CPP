#include "behaviortree_cpp/python_types.h"

#include <pybind11/pybind11.h>
#include <pybind11/detail/typeid.h>

#include "behaviortree_cpp/json_export.h"
#include "behaviortree_cpp/contrib/json.hpp"
#include "behaviortree_cpp/contrib/pybind11_json.hpp"

namespace BT
{

bool toPythonObject(const BT::Any& val, pybind11::object& dest)
{
  nlohmann::json json;
  if (JsonExporter::get().toJson(val, json))
  {
    dest = json;
    return true;
  }

  return false;
}

}   // namespace BT
