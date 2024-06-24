// Copyright (C) 2020-2024 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_EXT_REPORT_ERROR_HPP_INCLUDED
#define LEXY_EXT_REPORT_ERROR_HPP_INCLUDED

#include <cstdio>
#include <lexy/_detail/assert.hpp>
#include <lexy/error.hpp>
#include <lexy/input_location.hpp>
#include <lexy/visualize.hpp>

namespace lexy_ext
{
/// The kind of diagnostic message.
enum class diagnostic_kind
{
    error,
    warning,
    note,
    info,
    debug,
    fixit,
    help,
};

/// Classifies a source code annotation.
enum class annotation_kind
{
    // foo
    // ^^^ primary annotation
    primary,
    // bar
    // ~~~ secondary annotation
    secondary,
};

/// Formats and writes diagnostic messages.
template <typename Input>
class diagnostic_writer
{
public:
    explicit diagnostic_writer(const Input& input, lexy::visualization_options opts = {})
    : _input(&input), _opts(opts)
    {}

    //=== writers ===//
    /// Writes a message.
    ///
    /// error: A message.
    template <typename OutputIt, typename Writer>
    OutputIt write_message(OutputIt out, diagnostic_kind kind, const Writer& message) const
    {
        using namespace lexy::_detail;
        using lexy::_detail::color; // clang-cl bug

        switch (kind)
        {
        case diagnostic_kind::error:
            out = write_color<color::red, color::bold>(out, _opts);
            out = write_str(out, "error: ");
            break;
        case diagnostic_kind::warning:
            out = write_color<color::yellow, color::bold>(out, _opts);
            out = write_str(out, " warn: ");
            break;
        case diagnostic_kind::note:
            out = write_color<color::bold>(out, _opts);
            out = write_str(out, " note: ");
            break;
        case diagnostic_kind::info:
            out = write_color<color::bold>(out, _opts);
            out = write_str(out, " info: ");
            break;
        case diagnostic_kind::debug:
            out = write_color<color::bold>(out, _opts);
            out = write_str(out, "debug: ");
            break;
        case diagnostic_kind::fixit:
            out = write_color<color::bold>(out, _opts);
            out = write_str(out, "fixit: ");
            break;
        case diagnostic_kind::help:
            out = write_color<color::bold>(out, _opts);
            out = write_str(out, " help: ");
            break;
        }
        out = write_color<color::reset>(out, _opts);

        out    = message(out, _opts);
        *out++ = '\n';

        return out;
    }

    /// Writes a path.
    template <typename OutputIt>
    OutputIt write_path(OutputIt out, const char* path) const
    {
        using namespace lexy::_detail;
        using lexy::_detail::color; // clang-cl bug

        out    = write_color<color::blue>(out, _opts);
        out    = write_str(out, path);
        out    = write_color<color::reset>(out, _opts);
        *out++ = '\n';
        return out;
    }

    /// Writes an empty annotation.
    ///
    ///    |\n
    template <typename OutputIt>
    OutputIt write_empty_annotation(OutputIt out) const
    {
        using namespace lexy::_detail;
        using lexy::_detail::color; // clang-cl bug

        out    = write_str(out, "     ");
        out    = write_str(out, column());
        *out++ = '\n';
        return out;
    }

    /// Writes a highlighted line with an annotation.
    ///
    /// 100 | void foo();
    ///     |      ^^^ annotation
    template <typename OutputIt, typename Location, typename IteratorOrSize, typename Writer>
    OutputIt write_annotation(OutputIt out, annotation_kind kind, const Location& begin_location,
                              IteratorOrSize end, const Writer& message) const
    {
        using namespace lexy::_detail;
        using lexy::_detail::color; // clang-cl bug

        auto line = lexy::get_input_line_annotation(*_input, begin_location, end);
        // If we had to truncate the annotation but don't include the newline,
        // this is a "multiline" annotation in the last line.
        auto annotate_eof = line.truncated_multiline && !line.annotated_newline;

        //=== Line with file contents ===//
        // Location column.
        out    = write_color<color::blue>(out, _opts);
        out    = write_format(out, "%4zd ", begin_location.line_nr());
        out    = write_color<color::reset>(out, _opts);
        out    = write_str(out, column());
        *out++ = ' ';

        // Print before underlined normally.
        out = lexy::visualize_to(out, line.before, _opts);

        // Print underlined colored.
        out = colorize_underline(out, kind);
        out = lexy::visualize_to(out, line.annotated, _opts.reset(lexy::visualize_use_color));
        out = write_color<color::reset>(out, _opts);

        // Print after underlined normally.
        out    = lexy::visualize_to(out, line.after, _opts);
        *out++ = '\n';

        //=== Line with annotation ===//
        // Initial column.
        out    = write_str(out, "     ");
        out    = write_str(out, column());
        *out++ = ' ';

        // Indent until the underline.
        auto indent_count = lexy::visualization_display_width(line.before, _opts);
        for (auto i = 0u; i != indent_count; ++i)
            *out++ = ' ';

        // Colorize.
        out = colorize_underline(out, kind);

        // Then underline.
        auto underline_count = lexy::visualization_display_width(line.annotated, _opts);
        for (auto i = 0u; i != underline_count; ++i)
            out = write_str(out, underline(kind));
        if (underline_count == 0 || annotate_eof)
            out = write_str(out, underline(kind));
        *out++ = ' ';

        // Print the message.
        out    = message(out, _opts.reset(lexy::visualize_use_color));
        *out++ = '\n';

        return write_color<color::reset>(out, _opts);
    }

private:
    const auto* column() const
    {
        if (_opts.is_set(lexy::visualize_use_unicode))
            return u8"│";
        else
            return u8"|";
    }

