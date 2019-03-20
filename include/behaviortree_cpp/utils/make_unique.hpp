#pragma once

#include <memory>

// https://en.cppreference.com/w/cpp/feature_test
#if not __has_cpp_attribute(__cpp_lib_make_unique)

//The compiler doesn't provide it, so implement it ourselves.

#include <cstddef>
#include <memory>
#include <type_traits>
#include <utility>

namespace std {

template<class _Ty> struct _Unique_if {
    typedef unique_ptr<_Ty> _Single_object;
};

template<class _Ty> struct _Unique_if<_Ty[]> {
    typedef unique_ptr<_Ty[]> _Unknown_bound;
};

template<class _Ty, size_t N> struct _Unique_if<_Ty[N]> {
    typedef void _Known_bound;
};

template<class _Ty, class... Args>
typename _Unique_if<_Ty>::_Single_object
make_unique(Args&&... args) {
    return unique_ptr<_Ty>(new _Ty(std::forward<Args>(args)...));
}

template<class _Ty>
typename _Unique_if<_Ty>::_Unknown_bound
make_unique(size_t n) {
    typedef typename remove_extent<_Ty>::type U;
    return unique_ptr<_Ty>(new U[n]());
}


} // namespace std

#endif // !COMPILER_SUPPORTS_MAKE_UNIQUE

