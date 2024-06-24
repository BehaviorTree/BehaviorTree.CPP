// Copyright (C) 2020-2024 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_CALLBACK_ADAPTER_HPP_INCLUDED
#define LEXY_CALLBACK_ADAPTER_HPP_INCLUDED

#include <lexy/callback/base.hpp>

namespace lexy
{
template <typename ReturnType, typename... Fns>
struct _callback : _overloaded<Fns...>
{
    using return_type = ReturnType;

    constexpr explicit _callback(Fns... fns) : _overloaded<Fns...>(LEXY_MOV(fns)...) {}
};

template <typename ReturnType, typename... Fns>
struct _callback_with_state : _overloaded<Fns...>
{
    using return_type = ReturnType;

    template <typename State>
    struct _with_state
    {
        const _callback_with_state& _cb;
        State&                      _state;

        template <typename... Args>
        constexpr return_type operator()(Args&&... args) const&&
        {
            return _cb(_state, LEXY_FWD(args)...);
        }
    };

    constexpr explicit _callback_with_state(Fns... fns) : _overloaded<Fns...>(LEXY_MOV(fns)...) {}

    template <typename State>
    constexpr auto operator[](State& state) const
    {
        return _with_state<State>{*this, state};
    }
};

/// Creates a callback.
template <typename... Fns>
constexpr auto callback(Fns&&... fns)
{
    if constexpr ((lexy::is_callback<std::decay_t<Fns>> && ...))
        return _callback<std::common_type_t<typename std::decay_t<Fns>::return_type...>,
                         std::decay_t<Fns>...>(LEXY_FWD(fns)...);
    else
        return _callback<void, std::decay_t<Fns>...>(LEXY_FWD(fns)...);
}
template <typename ReturnType, typename... Fns>
constexpr auto callback(Fns&&... fns)
{
    return _callback<ReturnType, std::decay_t<Fns>...>(LEXY_FWD(fns)...);
}

/// Creates a callback that also receives the parse state.
template <typename... Fns>
constexpr auto callback_with_state(Fns&&... fns)
{
    if constexpr ((lexy::is_callback<std::decay_t<Fns>> && ...))
        return _callback_with_state<std::common_type_t<typename std::decay_t<Fns>::return_type...>,
                                    std::decay_t<Fns>...>(LEXY_FWD(fns)...);
    else
        return _callback_with_state<void, std::decay_t<Fns>...>(LEXY_FWD(fns)...);
}
template <typename ReturnType, typename... Fns>
constexpr auto callback_with_state(Fns&&... fns)
{
    return _callback_with_state<ReturnType, std::decay_t<Fns>...>(LEXY_FWD(fns)...);
}

template <typename Sink>
struct _cb_from_sink
{
    Sink _sink;

    using _cb         = lexy::sink_callback<Sink>;
    using return_type = typename _cb::return_type;

    template <typename... Args>
    constexpr auto operator()(Args&&... args) const
        -> decltype((LEXY_DECLVAL(_cb&)(LEXY_FWD(args)), ..., LEXY_DECLVAL(_cb&&).finish()))
    {
        auto cb = _sink.sink();
        (cb(LEXY_FWD(args)), ...);
        return LEXY_MOV(cb).finish();
    }
};

/// Creates a callback that forwards all arguments to the sink.
template <typename Sink, typename = lexy::sink_callback<Sink>>
constexpr auto callback(Sink&& sink)
{
    return _cb_from_sink<std::decay_t<Sink>>{LEXY_FWD(sink)};
}
} // namespace lexy

namespace lexy
{
template <typename MemFn>
struct _mem_fn_traits // MemFn is member data
{
    using return_type = MemFn;
};

#define LEXY_MAKE_MEM_FN_TRAITS(...)                                                               \
    template <typename ReturnType, typename... Args>                                               \
    struct _mem_fn_traits<ReturnType(Args...) __VA_ARGS__>                                         \
    {                                                                                              \
        using return_type = ReturnType;                                                            \
    };                                                                                             \
    template <typename ReturnType, typename... Args>                                               \
    struct _mem_fn_traits<ReturnType(Args..., ...) __VA_ARGS__>                                    \
    {                                                                                              \
        using return_type = ReturnType;                                                            \
    };

#define LEXY_MAKE_MEM_FN_TRAITS_CV(...)                                                            \
    LEXY_MAKE_MEM_FN_TRAITS(__VA_ARGS__)                                                           \
    LEXY_MAKE_MEM_FN_TRAITS(const __VA_ARGS__)                                                     \
    LEXY_MAKE_MEM_FN_TRAITS(volatile __VA_ARGS__)                                                  \
    LEXY_MAKE_MEM_FN_TRAITS(const volatile __VA_ARGS__)

#define LEXY_MAKE_MEM_FN_TRAITS_CV_REF(...)                                                        \
    LEXY_MAKE_MEM_FN_TRAITS_CV(__VA_ARGS__)                                                        \
    LEXY_MAKE_MEM_FN_TRAITS_CV(&__VA_ARGS__)                                                       \
    LEXY_MAKE_MEM_FN_TRAITS_CV(&&__VA_ARGS__)

LEXY_MAKE_MEM_FN_TRAITS_CV_REF()
LEXY_MAKE_MEM_FN_TRAITS_CV_REF(noexcept)

#undef LEXY_MAKE_MEM_FN_TRAITS_CV_REF
#undef LEXY_MAKE_MEM_FN_TRAITS_CV
#undef LEXY_MAKE_MEM_FN_TRAITS

template <typename Fn>
struct _mem_fn;
template <typename MemFn, typename T>
struct _mem_fn<MemFn T::*>
{
    MemFn T::*_fn;

    using return_type = typename _mem_fn_traits<MemFn>::return_type;

    template <typename... Args>
    constexpr auto operator()(Args&&... args) const
        -> decltype(_detail::_mem_invoker<MemFn T::*>::invoke(_fn, LEXY_FWD(args)...))
    {
        return _detail::_mem_invoker<MemFn T::*>::invoke(_fn, LEXY_FWD(args)...);
    }
};

/// Creates a callback from a member function.
template <typename MemFn, typename T>
constexpr auto mem_fn(MemFn T::*fn)
{
    return _mem_fn<MemFn T::*>{fn};
}
} // namespace lexy

#endif // LEXY_CALLBACK_ADAPTER_HPP_INCLUDED

