// json_bridge.hpp — convert between nlohmann::json and Python objects.
//
// Used by bind_tree_node.cpp to bridge port get/set across the BT.CPP
// JsonExporter and Python. Header-only; included only by binding TUs.
//
// Coverage: null, bool, int, float, str, list, tuple, dict.
// Unsupported: bytes, binary blobs, NaN (json default is silently null) —
// users hitting these limits should register a custom JsonExporter converter.

#pragma once

#include "behaviortree_cpp/contrib/json.hpp"

#include <stdexcept>
#include <string>

#include <nanobind/nanobind.h>
#include <nanobind/stl/string.h>

namespace pybt
{

namespace nb = nanobind;
using nlohmann::json;

inline nb::object json_to_python(const json& j)
{
  switch(j.type())
  {
    case json::value_t::null:
      return nb::none();
    case json::value_t::boolean:
      return nb::cast(j.get<bool>());
    case json::value_t::number_integer:
      return nb::cast(j.get<int64_t>());
    case json::value_t::number_unsigned:
      return nb::cast(j.get<uint64_t>());
    case json::value_t::number_float:
      return nb::cast(j.get<double>());
    case json::value_t::string:
      return nb::cast(j.get<std::string>());
    case json::value_t::array: {
      nb::list result;
      for(const auto& el : j)
      {
        result.append(json_to_python(el));
      }
      return result;
    }
    case json::value_t::object: {
      nb::dict result;
      for(auto it = j.begin(); it != j.end(); ++it)
      {
        result[nb::cast(it.key())] = json_to_python(it.value());
      }
      return result;
    }
    case json::value_t::binary:
    case json::value_t::discarded:
    default:
      throw std::runtime_error("json_to_python: unsupported JSON value type");
  }
}

inline json python_to_json(nb::handle obj)
{
  if(obj.is_none())
  {
    return json(nullptr);
  }
  // bool must be checked before int because bool is a subclass of int.
  if(nb::isinstance<nb::bool_>(obj))
  {
    return json(nb::cast<bool>(obj));
  }
  if(nb::isinstance<nb::int_>(obj))
  {
    return json(nb::cast<int64_t>(obj));
  }
  if(nb::isinstance<nb::float_>(obj))
  {
    return json(nb::cast<double>(obj));
  }
  if(nb::isinstance<nb::str>(obj))
  {
    return json(nb::cast<std::string>(obj));
  }
  if(nb::isinstance<nb::list>(obj))
  {
    json arr = json::array();
    for(auto item : nb::cast<nb::list>(obj))
    {
      arr.push_back(python_to_json(nb::handle(item)));
    }
    return arr;
  }
  if(nb::isinstance<nb::tuple>(obj))
  {
    json arr = json::array();
    for(auto item : nb::cast<nb::tuple>(obj))
    {
      arr.push_back(python_to_json(nb::handle(item)));
    }
    return arr;
  }
  if(nb::isinstance<nb::dict>(obj))
  {
    json result = json::object();
    for(auto [k, v] : nb::cast<nb::dict>(obj))
    {
      result[nb::cast<std::string>(k)] = python_to_json(nb::handle(v));
    }
    return result;
  }
  throw std::runtime_error("python_to_json: unsupported Python type — "
                           "register a JsonExporter converter or pass a "
                           "JSON-native value (None/bool/int/float/str/list/dict)");
}

}  // namespace pybt
