#include "behaviortree_cpp/json_export.h"

namespace BT
{

bool JsonExporter::toJson(const Any &any, nlohmann::json &dst) const
{
  nlohmann::json json;
  auto const& type = any.castedType();

  if (any.isString())
  {
    dst = any.cast<std::string>();
  }
  else if (type == typeid(int64_t))
  {
    dst = any.cast<int64_t>();
  }
  else if (type == typeid(uint64_t))
  {
    dst = any.cast<uint64_t>();
  }
  else if (type == typeid(double))
  {
    dst = any.cast<double>();
  }
  else
  {
    auto it = type_converters_.find(type);
    if(it != type_converters_.end())
    {
      it->second(any, dst);
    }
    else {
      return false;
    }
  }
  return true;
}

nlohmann::json ExportBlackboardToJSON(Blackboard &blackboard)
{
  nlohmann::json dest;
  for(auto entry_name: blackboard.getKeys())
  {
    std::string name(entry_name);
    std::unique_lock lk(blackboard.entryMutex());
    if(auto any_ptr = blackboard.getAny(name))
    {
      JsonExporter::get().toJson(*any_ptr, dest[name]);
    }
  }
  return dest;
}

}
