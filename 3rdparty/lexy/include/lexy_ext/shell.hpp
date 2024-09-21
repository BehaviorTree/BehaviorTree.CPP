// Copyright (C) 2020-2024 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_EXT_SHELL_HPP_INCLUDED
#define LEXY_EXT_SHELL_HPP_INCLUDED

#include <cstdio>

#include <lexy/_detail/assert.hpp>
#include <lexy/_detail/buffer_builder.hpp>
#include <lexy/error.hpp>
#include <lexy/input/base.hpp>
#include <lexy/lexeme.hpp>

namespace lexy_ext
{
#if 0
/// Controls how the shell performs I/O.
class Prompt
{
    using encoding = ...;

    /// Called to display the primary prompt.
    void primary_prompt();

    /// Called to display the continuation prompt.
    void continuation_prompt();

    /// Called to display EOF.
    void eof_prompt();

    /// Whether or not the user has closed the input.
    bool is_open() const;

    struct read_line_callback
    {
        /// Reads at most `size` characters into the `buffer` until and including a newline.
        /// Returns the number of characters read.
        /// If the number of characters read is less than the size,
        /// the entire line has been read or a read error occurs.
        std::size_t operator()(char_type* buffer, std::size_t size);

        /// Called after the shell has finished reading.
        void done() &&;
    };

    /// Returns a callback object for reading the next line.
    auto read_line()
    {
        return read_line_callback{...};
    }

    struct write_message_callback
    {
        /// Writes the buffer.
        void operator()(const char_type* buffer, std::size_t size);

        /// Called to finish writing.
        void done() &&;
    };

    /// Writes a message out.
    /// The arguments are passed by the user to indicate kinds of messages.
    auto write_message(Args&&... config_args)
    {
        return write_message_callback{...};
    }
};
#endif

/// Prompt using stdin and stdout.
template <typename Encoding = lexy::default_encoding>
struct default_prompt
{
    using encoding  = Encoding;
    using char_type = typename Encoding::char_type;
    static_assert(sizeof(char_type) == sizeof(char), "only support single-byte encodings");

    void primary_prompt() noexcept
    {
        std::fputs("> ", stdout);
    }

    void continuation_prompt() noexcept
    {
        std::fputs(". ", stdout);
    }

    void eof_prompt() noexcept
    {
        // We write an additional newline to prevent output on the same line.
        std::fputs("\n", stdout);
    }

    bool is_open() const noexcept
    {
        return std::feof(stdin) == 0 && std::ferror(stdin) == 0;
    }

    struct read_line_callback
    {
        std::size_t operator()(char_type* buffer, std::size_t size)
        {
            LEXY_PRECONDITION(size > 1);

            auto memory = reinterpret_cast<char*>(buffer);
            if (auto str = std::fgets(memory, int(size), stdin))
                return std::strlen(str);
            else
                return 0;
        }

        void done() && {}
    };
    auto read_line()
    {
        return read_line_callback{};
    }

    struct write_message_callback
    {
        bool _last_was_newline = true; // If we printed nothing, it's a newline.

        void operator()(const char_type* buffer, std::size_t size)
        {
            std::fprintf(stdout, "%.*s", int(size), reinterpret_cast<const char*>(buffer));
            if (size > 0)
                _last_was_newline = buffer[size - 1] == '\n';
        }

        void done() &&
        {
            if (!_last_was_newline)
                std::putchar('\n');
        }
    };
    auto write_message()
    {
        return write_message_callback{};
    }
};
} // namespace lexy_ext

namespace lexy_ext
{
/// Reads input from an interactive shell.
template <typename Prompt = default_prompt<>>
class shell
{
public:
    using encoding    = typename Prompt::encoding;
    using char_type   = typename encoding::char_type;
    using prompt_type = Prompt;

    shell() = default;
    explicit shell(Prompt prompt) : _prompt(LEXY_MOV(prompt)) {}

    /// Whether or not the shell is still open.
    bool is_open() const noexcept
    {
        return _prompt.is_open();
    }

    /// This is both Reader and Input.
    class input
    {
    public:
        using encoding = typename Prompt::encoding;
        using iterator = typename lexy::_detail::buffer_builder<char_type>::stable_iterator;

        struct marker
        {
            iterator _it;

            constexpr iterator position() const noexcept
            {
                return _it;
            }
        };

        auto reader() const&
        {
            return *this;
        }

        auto peek() const
        {
            if (is_eof())
                return encoding::eof();
            else
                return encoding::to_int_type(_shell->_buffer.read_data()[_idx]);
        }

        void bump() noexcept
        {
            ++_idx;
        }

        auto position() const noexcept
        {
            return iterator(_shell->_buffer, _idx);
        }

        marker current() const noexcept
        {
            return {position()};
        }
        void reset(marker m) noexcept
        {
            _idx = m._it.index();
        }

    private:
        explicit input(shell* s) : _shell(s), _idx(0)
        {
            _shell->_buffer.clear();
            _shell->_prompt.primary_prompt();
            if (!_shell->append_next_line())
                _shell->_prompt.eof_prompt();
        }

