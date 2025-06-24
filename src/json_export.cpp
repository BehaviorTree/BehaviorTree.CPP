#include "behaviortree_cpp/json_export.h"

namespace
{
constexpr std::string_view kTypeField = "__type";
constexpr std::string_view kValueField = "value";
}  // namespace

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
    dst[kTypeField] = "string";
    dst[kValueField] = any.cast<std::string>();
  }
  else if(type == typeid(int64_t))
  {
    dst[kTypeField] = "int64_t";
    dst[kValueField] = any.cast<int64_t>();
  }
  else if(type == typeid(uint64_t))
  {
    dst[kTypeField] = "uint64_t";
    dst[kValueField] = any.cast<uint64_t>();
  }
  else if(type == typeid(double))
  {
    dst[kTypeField] = "double";
    dst[kValueField] = any.cast<double>();
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
      dst[kTypeField] = demangle(type);
      dst[kValueField] = any.empty() ? "no value set" : "no JSON converter registered";
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
  if(!source.contains(kTypeField))
  {
    return nonstd::make_unexpected("Missing field '" + std::string(kTypeField) + "'.");
  }

  const auto source_value_it = source.find(kValueField);
  if(source_value_it != source.end())
  {
    if(source_value_it->is_string())
    {
      return Entry{ BT::Any(source_value_it->get<std::string>()),
                    BT::TypeInfo::Create<std::string>() };
    }
    if(source_value_it->is_number_unsigned())
    {
      return Entry{ BT::Any(source_value_it->get<uint64_t>()),
                    BT::TypeInfo::Create<uint64_t>() };
    }
    if(source_value_it->is_number_integer())
    {
      return Entry{ BT::Any(source_value_it->get<int64_t>()),
                    BT::TypeInfo::Create<int64_t>() };
    }
    if(source_value_it->is_number_float())
    {
      return Entry{ BT::Any(source_value_it->get<double>()),
                    BT::TypeInfo::Create<double>() };
    }
    if(source_value_it->is_boolean())
    {
      return Entry{ BT::Any(source_value_it->get<bool>()), BT::TypeInfo::Create<bool>() };
    }
  }

  auto type_it = type_names_.find(source[kTypeField]);
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
