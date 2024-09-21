// Copyright (C) 2020-2024 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_VISUALIZE_HPP_INCLUDED
#define LEXY_VISUALIZE_HPP_INCLUDED

#include <cstdio>
#include <lexy/_detail/config.hpp>
#include <lexy/dsl/code_point.hpp>
#include <lexy/input/range_input.hpp>
#include <lexy/lexeme.hpp>

//=== visualization_options ===//
namespace lexy
{
enum visualization_flags
{
    visualize_default = 0,

    /// Visualization can use unicode characters.
    visualize_use_unicode = 1 << 0,
    /// Visualization can use ANSI color escape sequences.
    visualize_use_color = 1 << 1,
    /// Visualization can use Unicode symbols e.g. for newline/space instead of the code point.
    visualize_use_symbols = 1 << 2,

    /// Visualization can use unicode, color and symbols.
    visualize_fancy = visualize_use_unicode | visualize_use_color | visualize_use_symbols,

    /// Visualize space ' ' as visible character/symbol.
    visualize_space = 1 << 3,
};

constexpr visualization_flags operator|(visualization_flags lhs, visualization_flags rhs) noexcept
{
    return visualization_flags(int(lhs) | int(rhs));
}

/// Options that control visualization.
struct visualization_options
{
    static constexpr unsigned char max_tree_depth_limit = 32;

    /// Boolean flags.
    visualization_flags flags = visualize_default;

    /// The maximal depth when visualizing a tree.
    /// Must be <= max_tree_depth_limit.
    unsigned char max_tree_depth = max_tree_depth_limit;
    /// The maximal width when visualizing a lexeme.
    /// 0 means unlimited.
    unsigned char max_lexeme_width = 0;
    /// How many spaces are printed for a tab character.
    /// 0 means it is printed as escaped control character.
    unsigned char tab_width = 0;

    constexpr bool is_set(visualization_flags f) const noexcept
    {
        return (flags & f) != 0;
    }

    constexpr visualization_options reset(visualization_flags f) const noexcept
    {
        auto copy  = *this;
        copy.flags = visualization_flags(copy.flags & ~f);
        return copy;
    }

    friend constexpr visualization_options operator|(visualization_options lhs,
                                                     visualization_flags   rhs) noexcept
    {
        lhs.flags = lhs.flags | rhs;
        return lhs;
    }
};
} // namespace lexy

//=== visualize_to ===//
namespace lexy::_detail
{
template <typename Encoding>
constexpr auto make_literal_lexeme(const typename Encoding::char_type* str, std::size_t length)
{
    struct reader
    {
        using encoding = Encoding;
        using iterator = const typename Encoding::char_type*;
    };

    return lexy::lexeme<reader>(str, str + length);
}

template <typename OutIt>
constexpr OutIt write_str(OutIt out, const char* str)
{
    while (*str)
        *out++ = *str++;
    return out;
}
template <typename OutIt>
constexpr OutIt write_str(OutIt out, const LEXY_CHAR8_T* str)
{
    while (*str)
        *out++ = static_cast<char>(*str++);
    return out;
}

template <int N = 16, typename OutIt, typename... Args>
constexpr OutIt write_format(OutIt out, const char* fmt, const Args&... args)
{
    char buffer[std::size_t(N + 1)];
    auto count = std::snprintf(buffer, N, fmt, args...);
    LEXY_ASSERT(count <= N, "buffer not big enough");

    for (auto i = 0; i != count; ++i)
        *out++ = buffer[i];
    return out;
}

enum class color
{
    reset  = 0,
    bold   = 1,
    faint  = 2,
    italic = 3,

