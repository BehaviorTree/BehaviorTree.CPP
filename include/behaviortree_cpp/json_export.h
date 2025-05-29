#pragma once

#include "behaviortree_cpp/basic_types.h"
#include "behaviortree_cpp/utils/safe_any.hpp"
#include "behaviortree_cpp/basic_types.h"

// Use the version nlohmann::json embedded in BT.CPP
#include "behaviortree_cpp/contrib/json.hpp"

namespace BT
{

/**
*  To add new type to the JSON library, you should follow these instructions:
*    https://json.nlohmann.me/features/arbitrary_types/
*
*  Considering for instance the type:
*
*   struct Point2D {
*     double x;
*     double y;
*   };
*
*  This would require the implementation of:
*
*   void to_json(nlohmann::json& j, const Point2D& point);
*   void from_json(const nlohmann::json& j, Point2D& point);
*
*  To avoid repeating yourself, we provide the macro BT_JSON_CONVERTION
*  that implements both those function, at once. Usage:
*
*  BT_JSON_CONVERTER(Point2D, point)
*  {
*     add_field("x", &point.x);
*     add_field("y", &point.y);
*  }
*
*  Later, you MUST register the type using:
*
*  BT::RegisterJsonDefinition<Point2D>();
*/

//-----------------------------------------------------------------------------------

/**
*  Use RegisterJsonDefinition<Foo>();
*/

class JsonExporter
{
public:
  static JsonExporter& get();

  // Delete copy constructors as can only be this one global instance.
  JsonExporter& operator=(JsonExporter&&) = delete;
  JsonExporter& operator=(JsonExporter&) = delete;

  /**
   * @brief toJson adds the content of "any" to the JSON "destination".
   *
   * It will return false if the conversion toJson is not possible
   * If it is a custom type, you might register it first with addConverter().
   */
  bool toJson(const BT::Any& any, nlohmann::json& destination) const;

  /// This information is needed to create a BT::Blackboard::entry
  using Entry = std::pair<BT::Any, BT::TypeInfo>;

  using ExpectedEntry = nonstd::expected<Entry, std::string>;

  /**
   * @brief fromJson will return an Entry (value wrappedn in Any + TypeInfo)
   * from a json source.
   * If it is a custom type, you might register it first with addConverter().
   * @param source
   * @return
   */
  ExpectedEntry fromJson(const nlohmann::json& source) const;

  /// Same as the other, but providing the specific type,
  /// To be preferred if the JSON doesn't contain the field [__type]
  ExpectedEntry fromJson(const nlohmann::json& source, std::type_index type) const;

  template <typename T>
  Expected<T> fromJson(const nlohmann::json& source) const;

  /**
   * @brief Register new JSON converters with addConverter<Foo>().
   * You should used first the macro BT_JSON_CONVERTER.
   * The conversions from/to vector<T> are automatically registered.
   */
  template <typename T>
  void addConverter();

  /**
   * @brief addConverter register a to_json function that converts a json to a type T.
   * The conversion to std:vector<T> is automatically registered.
   *
   * @param to_json the function with signature void(const T&, nlohmann::json&)
   * @param add_type if true, add a field called [__type] with the name of the type.
   */
  template <typename T>
  void addConverter(std::function<void(const T&, nlohmann::json&)> to_json,
                    bool add_type = true);

  /**
   * @brief addConverter register a from_json function that converts a json to a type T.
   * The conversions from std::vector<T> is automatically registered.
   *
   * @param from_json the function with signature void(const nlohmann::json&, T&)
   */
  template <typename T>
  void addConverter(std::function<void(const nlohmann::json&, T&)> from_json);

private:
  using ToJonConverter = std::function<void(const BT::Any&, nlohmann::json&)>;
  using FromJonConverter = std::function<Entry(const nlohmann::json&)>;

