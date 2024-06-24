// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_EXT_REPORT_ERROR_HPP_INCLUDED
#define LEXY_EXT_REPORT_ERROR_HPP_INCLUDED

#include <cstdio>
#include <lexy/_detail/assert.hpp>
#include <lexy/error.hpp>
#include <lexy/input_location.hpp>
#include <lexy/visualize.hpp>

namespace lexy_ext::_detail
{
enum class annotation_kind
{
    primary,
    secondary,
};

template <typename Input>
struct error_writer
{
    const Input*                input;
    lexy::visualization_options opts;

    const auto* column() const
    {
        if (opts.is_set(lexy::visualize_use_unicode))
            return u8"│";
        else
            return u8"|";
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

    //=== writers ===//
    template <typename OutputIt, typename Writer>
    OutputIt write_message(OutputIt out, const Writer& message)
    {
        using namespace lexy::_detail;
        using lexy::_detail::color; // clang-cl bug

        out    = write_color<color::red, color::bold>(out, opts);
        out    = write_str(out, "error: ");
        out    = write_color<color::reset>(out, opts);
        out    = message(out, opts);
        *out++ = '\n';

        return out;
    }

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

    template <typename OutputIt, typename Location, typename IteratorOrSize, typename Writer>
    OutputIt write_annotation(OutputIt out, annotation_kind kind, const Location& begin_location,
                              IteratorOrSize end, const Writer& message) const
    {
        using namespace lexy::_detail;
        using lexy::_detail::color; // clang-cl bug

        auto colorize_underline = [&](OutputIt out) {
            switch (kind)
            {
            case annotation_kind::primary:
                return write_color<color::red, color::bold>(out, opts);
            case annotation_kind::secondary:
                return write_color<color::yellow>(out, opts);
            }

            return out;
        };

        auto line = lexy::get_input_line_annotation(*input, begin_location, end);
        // If we had to truncate the annotation but don't include the newline,
        // this is a "multiline" annotation in the last line.
        auto annotate_eof = line.truncated_multiline && !line.annotated_newline;

        //=== Line with file contents ===//
        // Location column.
        out    = write_format(out, "%4zd ", begin_location.line_nr());
        out    = write_str(out, column());
        *out++ = ' ';

        // Print before underlined normally.
        out = lexy::visualize_to(out, line.before, opts);

        // Print underlined colored.
        out = colorize_underline(out);
        out = lexy::visualize_to(out, line.annotated, opts.reset(lexy::visualize_use_color));
        out = write_color<color::reset>(out, opts);

        // Print after underlined normally.
        out    = lexy::visualize_to(out, line.after, opts);
        *out++ = '\n';

        //=== Line with annotation ===//
        // Initial column.
        out    = write_str(out, "     ");
        out    = write_str(out, column());
        *out++ = ' ';

        // Indent until the underline.
        auto indent_count = lexy::visualization_display_width(line.before, opts);
        for (auto i = 0u; i != indent_count; ++i)
            *out++ = ' ';

        // Colorize.
        out = colorize_underline(out);

        // Then underline.
        auto underline_count = lexy::visualization_display_width(line.annotated, opts);
        for (auto i = 0u; i != underline_count; ++i)
            out = write_str(out, underline(kind));
        if (underline_count == 0 || annotate_eof)
            out = write_str(out, underline(kind));
        *out++ = ' ';

        // Print the message.
        out    = message(out, opts.reset(lexy::visualize_use_color));
        *out++ = '\n';

        return write_color<color::reset>(out, opts);
    }
};
} // namespace lexy_ext::_detail

namespace lexy_ext::_detail
{
template <typename OutputIt, typename Production, typename Input, typename Reader, typename Tag>
OutputIt write_error(OutputIt out, const lexy::error_context<Production, Input>& context,
                     const lexy::error<Reader, Tag>& error, lexy::visualization_options opts)
{
    _detail::error_writer<Input> writer{&context.input(), opts};

    // Convert the context location and error location into line/column information.
    auto context_location = lexy::get_input_location(context.input(), context.position());
    auto location
        = lexy::get_input_location(context.input(), error.position(), context_location.anchor());

    // Write the main error headline.
    out = writer.write_message(out, [&](OutputIt out, lexy::visualization_options) {
        out = lexy::_detail::write_str(out, "while parsing ");
        out = lexy::_detail::write_str(out, context.production());
        return out;
    });
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
struct _report_error
{
    struct _sink
    {
        std::size_t _count;

        using return_type = std::size_t;

        template <typename Production, typename Input, typename Reader, typename Tag>
        void operator()(const lexy::error_context<Production, Input>& context,
                        const lexy::error<Reader, Tag>&               error)
        {
            _detail::write_error(lexy::cfile_output_iterator{stderr}, context, error,
                                 {lexy::visualize_fancy});
            ++_count;
        }

        std::size_t finish() &&
        {
            if (_count != 0)
                std::fputs("\n", stderr);
            return _count;
        }
    };

    constexpr auto sink() const
    {
        return _sink{};
    }
};

// The error callback that prints to stderr.
constexpr auto report_error = _report_error{};
} // namespace lexy_ext

#endif // LEXY_EXT_REPORT_ERROR_HPP_INCLUDED

