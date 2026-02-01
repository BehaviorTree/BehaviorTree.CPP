// Copyright (C) 2020-2024 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_DETAIL_LAZY_INIT_HPP_INCLUDED
#define LEXY_DETAIL_LAZY_INIT_HPP_INCLUDED

#include <lexy/_detail/assert.hpp>
#include <lexy/_detail/config.hpp>
#include <lexy/_detail/std.hpp>

namespace lexy::_detail
{
template <typename T>
struct _lazy_init_storage_trivial
{
    bool _init;
    union
    {
        char _empty;
        T    _value;
    };

    constexpr _lazy_init_storage_trivial() noexcept : _init(false), _empty() {}

    template <typename... Args>
    constexpr _lazy_init_storage_trivial(int, Args&&... args)
    : _init(true), _value(LEXY_FWD(args)...)
    {}

    template <typename... Args>
    constexpr void _construct(Args&&... args)
    {
        *this = _lazy_init_storage_trivial(0, LEXY_FWD(args)...);
    }
};

template <typename T>
struct _lazy_init_storage_non_trivial
{
    bool _init;
    union
    {
        char _empty;
        T    _value;
    };

    constexpr _lazy_init_storage_non_trivial() noexcept : _init(false), _empty() {}

    template <typename... Args>
    LEXY_CONSTEXPR_DTOR void _construct(Args&&... args)
    {
        _detail::construct_at(&_value, LEXY_FWD(args)...);
        _init = true;
    }

    // Cannot add noexcept due to https://github.com/llvm/llvm-project/issues/59854.
    LEXY_CONSTEXPR_DTOR ~_lazy_init_storage_non_trivial() /* noexcept */
    {
        if (_init)
            _value.~T();
    }

    LEXY_CONSTEXPR_DTOR _lazy_init_storage_non_trivial(
        _lazy_init_storage_non_trivial&& other) noexcept
    : _init(other._init), _empty()
    {
        if (_init)
            _detail::construct_at(&_value, LEXY_MOV(other._value));
    }

    LEXY_CONSTEXPR_DTOR _lazy_init_storage_non_trivial& operator=(
        _lazy_init_storage_non_trivial&& other) noexcept
    {
        if (_init && other._init)
            _value = LEXY_MOV(other._value);
        else if (_init && !other._init)
        {
            _value.~T();
            _init = false;
        }
        else if (!_init && other._init)
        {
            _detail::construct_at(&_value, LEXY_MOV(other._value));
            _init = true;
        }
        else
        {
            // Both not initialized, nothing to do.
        }

        return *this;
    }
};

template <typename T>
constexpr auto _lazy_init_trivial = [] {
    // https://www.foonathan.net/2021/03/trivially-copyable/
    return std::is_trivially_destructible_v<T>          //
           && std::is_trivially_copy_constructible_v<T> //
           && std::is_trivially_copy_assignable_v<T>    //
           && std::is_trivially_move_constructible_v<T> //
           && std::is_trivially_move_assignable_v<T>;
}();
template <typename T>
using _lazy_init_storage = std::conditional_t<_lazy_init_trivial<T>, _lazy_init_storage_trivial<T>,
                                              _lazy_init_storage_non_trivial<T>>;

template <typename T>
class lazy_init : _lazy_init_storage<T>
{
public:
    using value_type = T;

    constexpr lazy_init() noexcept = default;

    template <typename... Args>
    constexpr T& emplace(Args&&... args)
    {
        if (*this)
            this->_value = T(LEXY_FWD(args)...);
        else
            this->_construct(LEXY_FWD(args)...);

        return this->_value;
    }

    template <typename Fn, typename... Args>
    constexpr T& emplace_result(Fn&& fn, Args&&... args)
    {
        return emplace(LEXY_FWD(fn)(LEXY_FWD(args)...));
    }

    constexpr explicit operator bool() const noexcept
    {
        return this->_init;
    }

    constexpr T& operator*() & noexcept
    {
        LEXY_PRECONDITION(*this);
        return this->_value;
    }
    constexpr const T& operator*() const& noexcept
    {
        LEXY_PRECONDITION(*this);
        return this->_value;
    }
    constexpr T&& operator*() && noexcept
    {
        LEXY_PRECONDITION(*this);
        return LEXY_MOV(this->_value);
    }
    constexpr const T&& operator*() const&& noexcept
    {
        LEXY_PRECONDITION(*this);
        return LEXY_MOV(this->_value);
    }

    constexpr T* operator->() noexcept
    {
        LEXY_PRECONDITION(*this);
        return &this->_value;
    }
    constexpr const T* operator->() const noexcept
    {
        LEXY_PRECONDITION(*this);
        return &this->_value;
    }

private:
    template <typename... Args>
    constexpr explicit lazy_init(int, Args&&... args) noexcept
    : _lazy_init_storage<T>(0, LEXY_FWD(args)...)
    {}
};
template <typename T>
class lazy_init<T&>
{
public:
    using value_type = T&;

    constexpr lazy_init() noexcept : _ptr(nullptr) {}

    constexpr T& emplace(T& ref)
    {
        _ptr = &ref;
        return ref;
    }

    template <typename Fn, typename... Args>
    constexpr T& emplace_result(Fn&& fn, Args&&... args)
    {
        return emplace(LEXY_FWD(fn)(LEXY_FWD(args)...));
    }

    constexpr explicit operator bool() const noexcept
    {
        return _ptr != nullptr;
    }

    constexpr T& operator*() const noexcept
    {
        LEXY_PRECONDITION(*this);
        return *_ptr;
    }

    constexpr T* operator->() const noexcept
    {
        LEXY_PRECONDITION(*this);
        return _ptr;
    }

private:
    T* _ptr;
};
template <>
class lazy_init<void>
{
public:
    using value_type = void;

    constexpr lazy_init() noexcept : _init(false) {}

    constexpr void emplace()
    {
        _init = true;
    }
    template <typename Fn, typename... Args>
    constexpr void emplace_result(Fn&& fn, Args&&... args)
    {
        LEXY_FWD(fn)(LEXY_FWD(args)...);
        _init = true;
    }

    constexpr explicit operator bool() const noexcept
    {
        return _init;
    }

private:
    bool _init;
};
} // namespace lexy::_detail

#endif // LEXY_DETAIL_LAZY_INIT_HPP_INCLUDED

