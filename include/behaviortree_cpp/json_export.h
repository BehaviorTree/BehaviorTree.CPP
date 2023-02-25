#pragma once

#include "nlohmann/json.hpp"
#include "behaviortree_cpp/utils/safe_any.hpp"

namespace BT
{

/**
*  To add new type, you must follow these isntructions:
*    https://json.nlohmann.me/features/arbitrary_types/
*
*  For instance the type Foo requires the implementation:
*
*   namespace nlohmann {
*      void to_json(json& j, const Foo& f);
*   }
*
*  Later, you MUST register this calling:
*
*   JsonExporter::get().addConverter<Foo>();
*/

class JsonExporter{

  public:
  JsonExporter& get() {
    static JsonExporter global_instance;
    return global_instance;
  }

  void toJson(const BT::Any& entry, nlohmann::json& dst, bool throw_if_unregistered = true) const;

  template <typename T>
  void toJson(const T& val, nlohmann::json& dst) const {
    dst = val;
  }

  /// Register new JSON converters with addConverter<Foo>(),
  /// But works only if this function is implemented:
  ///    nlohmann::to_json(json& j, const Foo& f)
  template <typename T> void addConverter()
  {
    auto converter = [](const BT::Any& entry, nlohmann::json& dst) {
      dst = entry.cast<T>();
    };
    type_converters_.insert( {typeid(T), std::move(converter)} );
  }

  private:

  using ToJonConverter = std::function<void(const BT::Any&, nlohmann::json&)>;
  std::unordered_map<std::type_index, ToJonConverter> type_converters_;

};

}
