#ifndef STRCAT_HPP
#define STRCAT_HPP

#include <string_view>
#include <string>

namespace BT
{

// -----------------------------------------------------------------------------
// StrCat()
// -----------------------------------------------------------------------------
//
// Merges given strings, using no delimiter(s).
//
// `StrCat()` is designed to be the fastest possible way to construct a string
// out of a mix of raw C strings, string_views, strings.

namespace strings_internal
{

inline void AppendPieces(std::string* dest,
                         std::initializer_list<std::string_view> pieces)
{
  size_t size = 0;
  for(const auto& piece : pieces)
  {
    size += piece.size();
  }
  dest->reserve(dest->size() + size);
  for(const auto& piece : pieces)
  {
    dest->append(piece.data(), piece.size());
  }
}

inline std::string CatPieces(std::initializer_list<std::string_view> pieces)
{
  std::string out;
  AppendPieces(&out, std::move(pieces));
  return out;
}

}  // namespace strings_internal

inline std::string StrCat()
{
  return std::string();
}

inline std::string StrCat(const std::string_view& a)
{
  return std::string(a.data(), a.size());
}

inline std::string StrCat(const std::string_view& a, const std::string_view& b)
{
  return strings_internal::CatPieces({ a, b });
}

inline std::string StrCat(const std::string_view& a, const std::string_view& b,
                          const std::string_view& c)
{
  return strings_internal::CatPieces({ a, b, c });
}

// Support 4 or more arguments
template <typename... AV>
inline std::string StrCat(const std::string_view& a, const std::string_view& b,
                          const std::string_view& c, const std::string_view& d,
                          const AV&... args)
{
  return strings_internal::CatPieces(
      { a, b, c, d, static_cast<const std::string_view&>(args)... });
}

//-----------------------------------------------

inline void StrAppend(std::string* destination, const std::string_view& a)
{
  destination->append(a.data(), a.size());
}

inline void StrAppend(std::string* destination, const std::string_view& a,
                      const std::string_view& b)
{
  strings_internal::AppendPieces(destination, { a, b });
}

inline void StrAppend(std::string* destination, const std::string_view& a,
                      const std::string_view& b, const std::string_view& c)
{
  strings_internal::AppendPieces(destination, { a, b, c });
}

// Support 4 or more arguments
template <typename... AV>
inline void StrAppend(std::string* destination, const std::string_view& a,
                      const std::string_view& b, const std::string_view& c,
                      const std::string_view& d, const AV&... args)
{
  strings_internal::AppendPieces(
      destination, { a, b, c, d, static_cast<const std::string_view&>(args)... });
}

}  // namespace BT

#endif  // STRCAT_HPP
