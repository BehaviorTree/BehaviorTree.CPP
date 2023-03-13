#pragma once

#include "behaviortree_cpp/utils/safe_any.hpp"
#include "behaviortree_cpp/blackboard.h"

// Use the version nlohmann::json embedded in BT.CPP
#include "behaviortree_cpp/utils/json.hpp"

namespace BT
{

/**
*  To add new type, you must follow these isntructions:
*    https://json.nlohmann.me/features/arbitrary_types/
*
*  For instance the type Foo requires the implementation:
*
*   void to_json(json& j, const Foo& f);
*
*  Later, you MUST register this calling:
*
*   JsonExporter::get().addConverter<Foo>();
*/

class JsonExporter{

  public:
  static JsonExporter& get() {
    static JsonExporter global_instance;
    return global_instance;
  }

  /**
   * @brief toJson adds the content of "any" to the JSON "destination".
   *
   * It will return false if the conversion toJson is not possible
   * ( you might need to register the converter with addConverter() ).
   */
  bool toJson(const BT::Any& any, nlohmann::json& destination) const;

  template <typename T>
  void toJson(const T& val, nlohmann::json& dst) const {
    dst = val;
  }

  /// Register new JSON converters with addConverter<Foo>(),
  /// But works only if this function is implemented:
  ///
  ///    void nlohmann::to_json(nlohmann::json& destination, const Foo& foo)
  template <typename T> void addConverter()
  {
    auto converter = [](const BT::Any& entry, nlohmann::json& dst) {
      nlohmann::to_json(dst, entry.cast<T>());
    };
    type_converters_.insert( {typeid(T), std::move(converter)} );
  }

  /// Register directly your own converter.
  template <typename T>
  void addConverter(std::function<void(const T&, nlohmann::json&)> to_json)
  {
    auto converter = [=](const BT::Any& entry, nlohmann::json& dst) {
      to_json(entry.cast<T>(), dst);
    };
    type_converters_.insert( {typeid(T), std::move(converter)} );
  }

  private:

  using ToJonConverter = std::function<void(const BT::Any&, nlohmann::json&)>;
  std::unordered_map<std::type_index, ToJonConverter> type_converters_;

};

nlohmann::json ExportBlackboardToJSON(BT::Blackboard& blackboard);

} // namespace BT