        bool is_eof() const
        {
            if (_idx != _shell->_buffer.read_size())
                // We're still having characters in the read buffer.
                return false;
            else if (!_shell->_prompt.is_open())
                // The prompt has been closed by the user.
                return true;
            else
            {
                // We've reached the end of the buffer, but the user might be willing to type
                // another line.
                _shell->_prompt.continuation_prompt();
                auto did_append = _shell->append_next_line();
                if (!did_append)
                    _shell->_prompt.eof_prompt();
                return !did_append;
            }
        }

        shell*      _shell;
        std::size_t _idx;

        friend shell;
    };

    /// Asks the user to enter input.
    /// This will invalidate the previous buffer.
    /// Returns an Input for that input.
    auto prompt_for_input()
    {
        return input(this);
    }

    class writer
    {
    public:
        writer(const writer&)            = delete;
        writer& operator=(const writer&) = delete;

        ~writer() noexcept
        {
            LEXY_MOV(_writer).done();
        }

        auto output_iterator()
        {
            struct iterator
            {
                typename Prompt::write_message_callback* _writer;

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
                    auto chr = static_cast<char_type>(c);
                    // clang-cl requires this->.
                    (*this->_writer)(&chr, 1);
                    return *this;
                }
            };

            return iterator{&_writer};
        }

        writer& operator()(const char_type* str, std::size_t length)
        {
            _writer(str, length);
            return *this;
        }
        writer& operator()(const char_type* str)
        {
            auto length = std::size_t(0);
            for (auto ptr = str; *ptr; ++ptr)
                ++length;
            _writer(str, length);
            return *this;
        }
        writer& operator()(char_type c)
        {
            _writer(&c, 1);
            return *this;
        }

        template <typename CharT,
                  typename = std::enable_if_t<encoding::template is_secondary_char_type<CharT>()>>
        writer& operator()(const CharT* str, std::size_t length)
        {
            return operator()(reinterpret_cast<const char_type*>(str), length);
        }
        template <typename CharT,
                  typename = std::enable_if_t<encoding::template is_secondary_char_type<CharT>()>>
        writer& operator()(const CharT* str)
        {
            return operator()(reinterpret_cast<const char_type*>(str));
        }
        template <typename CharT,
                  typename = std::enable_if_t<encoding::template is_secondary_char_type<CharT>()>>
        writer& operator()(CharT c)
        {
            return operator()(char_type(c));
        }

        writer& operator()(lexy::lexeme_for<input> lexeme)
        {
            // We know that the iterator is contiguous.
            auto data = &*lexeme.begin();
            _writer(data, lexeme.size());
            return *this;
        }

    private:
        explicit writer(typename Prompt::write_message_callback&& writer)
        : _writer(LEXY_MOV(writer))
        {}

        LEXY_EMPTY_MEMBER typename Prompt::write_message_callback _writer;

        friend shell;
    };

    /// Constructs a writer for writing to the prompt.
    /// The arguments depend on the Prompt in use.
    template <typename... Args>
    auto write_message(Args&&... args)
    {
        return writer{_prompt.write_message(LEXY_FWD(args)...)};
    }

    Prompt& get_prompt() noexcept
    {
        return _prompt;
    }
    const Prompt& get_prompt() const noexcept
    {
        return _prompt;
    }

private:
    // Returns whether or not we've read anything.
    bool append_next_line()
    {
        // Grwo buffer if necessary.
        constexpr auto min_capacity = 128;
        if (_buffer.write_size() < min_capacity)
            _buffer.grow();

        for (auto reader = _prompt.read_line(); true;)
        {
            const auto buffer_size = _buffer.write_size();

            // Read into the entire write area of the buffer from the file,
            // commiting what we've just read.
            const auto read = reader(_buffer.write_data(), buffer_size);
            _buffer.commit(read);

            // Check whether we've read the entire line.
            if (_buffer.write_data()[-1] == '\n')
            {
                LEXY_MOV(reader).done();
                return true;
            }
            else if (read < buffer_size)
            {
                LEXY_ASSERT(!_prompt.is_open(), "read error but prompt still open?!");
                return false;
            }

            // We've filled the entire buffer and need more space.
            // This grow might be unnecessary if we're just so happen to reach the newline with the
            // next character, but checking this requires reading more input.
            _buffer.grow();
        }

        return false;
    }

    lexy::_detail::buffer_builder<char_type> _buffer;
    LEXY_EMPTY_MEMBER Prompt                 _prompt;
};

//=== convenience typedefs ===//
template <typename Prompt = default_prompt<>>
using shell_lexeme = lexy::lexeme_for<shell<Prompt>>;

template <typename Tag, typename Prompt = default_prompt<>>
using shell_error = lexy::error_for<shell<Prompt>, Tag>;

template <typename Prompt = default_prompt<>>
using shell_error_context = lexy::error_context<shell<Prompt>>;
} // namespace lexy_ext

#endif // LEXY_EXT_SHELL_HPP_INCLUDED