  std::unordered_map<std::type_index, ToJonConverter> to_json_converters_;
  std::unordered_map<std::type_index, FromJonConverter> from_json_converters_;
  std::unordered_map<std::type_index, FromJonConverter> from_json_array_converters_;
  std::unordered_map<std::string, BT::TypeInfo> type_names_;
};

template <typename T>
inline Expected<T> JsonExporter::fromJson(const nlohmann::json& source) const
{
  auto res = fromJson(source);
  if(!res)
  {
    return nonstd::make_unexpected(res.error());
  }
  auto casted = res->first.tryCast<T>();
  if(!casted)
  {
    return nonstd::make_unexpected(casted.error());
  }
  return *casted;
}

//-------------------------------------------------------------------

template <typename T>
inline void JsonExporter::addConverter()
{
  // we need to get the name of the type
  nlohmann::json const js = T{};
  // we insert both the name obtained from JSON and demangle
  if(js.contains("__type"))
  {
    type_names_.insert({ std::string(js["__type"]), BT::TypeInfo::Create<T>() });
  }
  type_names_.insert({ BT::demangle(typeid(T)), BT::TypeInfo::Create<T>() });

  ToJonConverter to_converter = [](const BT::Any& entry, nlohmann::json& dst) {
    dst = *const_cast<BT::Any&>(entry).castPtr<T>();
  };
  to_json_converters_.insert({ typeid(T), to_converter });

  FromJonConverter from_converter = [](const nlohmann::json& src) -> Entry {
    T value = src.get<T>();
    return { BT::Any(value), BT::TypeInfo::Create<T>() };
  };

  from_json_converters_.insert({ typeid(T), from_converter });

  //---- include vectors of T
  ToJonConverter to_array_converter = [](const BT::Any& entry, nlohmann::json& dst) {
    dst = *const_cast<BT::Any&>(entry).castPtr<std::vector<T>>();
  };
  to_json_converters_.insert({ typeid(std::vector<T>), to_array_converter });

  FromJonConverter from_array_converter = [](const nlohmann::json& src) -> Entry {
    std::vector<T> value;
    for(const auto& item : src)
    {
      value.push_back(item.get<T>());
    }
    return { BT::Any(value), BT::TypeInfo::Create<std::vector<T>>() };
  };
  from_json_array_converters_.insert({ typeid(T), from_array_converter });
}

template <typename T>
inline void JsonExporter::addConverter(
    std::function<void(const T&, nlohmann::json&)> func, bool add_type)
{
  auto converter = [func, add_type](const BT::Any& entry, nlohmann::json& json) {
    func(entry.cast<T>(), json);
    if(add_type)
    {
      json["__type"] = BT::demangle(typeid(T));
    }
  };
  to_json_converters_.insert({ typeid(T), std::move(converter) });
  //---------------------------------------------
  // add the vector<T> converter
  auto vector_converter = [converter](const BT::Any& entry, nlohmann::json& json) {
    auto& vec = *const_cast<BT::Any&>(entry).castPtr<std::vector<T>>();
    for(const auto& item : vec)
    {
      nlohmann::json item_json;
      converter(BT::Any(item), item_json);
      json.push_back(item_json);
    }
  };
  to_json_converters_.insert({ typeid(std::vector<T>), std::move(vector_converter) });
}

template <typename T>
inline void
JsonExporter::addConverter(std::function<void(const nlohmann::json&, T&)> func)
{
  auto converter = [func](const nlohmann::json& json) -> Entry {
    T tmp;
    func(json, tmp);
    return { BT::Any(tmp), BT::TypeInfo::Create<T>() };
  };
  type_names_.insert({ BT::demangle(typeid(T)), BT::TypeInfo::Create<T>() });
  from_json_converters_.insert({ typeid(T), std::move(converter) });
  //---------------------------------------------
  // add the vector<T> converter
  auto vector_converter = [func](const nlohmann::json& json) -> Entry {
    std::vector<T> tmp;
    for(const auto& item : json)
    {
      T item_tmp;
      func(item, item_tmp);
      tmp.push_back(item_tmp);
    }
    return { BT::Any(tmp), BT::TypeInfo::Create<std::vector<T>>() };
  };
  from_json_array_converters_.insert({ typeid(T), std::move(vector_converter) });
}

template <typename T>
inline void RegisterJsonDefinition()
{
  JsonExporter::get().addConverter<T>();
}

}  // namespace BT

//------------------------------------------------
//------------------------------------------------
//------------------------------------------------

// Macro to implement to_json() and from_json()

#define BT_JSON_CONVERTER(Type, value)                                                   \
  template <class AddField>                                                              \
  void _JsonTypeDefinition(Type&, AddField&);                                            \
                                                                                         \
  inline void to_json(nlohmann::json& js, const Type& p)                                 \
  {                                                                                      \
    auto op = [&js](const char* name, auto* val) { js[name] = *val; };                   \
    _JsonTypeDefinition(const_cast<Type&>(p), op);                                       \
    js["__type"] = #Type;                                                                \
  }                                                                                      \
                                                                                         \
  inline void from_json(const nlohmann::json& js, Type& p)                               \
  {                                                                                      \
    auto op = [&js](const char* name, auto* v) { js.at(name).get_to(*v); };              \
    _JsonTypeDefinition(p, op);                                                          \
  }                                                                                      \
                                                                                         \
  template <class AddField>                                                              \
  inline void _JsonTypeDefinition(Type& value, AddField& add_field)

//end of file
