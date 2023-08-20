/***************************************************************************
* Copyright (c) 2019, Martin Renou                                         *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#ifndef PYBIND11_JSON_HPP
#define PYBIND11_JSON_HPP

#include <string>
#include <vector>

#include "behaviortree_cpp/contrib/json.hpp"

#include "pybind11/pybind11.h"

namespace pyjson
{
    namespace py = pybind11;
    namespace nl = nlohmann;

    inline py::object from_json(const nl::json& j)
    {
        if (j.is_null())
        {
            return py::none();
        }
        else if (j.is_boolean())
        {
            return py::bool_(j.get<bool>());
        }
        else if (j.is_number_unsigned())
        {
            return py::int_(j.get<nl::json::number_unsigned_t>());
        }
        else if (j.is_number_integer())
        {
            return py::int_(j.get<nl::json::number_integer_t>());
        }
        else if (j.is_number_float())
        {
            return py::float_(j.get<double>());
        }
        else if (j.is_string())
        {
            return py::str(j.get<std::string>());
        }
        else if (j.is_array())
        {
            py::list obj(j.size());
            for (std::size_t i = 0; i < j.size(); i++)
            {
                obj[i] = from_json(j[i]);
            }
            return obj;
        }
        else // Object
        {
            py::dict obj;
            for (nl::json::const_iterator it = j.cbegin(); it != j.cend(); ++it)
            {
                obj[py::str(it.key())] = from_json(it.value());
            }
            return obj;
        }
    }

    inline nl::json to_json(const py::handle& obj)
    {
        if (obj.ptr() == nullptr || obj.is_none())
        {
            return nullptr;
        }
        if (py::isinstance<py::bool_>(obj))
        {
            return obj.cast<bool>();
        }
        if (py::isinstance<py::int_>(obj))
        {
            try
            {
                nl::json::number_integer_t s = obj.cast<nl::json::number_integer_t>();
                if (py::int_(s).equal(obj))
                {
                    return s;
                }
            }
            catch (...)
            {
            }
            try
            {
                nl::json::number_unsigned_t u = obj.cast<nl::json::number_unsigned_t>();
                if (py::int_(u).equal(obj))
                {
                    return u;
                }
            }
            catch (...)
            {
            }
            throw std::runtime_error("to_json received an integer out of range for both nl::json::number_integer_t and nl::json::number_unsigned_t type: " + py::repr(obj).cast<std::string>());
        }
        if (py::isinstance<py::float_>(obj))
        {
            return obj.cast<double>();
        }
        if (py::isinstance<py::bytes>(obj))
        {
            py::module base64 = py::module::import("base64");
            return base64.attr("b64encode")(obj).attr("decode")("utf-8").cast<std::string>();
        }
        if (py::isinstance<py::str>(obj))
        {
            return obj.cast<std::string>();
        }
        if (py::isinstance<py::tuple>(obj) || py::isinstance<py::list>(obj))
        {
            auto out = nl::json::array();
            for (const py::handle value : obj)
            {
                out.push_back(to_json(value));
            }
            return out;
        }
        if (py::isinstance<py::dict>(obj))
        {
            auto out = nl::json::object();
            for (const py::handle key : obj)
            {
                out[py::str(key).cast<std::string>()] = to_json(obj[key]);
            }
            return out;
        }
        throw std::runtime_error("to_json not implemented for this type of object: " + py::repr(obj).cast<std::string>());
    }
}

// nlohmann_json serializers
namespace nlohmann
{
    namespace py = pybind11;

    #define MAKE_NLJSON_SERIALIZER_DESERIALIZER(T)         \
    template <>                                            \
    struct adl_serializer<T>                               \
    {                                                      \
        inline static void to_json(json& j, const T& obj)  \
        {                                                  \
            j = pyjson::to_json(obj);                      \
        }                                                  \
                                                           \
        inline static T from_json(const json& j)           \
        {                                                  \
            return pyjson::from_json(j);                   \
        }                                                  \
    }

    #define MAKE_NLJSON_SERIALIZER_ONLY(T)                 \
    template <>                                            \
    struct adl_serializer<T>                               \
    {                                                      \
        inline static void to_json(json& j, const T& obj)  \
        {                                                  \
            j = pyjson::to_json(obj);                      \
        }                                                  \
    }

    MAKE_NLJSON_SERIALIZER_DESERIALIZER(py::object);

    MAKE_NLJSON_SERIALIZER_DESERIALIZER(py::bool_);
    MAKE_NLJSON_SERIALIZER_DESERIALIZER(py::int_);
    MAKE_NLJSON_SERIALIZER_DESERIALIZER(py::float_);
    MAKE_NLJSON_SERIALIZER_DESERIALIZER(py::str);

    MAKE_NLJSON_SERIALIZER_DESERIALIZER(py::list);
    MAKE_NLJSON_SERIALIZER_DESERIALIZER(py::tuple);
    MAKE_NLJSON_SERIALIZER_DESERIALIZER(py::dict);

    MAKE_NLJSON_SERIALIZER_ONLY(py::handle);
    MAKE_NLJSON_SERIALIZER_ONLY(py::detail::item_accessor);
    MAKE_NLJSON_SERIALIZER_ONLY(py::detail::list_accessor);
    MAKE_NLJSON_SERIALIZER_ONLY(py::detail::tuple_accessor);
    MAKE_NLJSON_SERIALIZER_ONLY(py::detail::sequence_accessor);
    MAKE_NLJSON_SERIALIZER_ONLY(py::detail::str_attr_accessor);
    MAKE_NLJSON_SERIALIZER_ONLY(py::detail::obj_attr_accessor);

    #undef MAKE_NLJSON_SERIALIZER
    #undef MAKE_NLJSON_SERIALIZER_ONLY
}

// pybind11 caster
namespace pybind11
{
    namespace detail
    {
        template <> struct type_caster<nlohmann::json>
        {
        public:
            PYBIND11_TYPE_CASTER(nlohmann::json, _("json"));

            bool load(handle src, bool)
            {
                try
                {
                    value = pyjson::to_json(src);
                    return true;
                }
                catch (...)
                {
                    return false;
                }
            }

            static handle cast(nlohmann::json src, return_value_policy /* policy */, handle /* parent */)
            {
                object obj = pyjson::from_json(src);
                return obj.release();
            }
        };
    }
}

#endif
