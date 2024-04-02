#include "behaviortree_cpp/json_export.h"

namespace BT
{

JsonExporter& JsonExporter::get()
{
  static JsonExporter global_instance;
  return global_instance;
}

bool JsonExporter::toJson(const Any& any, nlohmann::json& dst) const
{
  nlohmann::json json;
  auto const& type = any.castedType();

  if(any.isString())
  {
    dst = any.cast<std::string>();
  }
  else if(type == typeid(int64_t))
  {
    dst = any.cast<int64_t>();
  }
  else if(type == typeid(uint64_t))
  {
    dst = any.cast<uint64_t>();
  }
  else if(type == typeid(double))
  {
    dst = any.cast<double>();
  }
  else
  {
    auto it = to_json_converters_.find(type);
    if(it != to_json_converters_.end())
    {
      it->second(any, dst);
    }
    else
    {
      return false;
    }
  }
  return true;
}

JsonExporter::ExpectedEntry JsonExporter::fromJson(const nlohmann::json& source) const
{
  if(source.is_null())
  {
    return nonstd::make_unexpected("json object is null");
  }
  if(source.is_string())
  {
    return Entry{ BT::Any(source.get<std::string>()),
                  BT::TypeInfo::Create<std::string>() };
  }
  if(source.is_number_unsigned())
  {
    return Entry{ BT::Any(source.get<uint64_t>()), BT::TypeInfo::Create<uint64_t>() };
  }
  if(source.is_number_integer())
  {
    return Entry{ BT::Any(source.get<int64_t>()), BT::TypeInfo::Create<int64_t>() };
  }
  if(source.is_number_float())
  {
    return Entry{ BT::Any(source.get<double>()), BT::TypeInfo::Create<double>() };
  }
  if(source.is_boolean())
  {
    return Entry{ BT::Any(source.get<bool>()), BT::TypeInfo::Create<bool>() };
  }

  if(!source.contains("__type"))
  {
    return nonstd::make_unexpected("Missing field '__type'");
  }
  auto type_it = type_names_.find(source["__type"]);
  if(type_it == type_names_.end())
  {
    return nonstd::make_unexpected("Type not found in registered list");
  }
  auto func_it = from_json_converters_.find(type_it->second.type());
  if(func_it == from_json_converters_.end())
  {
    return nonstd::make_unexpected("Type not found in registered list");
  }
  return func_it->second(source);
}

JsonExporter::ExpectedEntry JsonExporter::fromJson(const nlohmann::json& source,
                                                   std::type_index type) const
{
  auto func_it = from_json_converters_.find(type);
  if(func_it == from_json_converters_.end())
  {
    return nonstd::make_unexpected("Type not found in registered list");
  }
  return func_it->second(source);
}

}  // namespace BT