    black   = 30,
    red     = 31,
    green   = 32,
    yellow  = 33,
    blue    = 34,
    magenta = 35,
    cyan    = 36,
    white   = 37,
};

template <color CodeHead, color... CodeTail, typename OutIt>
constexpr OutIt write_color(OutIt out, visualization_options opts)
{
    if (!opts.is_set(visualize_use_color))
        return out;

    out = write_str(out, "\033[");

    auto write_code = [](OutIt out, int code) {
        if (code > 10)
        {
            *out++ = static_cast<char>((code / 10) + '0');
            *out++ = static_cast<char>((code % 10) + '0');
        }
        else
        {
            *out++ = static_cast<char>(code + '0');
        }
        return out;
    };
    out = write_code(out, int(CodeHead));
    ((*out++ = ';', write_code(out, int(CodeTail))), ...);

    *out++ = 'm';
    return out;
}

template <typename OutIt>
constexpr OutIt write_ellipsis(OutIt out, visualization_options opts)
{
    if (opts.is_set(visualize_use_unicode))
        return _detail::write_str(out, u8"…");
    else
        return _detail::write_str(out, "...");
}

template <typename OutIt, typename Fn>
constexpr OutIt write_special_char(OutIt out, visualization_options opts, Fn inner)
{
    out = _detail::write_color<_detail::color::cyan, _detail::color::faint>(out, opts);
    if (opts.is_set(visualize_use_unicode))
        out = _detail::write_str(out, u8"⟨");
    else
        out = _detail::write_str(out, "\\");

    out = inner(out);

    if (opts.is_set(visualize_use_unicode))
        out = _detail::write_str(out, u8"⟩");
    out = _detail::write_color<_detail::color::reset>(out, opts);
    return out;
}
} // namespace lexy::_detail

