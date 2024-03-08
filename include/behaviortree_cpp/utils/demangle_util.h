#ifndef DEMANGLE_UTIL_H
#define DEMANGLE_UTIL_H

#include <chrono>
#include <string>
#include <typeindex>

#if defined(__clang__) && defined(__has_include)
#if __has_include(<cxxabi.h>)
#define HAS_CXXABI_H
#endif
#elif defined(__GLIBCXX__) || defined(__GLIBCPP__)
#define HAS_CXXABI_H
#endif

#if defined(HAS_CXXABI_H)
#include <cxxabi.h>
#include <cstdlib>
#include <cstddef>
#endif

namespace BT
{
inline char const* demangle_alloc(char const* name) noexcept;
inline void demangle_free(char const* name) noexcept;

class scoped_demangled_name
{
private:
  char const* m_p;

public:
  explicit scoped_demangled_name(char const* name) noexcept : m_p(demangle_alloc(name))
  {}

  ~scoped_demangled_name() noexcept
  {
    demangle_free(m_p);
  }

  char const* get() const noexcept
  {
    return m_p;
  }

  scoped_demangled_name(scoped_demangled_name const&) = delete;
  scoped_demangled_name& operator=(scoped_demangled_name const&) = delete;
};

#if defined(HAS_CXXABI_H)

inline char const* demangle_alloc(char const* name) noexcept
{
  int status = 0;
  std::size_t size = 0;
  return abi::__cxa_demangle(name, NULL, &size, &status);
}

inline void demangle_free(char const* name) noexcept
{
  std::free(const_cast<char*>(name));
}

#else

inline char const* demangle_alloc(char const* name) noexcept
{
  return name;
}

inline void demangle_free(char const*) noexcept
{}

inline std::string demangle(char const* name)
{
  return name;
}

#endif

inline std::string demangle(const std::type_index& index)
{
  if(index == typeid(std::string))
  {
    return "std::string";
  }
  if(index == typeid(std::string_view))
  {
    return "std::string_view";
  }
  if(index == typeid(std::chrono::seconds))
  {
    return "std::chrono::seconds";
  }
  if(index == typeid(std::chrono::milliseconds))
  {
    return "std::chrono::milliseconds";
  }
  if(index == typeid(std::chrono::microseconds))
  {
    return "std::chrono::microseconds";
  }

  scoped_demangled_name demangled_name(index.name());
  char const* const p = demangled_name.get();
  if(p)
  {
    return p;
  }
  else
  {
    return index.name();
  }
}

inline std::string demangle(const std::type_info& info)
{
  return demangle(std::type_index(info));
}

}  // namespace BT

#undef HAS_CXXABI_H

#endif  // DEMANGLE_UTIL_H