    template <typename OutputIt>
    OutputIt colorize_underline(OutputIt out, annotation_kind kind) const
    {
        using namespace lexy::_detail;
        using lexy::_detail::color; // clang-cl bug

        switch (kind)
        {
        case annotation_kind::primary:
            return write_color<color::red, color::bold>(out, _opts);
        case annotation_kind::secondary:
            return write_color<color::yellow>(out, _opts);
        }

        return out;
    }

    const auto* underline(annotation_kind kind) const
    {
        switch (kind)
        {
        case annotation_kind::primary:
            return "^";
        case annotation_kind::secondary:
            return "~";
        }

        return "";
    }

    const Input*                _input;
    lexy::visualization_options _opts;
};
} // namespace lexy_ext

namespace lexy_ext::_detail
{
template <typename OutputIt, typename Input, typename Reader, typename Tag>
OutputIt write_error(OutputIt out, const lexy::error_context<Input>& context,
                     const lexy::error<Reader, Tag>& error, lexy::visualization_options opts,
                     const char* path)
{
    diagnostic_writer<Input> writer(context.input(), opts);

    // Convert the context location and error location into line/column information.
    auto context_location = lexy::get_input_location(context.input(), context.position());
    auto location
        = lexy::get_input_location(context.input(), error.position(), context_location.anchor());

    // Write the main error headline.
    out = writer.write_message(out, diagnostic_kind::error,
                               [&](OutputIt out, lexy::visualization_options) {
                                   out = lexy::_detail::write_str(out, "while parsing ");
                                   out = lexy::_detail::write_str(out, context.production());
                                   return out;
                               });
    if (path != nullptr)
        out = writer.write_path(out, path);
    out = writer.write_empty_annotation(out);

    // Write an annotation for the context.
    if (location.line_nr() != context_location.line_nr())
    {
        out = writer.write_annotation(out, annotation_kind::secondary, context_location,
                                      lexy::_detail::next(context.position()),
                                      [&](OutputIt out, lexy::visualization_options) {
                                          return lexy::_detail::write_str(out, "beginning here");
                                      });
        out = writer.write_empty_annotation(out);
    }

    // Write the main annotation.
    if constexpr (std::is_same_v<Tag, lexy::expected_literal>)
    {
        auto string = lexy::_detail::make_literal_lexeme<typename Reader::encoding>(error.string(),
                                                                                    error.length());

        out = writer.write_annotation(out, annotation_kind::primary, location, error.index() + 1,
                                      [&](OutputIt out, lexy::visualization_options opts) {
                                          out = lexy::_detail::write_str(out, "expected '");
                                          out = lexy::visualize_to(out, string, opts);
                                          out = lexy::_detail::write_str(out, "'");
                                          return out;
                                      });
    }
    else if constexpr (std::is_same_v<Tag, lexy::expected_keyword>)
    {
        auto string = lexy::_detail::make_literal_lexeme<typename Reader::encoding>(error.string(),
                                                                                    error.length());

        out = writer.write_annotation(out, annotation_kind::primary, location, error.end(),
                                      [&](OutputIt out, lexy::visualization_options opts) {
                                          out = lexy::_detail::write_str(out, "expected keyword '");
                                          out = lexy::visualize_to(out, string, opts);
                                          out = lexy::_detail::write_str(out, "'");
                                          return out;
                                      });
    }
    else if constexpr (std::is_same_v<Tag, lexy::expected_char_class>)
    {
        out = writer.write_annotation(out, annotation_kind::primary, location, 1u,
                                      [&](OutputIt out, lexy::visualization_options) {
                                          out = lexy::_detail::write_str(out, "expected ");
                                          out = lexy::_detail::write_str(out, error.name());
                                          return out;
                                      });
    }
    else
    {
        out = writer.write_annotation(out, annotation_kind::primary, location, error.end(),
                                      [&](OutputIt out, lexy::visualization_options) {
                                          return lexy::_detail::write_str(out, error.message());
                                      });
    }

    return out;
}
} // namespace lexy_ext::_detail

namespace lexy_ext
{
template <typename OutputIterator>
struct _report_error
{
    OutputIterator              _iter;
    lexy::visualization_options _opts;
    const char*                 _path;

    struct _sink
    {
        OutputIterator              _iter;
        lexy::visualization_options _opts;
        const char*                 _path;
        std::size_t                 _count;

        using return_type = std::size_t;

        template <typename Input, typename Reader, typename Tag>
        void operator()(const lexy::error_context<Input>& context,
                        const lexy::error<Reader, Tag>&   error)
        {
            _iter = _detail::write_error(_iter, context, error, _opts, _path);
            ++_count;
        }

        std::size_t finish() &&
        {
            if (_count != 0)
                *_iter++ = '\n';
            return _count;
        }
    };
    constexpr auto sink() const
    {
        return _sink{_iter, _opts, _path, 0};
    }

    /// Specifies a path that will be printed alongside the diagnostic.
    constexpr _report_error path(const char* path) const
    {
        return {_iter, _opts, path};
    }

    /// Specifies an output iterator where the errors are written to.
    template <typename OI>
    constexpr _report_error<OI> to(OI out) const
    {
        return {out, _opts, _path};
    }

    /// Overrides visualization options.
    constexpr _report_error opts(lexy::visualization_options opts) const
    {
        return {_iter, opts, _path};
    }
};

/// An error callback that uses diagnostic_writer to print to stderr (by default).
constexpr auto report_error = _report_error<lexy::stderr_output_iterator>{};
} // namespace lexy_ext

#endif // LEXY_EXT_REPORT_ERROR_HPP_INCLUDED