namespace lexy
{
template <typename OutputIt>
OutputIt visualize_to(OutputIt out, lexy::code_point cp, visualization_options opts = {})
{
    if (!cp.is_valid())
    {
        out = _detail::write_special_char(out, opts, [opts](OutputIt out) {
            if (opts.is_set(visualize_use_unicode))
                return _detail::write_str(out, "U+????");
            else
                return _detail::write_str(out, "u????");
        });
        return out;
    }
    else if (cp.is_control())
    {
        auto c = static_cast<int>(cp.value());
        switch (c)
        {
        case '\0':
            out = _detail::write_special_char(out, opts, [opts](OutputIt out) {
                if (opts.is_set(visualize_use_unicode))
                    return _detail::write_str(out, "NUL");
                else
                    return _detail::write_str(out, "0");
            });
            break;
        case '\r':
            out = _detail::write_special_char(out, opts, [opts](OutputIt out) {
                if (opts.is_set(visualize_use_unicode))
                    return _detail::write_str(out, "CR");
                else
                    return _detail::write_str(out, "r");
            });
            break;

        case '\n':
            if (opts.is_set(visualize_use_symbols))
            {
                out = _detail::write_color<_detail::color::faint>(out, opts);
                out = _detail::write_str(out, u8"⏎");
                out = _detail::write_color<_detail::color::reset>(out, opts);
            }
            else if (opts.is_set(visualize_use_unicode))
            {
                out = _detail::write_special_char(out, opts, [](OutputIt out) {
                    return _detail::write_str(out, "LF");
                });
            }
            else
            {
                out = _detail::write_special_char(out, opts, [](OutputIt out) {
                    return _detail::write_str(out, "n");
                });
            }
            break;
        case '\t':
            if (opts.tab_width > 0)
            {
                // We print that many space characters.
                // This is recursion, but the recursive call does not recurse further.
                for (auto i = 0u; i < opts.tab_width; ++i)
                    out = visualize_to(out, lexy::code_point(' '), opts);
            }
            else if (opts.is_set(visualize_use_symbols))
            {
                out = _detail::write_color<_detail::color::faint>(out, opts);
                out = _detail::write_str(out, u8"⇨");
                out = _detail::write_color<_detail::color::reset>(out, opts);
            }
            else if (opts.is_set(visualize_use_unicode))
            {
                out = _detail::write_special_char(out, opts, [](OutputIt out) {
                    return _detail::write_str(out, "HT");
                });
            }
            else
            {
                out = _detail::write_special_char(out, opts, [](OutputIt out) {
                    return _detail::write_str(out, "t");
                });
            }
            break;

        default:
            out = _detail::write_special_char(out, opts, [opts, c](OutputIt out) {
                if (opts.is_set(visualize_use_unicode))
                    return _detail::write_format(out, "U+%04X", c);
                else
                    return _detail::write_format(out, "u%04X", c);
            });
            break;
        }
        return out;
    }
    else if (cp.value() == ' ')
    {
        if (opts.is_set(visualize_space))
        {
            if (opts.is_set(visualize_use_symbols))
            {
                out = _detail::write_color<_detail::color::faint>(out, opts);
                out = _detail::write_str(out, u8"␣");
                out = _detail::write_color<_detail::color::reset>(out, opts);
            }
            else if (opts.is_set(visualize_use_unicode))
            {
                out = _detail::write_special_char(out, opts, [](OutputIt out) {
                    return _detail::write_str(out, "SP");
                });
            }
            else
            {
                out = _detail::write_special_char(out, opts, [](OutputIt out) {
                    return _detail::write_str(out, "u0020");
                });
            }
        }
        else
        {
            *out++ = ' ';
        }

        return out;
    }
    else if (cp.value() == '\\')
    {
        if (!opts.is_set(visualize_use_unicode))
            out = _detail::write_special_char(out, opts, [](OutputIt out) {
                return _detail::write_str(out, "\\");
            });
        else
            *out++ = '\\'; // Doesn't need escaping if we can use unicode.
        return out;
    }
    else if (cp.is_ascii())
    {
        // ASCII, non-control character, print as-is.
        *out++ = static_cast<char>(cp.value());
        return out;
    }
    else
    {
        out = _detail::write_special_char(out, opts, [opts, cp](OutputIt out) {
            auto c = static_cast<int>(cp.value());
            if (opts.is_set(visualize_use_unicode))
                return _detail::write_format(out, "U+%04X", c);
            else if (cp.is_bmp())
                return _detail::write_format(out, "u%04X", c);
            else
                return _detail::write_format(out, "U%08X", c);
        });
        return out;
    }
}

template <typename OutputIt, typename Reader>
OutputIt visualize_to(OutputIt out, lexy::lexeme<Reader> lexeme,
                      [[maybe_unused]] visualization_options opts = {})
{
    // We need to ensure that we're not printing more "code points" than `opts.max_lexeme_width`,
    // or unlimited if `opts.max_lexeme_width == 0`.
    // The trick is to count and check for `count == opts.max_lexeme_width` after increment.

    [[maybe_unused]] auto write_escaped_byte = [opts](OutputIt out, unsigned char byte) {
        return _detail::write_special_char(out, opts, [opts, byte](OutputIt out) {
            if (opts.is_set(visualize_use_unicode))
                return _detail::write_format(out, "0x%02X", byte);
            else
                return _detail::write_format(out, "x%02X", byte);
        });
    };

    using encoding = typename Reader::encoding;
    if constexpr (lexy::is_unicode_encoding<encoding>)
    {
        // Parse the individual code points, and write them out.
        lexy::range_input<encoding, typename Reader::iterator> input(lexeme.begin(), lexeme.end());
        auto                                                   reader = input.reader();

        auto count = 0u;
        while (true)
        {
            if (auto result = lexy::_detail::parse_code_point(reader);
                result.error == lexy::_detail::cp_error::eof)
            {
                // No more code points in the lexeme, finish.
                break;
            }
            else if (result.error == lexy::_detail::cp_error::success)
            {
                // Consume and visualize.
                reader.reset(result.end);
                out = visualize_to(out, lexy::code_point(result.cp), opts);
            }
            else
            {
                // Recover from the failed code point.
                auto begin = reader.position();
                lexy::_detail::recover_code_point(reader, result);
                auto end = reader.position();

                // Visualize each skipped code unit as byte.
                for (auto cur = begin; cur != end; ++cur)
                {
                    if constexpr (std::is_same_v<encoding,
                                                 lexy::utf8_encoding> //
                                  || std::is_same_v<encoding, lexy::utf8_char_encoding>)
                    {
                        out = write_escaped_byte(out, static_cast<unsigned char>(*cur & 0xFF));
                    }
                    else if constexpr (std::is_same_v<encoding, lexy::utf16_encoding>)
                    {
                        auto first  = static_cast<unsigned char>((*cur >> 8) & 0xFF);
                        auto second = static_cast<unsigned char>(*cur & 0xFF);

                        if (first != 0)
                            out = write_escaped_byte(out, first);
                        out = write_escaped_byte(out, second);
                    }
                    else if constexpr (std::is_same_v<encoding, lexy::utf32_encoding>)
                    {
                        auto first  = static_cast<unsigned char>((*cur >> 24) & 0xFF);
                        auto second = static_cast<unsigned char>((*cur >> 16) & 0xFF);
                        auto third  = static_cast<unsigned char>((*cur >> 8) & 0xFF);
                        auto fourth = static_cast<unsigned char>(*cur & 0xFF);

                        if (first != 0)
                            out = write_escaped_byte(out, first);
                        if (first != 0 || second != 0)
                            out = write_escaped_byte(out, second);
                        if (first != 0 || second != 0 || third != 0)
                            out = write_escaped_byte(out, third);
                        out = write_escaped_byte(out, fourth);
                    }
                }
            }

            ++count;
            if (count == opts.max_lexeme_width)
            {
                out = _detail::write_ellipsis(out, opts);
                break;
            }
        }
        return out;
    }
    else if constexpr (lexy::is_text_encoding<encoding>)
    {
        auto count = 0u;
        for (char c : lexeme)
        {
            // If the character is in fact ASCII, visualize the code point.
            // Otherwise, visualize as byte.
            if (lexy::_detail::is_ascii(c))
                out = visualize_to(out, lexy::code_point(static_cast<char32_t>(c)), opts);
            else
                out = write_escaped_byte(out, static_cast<unsigned char>(c));

            ++count;
            if (count == opts.max_lexeme_width)
            {
                out = _detail::write_ellipsis(out, opts);
                break;
            }
        }
        return out;
    }
    else if constexpr (lexy::is_byte_encoding<encoding>)
    {
        auto count = 0u;
        for (auto iter = lexeme.begin(); iter != lexeme.end(); ++iter)
        {
            // Write each byte.
            out = _detail::write_special_char(out, opts, [c = *iter](OutputIt out) {
                return _detail::write_format(out, "%02X", c);
            });

            ++count;
            if (count == opts.max_lexeme_width)
            {
                out = _detail::write_ellipsis(out, opts);
                break;
            }
        }
        return out;
    }
    else if constexpr (lexy::is_node_encoding<encoding>)
    {
        // Visualize as an iterator range of characters.
        lexy::range_input<typename encoding::char_encoding, typename Reader::iterator>
            input(lexeme.begin(), lexeme.end());
        return visualize_to(out, lexy::lexeme_for<decltype(input)>(input.begin(), input.end()));
    }
    else
    {
        static_assert(lexy::_detail::error<encoding>, "unknown encoding");
        return out;
    }
}

template <typename OutputIt, typename Tree, typename = decltype(LEXY_DECLVAL(Tree&).traverse())>
OutputIt visualize_to(OutputIt out, const Tree& tree, visualization_options opts = {})
{
    struct label_t
    {
        const LEXY_CHAR_OF_u8* space;
        const LEXY_CHAR_OF_u8* line;
        const LEXY_CHAR_OF_u8* end;
        const LEXY_CHAR_OF_u8* branch;
    };
    auto label = opts.is_set(visualize_use_unicode) ? label_t{u8"   ", u8"│  ", u8"└──", u8"├──"}
                                                    : label_t{u8"  ", u8"  ", u8"- ", u8"- "};

    // True if the node currently opened at the depth is the last child of its parent,
    // false otherwise.
    bool is_last_child[visualization_options::max_tree_depth_limit] = {};
    LEXY_PRECONDITION(opts.max_tree_depth <= visualization_options::max_tree_depth_limit);

    // Writes the prefix using the last child information.
    auto write_prefix
        = [opts, label, &is_last_child](OutputIt out, std::size_t cur_depth, bool cur_is_last) {
              if (cur_depth == 0)
                  // Root node doesn't have a prefix.
                  return out;

              out = _detail::write_color<_detail::color::faint>(out, opts);

              // We begin at depth 1, as depth 0 doesn't require a prefix.
              for (auto i = 1u; i < cur_depth; ++i)
                  if (is_last_child[i])
                      // If the current node at that depth is the last child, we just indent.
                      out = _detail::write_str(out, label.space);
                  else
                      // Otherwise, we need to carry the line.
                      out = _detail::write_str(out, label.line);

              // Print the final branching symbol for the current node.
              if (cur_is_last)
                  out = _detail::write_str(out, label.end);
              else
                  out = _detail::write_str(out, label.branch);

              out = _detail::write_color<_detail::color::reset>(out, opts);
              return out;
          };

    // Traverse and print the tree.
    std::size_t cur_depth = 0;
    for (auto [event, node] : tree.traverse())
    {
        auto last_child = node.is_last_child();

        using event_t = typename decltype(tree.traverse())::event;
        switch (event)
        {
        case event_t::enter:
            if (cur_depth <= opts.max_tree_depth)
            {
                out = write_prefix(out, cur_depth, last_child);

                out = _detail::write_color<_detail::color::bold>(out, opts);
                out = _detail::write_str(out, node.kind().name());
                out = _detail::write_color<_detail::color::reset>(out, opts);

                if (cur_depth == opts.max_tree_depth)
                {
                    // Print an ellipsis instead of children.
                    out = _detail::write_str(out, ": ");
                    out = _detail::write_ellipsis(out, opts);
                    out = _detail::write_str(out, "\n");
                }
                else
                {
                    // Print a newline and prepare for children.
                    out                      = _detail::write_str(out, ":\n");
                    is_last_child[cur_depth] = last_child;
                }
            }
            ++cur_depth;
            break;

        case event_t::exit:
            --cur_depth;
            break;

        case event_t::leaf:
            if (cur_depth <= opts.max_tree_depth)
            {
                out = write_prefix(out, cur_depth, last_child);

                out = _detail::write_color<_detail::color::bold>(out, opts);
                out = _detail::write_str(out, node.kind().name());
                out = _detail::write_color<_detail::color::reset>(out, opts);

                if (!node.lexeme().empty())
                {
                    out = _detail::write_str(out, ": ");
                    out = visualize_to(out, node.lexeme(), opts | lexy::visualize_space);
                }

                out = _detail::write_str(out, "\n");
            }
            break;
        }
    }

    return out;
}
} // namespace lexy

