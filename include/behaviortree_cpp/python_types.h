#pragma once

#include <pybind11/pybind11.h>

#include "behaviortree_cpp/json_export.h"
#include "behaviortree_cpp/contrib/json.hpp"
#include "behaviortree_cpp/contrib/pybind11_json.hpp"
#include "behaviortree_cpp/utils/safe_any.hpp"

namespace BT
{

/**
 * @brief Generic method to convert Python objects to type T via JSON.
 *
 * For this function to succeed, the type T must be convertible from JSON via
 * the JsonExporter interface.
 */
template <typename T>
bool fromPythonObject(const pybind11::object& obj, T& dest)
{
  if constexpr (nlohmann::detail::is_getable<nlohmann::json, T>::value)
  {
    JsonExporter::get().fromJson<T>(obj, dest);
    return true;
  }

  return false;
}

/**
 * @brief Convert a BT::Any to a Python object via JSON.
 *
 * For this function to succeed, the type stored inside the Any must be
 * convertible to JSON via the JsonExporter interface.
 */
bool toPythonObject(const BT::Any& val, pybind11::object& dest);

}   // namespace BT
