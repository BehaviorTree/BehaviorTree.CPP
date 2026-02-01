// Copyright (C) 2020-2024 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_DETAIL_INVOKE_HPP_INCLUDED
#define LEXY_DETAIL_INVOKE_HPP_INCLUDED

#include <lexy/_detail/config.hpp>

namespace lexy::_detail
{
template <typename MemberPtr, bool = std::is_member_object_pointer_v<MemberPtr>>
struct _mem_invoker;
template <typename R, typename ClassT>
struct _mem_invoker<R ClassT::*, true>
{
    static constexpr decltype(auto) invoke(R ClassT::*f, ClassT& object)
    {
        return object.*f;
    }
    static constexpr decltype(auto) invoke(R ClassT::*f, const ClassT& object)
    {
        return object.*f;
    }

    template <typename Ptr>
    static constexpr auto invoke(R ClassT::*f, Ptr&& ptr) -> decltype((*LEXY_FWD(ptr)).*f)
    {
        return (*LEXY_FWD(ptr)).*f;
    }
};
template <typename F, typename ClassT>
struct _mem_invoker<F ClassT::*, false>
{
    template <typename ObjectT, typename... Args>
    static constexpr auto _invoke(int, F ClassT::*f, ObjectT&& object, Args&&... args)
        -> decltype((LEXY_FWD(object).*f)(LEXY_FWD(args)...))
    {
        return (LEXY_FWD(object).*f)(LEXY_FWD(args)...);
    }
    template <typename PtrT, typename... Args>
    static constexpr auto _invoke(short, F ClassT::*f, PtrT&& ptr, Args&&... args)
        -> decltype(((*LEXY_FWD(ptr)).*f)(LEXY_FWD(args)...))
    {
        return ((*LEXY_FWD(ptr)).*f)(LEXY_FWD(args)...);
    }

    template <typename... Args>
    static constexpr auto invoke(F ClassT::*f, Args&&... args)
        -> decltype(_invoke(0, f, LEXY_FWD(args)...))
    {
        return _invoke(0, f, LEXY_FWD(args)...);
    }
};

template <typename ClassT, typename F, typename... Args>
constexpr auto invoke(F ClassT::*f, Args&&... args)
    -> decltype(_mem_invoker<F ClassT::*>::invoke(f, LEXY_FWD(args)...))
{
    return _mem_invoker<F ClassT::*>::invoke(f, LEXY_FWD(args)...);
}

template <typename F, typename... Args>
constexpr auto invoke(F&& f, Args&&... args) -> decltype(LEXY_FWD(f)(LEXY_FWD(args)...))
{
    return LEXY_FWD(f)(LEXY_FWD(args)...);
}
} // namespace lexy::_detail

#endif // LEXY_DETAIL_INVOKE_HPP_INCLUDED

