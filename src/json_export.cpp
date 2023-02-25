#include "behaviortree_cpp/json_export.h"

void BT::JsonExporter::toJson(const Any &entry, nlohmann::json &dst,
                              bool throw_if_unregistered) const
{
  nlohmann::json json;
  auto const& type = entry.castedType();

  if (entry.isString())
  {
    dst = entry.cast<std::string>();
  }
  else if (type == typeid(int64_t))
  {
    dst = entry.cast<int64_t>();
  }
  else if (type == typeid(uint64_t))
  {
    dst = entry.cast<uint64_t>();
  }
  else if (type == typeid(double))
  {
    dst = entry.cast<double>();
  }
  else
  {
    auto it = type_converters_.find(type);
    if(it != type_converters_.end())
    {
      it->second(entry, dst);
    }
    else if(throw_if_unregistered)
    {
      throw std::logic_error("JSON converted not registered. "
                             "Use JsonExporter::addConverter<T>()");
    }
  }
}
