// Copyright (C) 2020-2024 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_DETAIL_ITERATOR_HPP_INCLUDED
#define LEXY_DETAIL_ITERATOR_HPP_INCLUDED

#include <lexy/_detail/assert.hpp>
#include <lexy/_detail/config.hpp>
#include <lexy/_detail/detect.hpp>
#include <lexy/_detail/std.hpp>

//=== iterator algorithms ===//
namespace lexy::_detail
{
// Can't use std::is_base_of_v<std::random_access_iterator_tag, ...> without including <iterator>.
template <typename Iterator>
using _detect_random_access = decltype(LEXY_DECLVAL(Iterator) - LEXY_DECLVAL(Iterator));
template <typename Iterator>
constexpr auto is_random_access_iterator = is_detected<_detect_random_access, Iterator>;

template <typename Iterator, typename Sentinel>
constexpr std::size_t range_size(Iterator begin, Sentinel end)
{
    if constexpr (std::is_same_v<Iterator, Sentinel> && is_random_access_iterator<Iterator>)
    {
        return static_cast<std::size_t>(end - begin);
    }
    else
    {
        std::size_t result = 0;
        for (auto cur = begin; cur != end; ++cur)
            ++result;
        return result;
    }
}

template <typename Iterator>
constexpr Iterator next(Iterator iter)
{
    return ++iter;
}
template <typename Iterator>
constexpr Iterator next(Iterator iter, std::size_t n)
{
    if constexpr (is_random_access_iterator<Iterator>)
    {
        return iter + n;
    }
    else
    {
        for (auto i = 0u; i != n; ++i)
            ++iter;
        return iter;
    }
}

template <typename Iterator, typename Sentinel>
constexpr Iterator next_clamped(Iterator iter, std::size_t n, Sentinel end)
{
    if constexpr (is_random_access_iterator<Iterator> && std::is_same_v<Iterator, Sentinel>)
    {
        auto remaining = std::size_t(end - iter);
        if (remaining < n)
            return end;
        else
            return iter + n;
    }
    else
    {
        for (auto i = 0u; i != n; ++i)
        {
            if (iter == end)
                break;
            ++iter;
        }
        return iter;
    }
}

// Used for assertions.
template <typename Iterator, typename Sentinel>
constexpr bool precedes([[maybe_unused]] Iterator first, [[maybe_unused]] Sentinel after)
{
    if constexpr (is_random_access_iterator<Iterator> && std::is_same_v<Iterator, Sentinel>)
        return first <= after;
    else
        return true;
}

// Requires: begin <= end_a && begin <= end_b.
// Returns min(end_a, end_b).
template <typename Iterator>
constexpr Iterator min_range_end(Iterator begin, Iterator end_a, Iterator end_b)
{
    if constexpr (is_random_access_iterator<Iterator>)
    {
        LEXY_PRECONDITION(begin <= end_a && begin <= end_b);
        if (end_a <= end_b)
            return end_a;
        else
            return end_b;
    }
    else
    {
        auto cur = begin;
        while (cur != end_a && cur != end_b)
            ++cur;
        return cur;
    }
}

// Requires: begin <= end_a && begin <= end_b.
// Returns max(end_a, end_b).
template <typename Iterator>
constexpr Iterator max_range_end(Iterator begin, Iterator end_a, Iterator end_b)
{
    if constexpr (is_random_access_iterator<Iterator>)
    {
        LEXY_PRECONDITION(begin <= end_a && begin <= end_b);
        if (end_a <= end_b)
            return end_b;
        else
            return end_a;
    }
    else
    {
        auto cur = begin;
        while (true)
        {
            if (cur == end_a)
                return end_b;
            else if (cur == end_b)
                return end_a;

            ++cur;
        }
        return begin; // unreachable
    }
}
} // namespace lexy::_detail

//=== facade classes ===//
namespace lexy::_detail
{
template <typename T>
struct _proxy_pointer
{
    T value;

    constexpr T* operator->() noexcept
    {
        return &value;
    }
};

template <typename Derived, typename T, typename Reference = T&, typename Pointer = const T*>
struct forward_iterator_base
{
    using value_type        = std::remove_cv_t<T>;
    using reference         = Reference;
    using pointer           = lexy::_detail::type_or<Pointer, _proxy_pointer<value_type>>;
    using difference_type   = std::ptrdiff_t;
    using iterator_category = std::forward_iterator_tag;

    constexpr reference operator*() const noexcept
    {
        return static_cast<const Derived&>(*this).deref();
    }
    constexpr pointer operator->() const noexcept
    {
        if constexpr (std::is_void_v<Pointer>)
            return pointer{**this};
        else
            return &**this;
    }

    constexpr Derived& operator++() noexcept
    {
        auto& derived = static_cast<Derived&>(*this);
        derived.increment();
        return derived;
    }
    constexpr Derived operator++(int) noexcept
    {
        auto& derived = static_cast<Derived&>(*this);
        auto  copy    = derived;
        derived.increment();
        return copy;
    }

    friend constexpr bool operator==(const Derived& lhs, const Derived& rhs)
    {
        return lhs.equal(rhs);
    }
    friend constexpr bool operator!=(const Derived& lhs, const Derived& rhs)
    {
        return !lhs.equal(rhs);
    }
};

template <typename Derived, typename T, typename Reference = T&, typename Pointer = const T*>
struct bidirectional_iterator_base : forward_iterator_base<Derived, T, Reference, Pointer>
{
    using iterator_category = std::bidirectional_iterator_tag;

    constexpr Derived& operator--() noexcept
    {
        auto& derived = static_cast<Derived&>(*this);
        derived.decrement();
        return derived;
    }
    constexpr Derived operator--(int) noexcept
    {
        auto& derived = static_cast<Derived&>(*this);
        auto  copy    = derived;
        derived.decrement();
        return copy;
    }
};

template <typename Derived, typename Iterator>
struct sentinel_base
{
    friend constexpr bool operator==(const Iterator& lhs, Derived) noexcept
    {
        return lhs.is_end();
    }
    friend constexpr bool operator!=(const Iterator& lhs, Derived) noexcept
    {
        return !(lhs == Derived{});
    }
    friend constexpr bool operator==(Derived, const Iterator& rhs) noexcept
    {
        return rhs == Derived{};
    }
    friend constexpr bool operator!=(Derived, const Iterator& rhs) noexcept
    {
        return !(rhs == Derived{});
    }
};
} // namespace lexy::_detail

#endif // LEXY_DETAIL_ITERATOR_HPP_INCLUDED