//=== visualize ===//
namespace lexy
{
struct cfile_output_iterator
{
    std::FILE* _file;

    explicit constexpr cfile_output_iterator(std::FILE* file) : _file(file) {}

    auto operator*() const noexcept
    {
        return *this;
    }
    auto operator++(int) const noexcept
    {
        return *this;
    }

    cfile_output_iterator& operator=(char c)
    {
        std::fputc(c, _file);
        return *this;
    }
};

struct stderr_output_iterator
{
    auto operator*() const noexcept
    {
        return *this;
    }
    auto operator++(int) const noexcept
    {
        return *this;
    }

    stderr_output_iterator& operator=(char c)
    {
        std::fputc(c, stderr);
        return *this;
    }
};
struct stdout_output_iterator
{
    auto operator*() const noexcept
    {
        return *this;
    }
    auto operator++(int) const noexcept
    {
        return *this;
    }

    stdout_output_iterator& operator=(char c)
    {
        std::fputc(c, stdout);
        return *this;
    }
};

/// Writes the visualization to the FILE.
template <typename T>
void visualize(std::FILE* file, const T& obj, visualization_options opts = {})
{
    visualize_to(cfile_output_iterator{file}, obj, opts);
}
} // namespace lexy

//=== visualization_display_width ===//
namespace lexy
{
template <typename T>
std::size_t visualization_display_width(const T& obj, visualization_options opts = {})
{
    struct iterator
    {
        std::size_t width;

        iterator& operator*() noexcept
        {
            return *this;
        }
        iterator& operator++(int) noexcept
        {
            return *this;
        }

        iterator& operator=(char c)
        {
            // We're having ASCII or UTF-8 characters.
            // All unicode characters used occupy a single cell,
            // so we just need to count all code units that are not continuation bytes.
            auto value = static_cast<unsigned char>(c);
            if ((value & 0b1100'0000) != 0b1000'0000)
                ++width;
            return *this;
        }
    };

    // Disable usage of color, as they introduce additional characters that must nobe counted.
    return visualize_to(iterator{0}, obj, opts.reset(visualize_use_color)).width;
}
} // namespace lexy

#endif // LEXY_VISUALIZE_HPP_INCLUDED

