// Copyright (C) 2020-2024 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_DETAIL_ANY_REF_HPP_INCLUDED
#define LEXY_DETAIL_ANY_REF_HPP_INCLUDED

#include <lexy/_detail/config.hpp>

// Essentially a void*, but we can cast it in a constexpr context.
// The cost is an extra layer of indirection.

namespace lexy::_detail
{
template <typename T>
class any_holder;

// Store a pointer to this instead of a void*.
class any_base
{
public:
    any_base(const any_base&)            = delete;
    any_base& operator=(const any_base&) = delete;

    template <typename T>
    constexpr T& get() noexcept
    {
        return static_cast<any_holder<T>*>(this)->get();
    }
    template <typename T>
    constexpr const T& get() const noexcept
    {
        return static_cast<const any_holder<T>*>(this)->get();
    }

private:
    constexpr any_base() = default;
    ~any_base()          = default;

    template <typename T>
    friend class any_holder;
};

using any_ref  = any_base*;
using any_cref = const any_base*;

// Need to store the object in here.
template <typename T>
class any_holder : public any_base
{
public:
    constexpr explicit any_holder(T&& obj) : _obj(LEXY_MOV(obj)) {}

    constexpr T& get() noexcept
    {
        return _obj;
    }
    constexpr const T& get() const noexcept
    {
        return _obj;
    }

private:
    T _obj;
};
} // namespace lexy::_detail

#endif // LEXY_DETAIL_ANY_REF_HPP_INCLUDED

