// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_INPUT_BUFFER_HPP_INCLUDED
#define LEXY_INPUT_BUFFER_HPP_INCLUDED

#include <cstring>
#include <lexy/_detail/memory_resource.hpp>
#include <lexy/error.hpp>
#include <lexy/input/base.hpp>
#include <lexy/lexeme.hpp>

namespace lexy
{
// The reader used by the buffer if it can use a sentinel.
template <typename Encoding>
class _br
{
public:
    using encoding = Encoding;
    using iterator = const typename Encoding::char_type*;

    explicit _br(iterator begin) noexcept : _cur(begin) {}

    auto peek() const noexcept
    {
        // The last one will be EOF.
        return *_cur;
    }

    void bump() noexcept
    {
        ++_cur;
    }

    iterator position() const noexcept
    {
        return _cur;
    }

    void set_position(iterator new_pos) noexcept
    {
        _cur = new_pos;
    }

private:
    iterator _cur;
};

// We use aliases for the three encodings that can actually use it.
// (i.e. where char_type == int_type).
LEXY_INSTANTIATION_NEWTYPE(_bra, _br, lexy::ascii_encoding);
LEXY_INSTANTIATION_NEWTYPE(_br8, _br, lexy::utf8_encoding);
LEXY_INSTANTIATION_NEWTYPE(_br32, _br, lexy::utf32_encoding);

// Create the appropriate buffer reader.
template <typename Encoding>
constexpr auto _buffer_reader(const typename Encoding::char_type* data)
{
    if constexpr (std::is_same_v<Encoding, lexy::ascii_encoding>)
        return _bra(data);
    else if constexpr (std::is_same_v<Encoding, lexy::utf8_encoding>)
        return _br8(data);
    else if constexpr (std::is_same_v<Encoding, lexy::utf32_encoding>)
        return _br32(data);
    else
        return _br<Encoding>(data);
}
} // namespace lexy

namespace lexy
{
/// Stores the input that will be parsed.
/// For encodings with spare code points, it can append an EOF sentinel.
/// This allows branch-less detection of EOF.
template <typename Encoding = default_encoding, typename MemoryResource = void>
class buffer
{
    static constexpr auto _has_sentinel
        = std::is_same_v<typename Encoding::char_type, typename Encoding::int_type>;

public:
    using encoding  = Encoding;
    using char_type = typename encoding::char_type;
    static_assert(std::is_trivial_v<char_type>);

    //=== constructors ===//
    /// Allows the creation of an uninitialized buffer that is then filled by the user.
    class builder
    {
    public:
        explicit builder(std::size_t     size,
                         MemoryResource* resource = _detail::get_memory_resource<MemoryResource>())
        : _buffer(resource)
        {
            _buffer._data = _buffer.allocate(size);
            _buffer._size = size;
        }

        char_type* data() const noexcept
        {
            return _buffer._data;
        }
        std::size_t size() const noexcept
        {
            return _buffer._size;
        }

        buffer finish() && noexcept
        {
            return LEXY_MOV(_buffer);
        }

    private:
        buffer _buffer;
    };

    constexpr buffer() noexcept : buffer(_detail::get_memory_resource<MemoryResource>()) {}

    constexpr explicit buffer(MemoryResource* resource) noexcept
    : _resource(resource), _data(nullptr), _size(0)
    {}

    explicit buffer(const char_type* data, std::size_t size,
                    MemoryResource* resource = _detail::get_memory_resource<MemoryResource>())
    : _resource(resource), _size(size)
    {
        _data = allocate(size);
        std::memcpy(_data, data, size * sizeof(char_type));
    }
    explicit buffer(const char_type* begin, const char_type* end,
                    MemoryResource* resource = _detail::get_memory_resource<MemoryResource>())
    : buffer(begin, std::size_t(end - begin), resource)
    {}

    template <typename CharT, typename = _detail::require_secondary_char_type<encoding, CharT>>
    explicit buffer(const CharT* data, std::size_t size,
                    MemoryResource* resource = _detail::get_memory_resource<MemoryResource>())
    : buffer(reinterpret_cast<const char_type*>(data), size, resource)
    {
        static_assert(sizeof(CharT) == sizeof(char_type));
    }
    template <typename CharT, typename = _detail::require_secondary_char_type<encoding, CharT>>
    explicit buffer(const CharT* begin, const CharT* end,
                    MemoryResource* resource = _detail::get_memory_resource<MemoryResource>())
    : buffer(reinterpret_cast<const char_type*>(begin), reinterpret_cast<const char_type*>(end),
             resource)
    {
        static_assert(sizeof(CharT) == sizeof(char_type));
    }

    template <typename View, typename = decltype(LEXY_DECLVAL(View).data())>
    explicit buffer(const View&     view,
                    MemoryResource* resource = _detail::get_memory_resource<MemoryResource>())
    : buffer(view.data(), view.size(), resource)
    {}

    buffer(const buffer& other) : buffer(other.data(), other.size(), other._resource.get()) {}
    buffer(const buffer& other, MemoryResource* resource)
    : buffer(other.data(), other.size(), resource)
    {}

    buffer(buffer&& other) noexcept
    : _resource(other._resource), _data(other._data), _size(other._size)
    {
        other._data = nullptr;
        other._size = 0;
    }

    ~buffer() noexcept
    {
        if (!_data)
            return;

        if constexpr (_has_sentinel)
            _resource->deallocate(_data, (_size + 1) * sizeof(char_type), alignof(char_type));
        else
            _resource->deallocate(_data, _size * sizeof(char_type), alignof(char_type));
    }

    buffer& operator=(const buffer& other) // NOLINT: we do guard against self-assignment
    {
        // Create a temporary buffer that owns the same memory as other but with our resource.
        // We then move assign it to *this.
        *this = buffer(other, _resource.get());
        return *this;
    }

    // NOLINTNEXTLINE: Unfortunately, sometimes move is not noexcept.
    buffer& operator=(buffer&& other) noexcept(std::is_empty_v<MemoryResource>)
    {
        if (*_resource == *other._resource)
        {
            // We have the same resource; we can just steal other's memory.
            // We do that by swapping - when other is destroyed it will free our memory.
            _detail::swap(_data, other._data);
            _detail::swap(_size, other._size);
            return *this;
        }
        else
        {
            LEXY_PRECONDITION(!std::is_empty_v<MemoryResource>);

            // We create a copy using the right resource and swap the ownership.
            buffer copy(other, _resource.get());
            _detail::swap(_data, copy._data);
            _detail::swap(_size, copy._size);
            return *this;
        }
    }

    //=== access ===//
    const char_type* data() const noexcept
    {
        return _data;
    }

    std::size_t size() const noexcept
    {
        return _size;
    }

    //=== input ===//
    auto reader() const& noexcept
    {
        if constexpr (_has_sentinel)
            return _buffer_reader<encoding>(_data);
        else
            return _range_reader<encoding>(_data, _data + _size);
    }

private:
    char_type* allocate(std::size_t size) const
    {
        if constexpr (_has_sentinel)
            ++size;

        auto memory = static_cast<char_type*>(
            _resource->allocate(size * sizeof(char_type), alignof(char_type)));
        if constexpr (_has_sentinel)
            memory[size - 1] = encoding::eof();
        return memory;
    }

    LEXY_EMPTY_MEMBER _detail::memory_resource_ptr<MemoryResource> _resource;
    char_type*                                                     _data;
    std::size_t                                                    _size;
};

template <typename CharT>
buffer(const CharT*, const CharT*) -> buffer<deduce_encoding<CharT>>;
template <typename CharT>
buffer(const CharT*, std::size_t) -> buffer<deduce_encoding<CharT>>;
template <typename View>
buffer(const View&) -> buffer<deduce_encoding<LEXY_DECAY_DECLTYPE(*LEXY_DECLVAL(View).data())>>;

template <typename CharT, typename MemoryResource>
buffer(const CharT*, const CharT*, MemoryResource*)
    -> buffer<deduce_encoding<CharT>, MemoryResource>;
template <typename CharT, typename MemoryResource>
buffer(const CharT*, std::size_t, MemoryResource*)
    -> buffer<deduce_encoding<CharT>, MemoryResource>;
template <typename View, typename MemoryResource>
buffer(const View&, MemoryResource*)
    -> buffer<deduce_encoding<LEXY_DECAY_DECLTYPE(*LEXY_DECLVAL(View).data())>, MemoryResource>;

//=== make_buffer ===//
template <typename Encoding, encoding_endianness Endian>
struct _make_buffer
{
    template <typename MemoryResource = void>
    auto operator()(const void* _memory, std::size_t size,
                    MemoryResource* resource = _detail::get_memory_resource<MemoryResource>()) const
    {
        constexpr auto native_endianness
            = LEXY_IS_LITTLE_ENDIAN ? encoding_endianness::little : encoding_endianness::big;

        using char_type = typename Encoding::char_type;
        LEXY_PRECONDITION(size % sizeof(char_type) == 0);
        auto memory = static_cast<const unsigned char*>(_memory);

        if constexpr (sizeof(char_type) == 1 || Endian == native_endianness)
        {
            // No need to deal with endianness at all.
            // The reinterpret_cast is technically UB, as we didn't create objects in memory,
            // but until std::start_lifetime_as is added, there is nothing we can do.
            return buffer<Encoding, MemoryResource>(reinterpret_cast<const char_type*>(memory),
                                                    size / sizeof(char_type), resource);
        }
        else
        {
            typename buffer<Encoding, MemoryResource>::builder builder(size / sizeof(char_type),
                                                                       resource);

            const auto end = memory + size;
            for (auto dest = builder.data(); memory != end; memory += sizeof(char_type))
            {
                constexpr auto is_char16 = std::is_same_v<char_type, char16_t>;
                constexpr auto is_char32 = std::is_same_v<char_type, char32_t>;

                // We convert each group of bytes to the appropriate value.
                if constexpr (is_char16 && Endian == encoding_endianness::little)
                    *dest++ = static_cast<char_type>((memory[0] << 0) | (memory[1] << 8));
                else if constexpr (is_char32 && Endian == encoding_endianness::little)
                    *dest++ = static_cast<char_type>((memory[0] << 0) | (memory[1] << 8)
                                                     | (memory[2] << 16) | (memory[3] << 24));
                else if constexpr (is_char16 && Endian == encoding_endianness::big)
                    *dest++ = static_cast<char_type>((memory[0] << 8) | (memory[1] << 0));
                else if constexpr (is_char32 && Endian == encoding_endianness::big)
                    *dest++ = static_cast<char_type>((memory[0] << 24) | (memory[1] << 16)
                                                     | (memory[2] << 8) | (memory[3] << 0));
                else
                    static_assert(_detail::error<Encoding>, "unhandled encoding/endianness");
            }

            return LEXY_MOV(builder).finish();
        }
    }
};
template <>
struct _make_buffer<utf8_encoding, encoding_endianness::bom>
{
    template <typename MemoryResource = void>
    auto operator()(const void* _memory, std::size_t size,
                    MemoryResource* resource = _detail::get_memory_resource<MemoryResource>()) const
    {
        auto memory = static_cast<const unsigned char*>(_memory);

        // We just skip over the BOM if there is one, it doesn't matter.
        if (size >= 3 && memory[0] == 0xEF && memory[1] == 0xBB && memory[2] == 0xBF)
        {
            memory += 3;
            size -= 3;
        }

        return _make_buffer<utf8_encoding, encoding_endianness::big>{}(memory, size, resource);
    }
};
template <>
struct _make_buffer<utf16_encoding, encoding_endianness::bom>
{
    template <typename MemoryResource = void>
    auto operator()(const void* _memory, std::size_t size,
                    MemoryResource* resource = _detail::get_memory_resource<MemoryResource>()) const
    {
        constexpr auto utf16_big    = _make_buffer<utf16_encoding, encoding_endianness::big>{};
        constexpr auto utf16_little = _make_buffer<utf16_encoding, encoding_endianness::little>{};
        auto           memory       = static_cast<const unsigned char*>(_memory);

        if (size < 2)
            return utf16_big(memory, size, resource);
        if (memory[0] == 0xFF && memory[1] == 0xFE)
            return utf16_little(memory + 2, size - 2, resource);
        else if (memory[0] == 0xFE && memory[1] == 0xFF)
            return utf16_big(memory + 2, size - 2, resource);
        else
            return utf16_big(memory, size, resource);
    }
};
template <>
struct _make_buffer<utf32_encoding, encoding_endianness::bom>
{
    template <typename MemoryResource = void>
    auto operator()(const void* _memory, std::size_t size,
                    MemoryResource* resource = _detail::get_memory_resource<MemoryResource>()) const
    {
        constexpr auto utf32_big    = _make_buffer<utf32_encoding, encoding_endianness::big>{};
        constexpr auto utf32_little = _make_buffer<utf32_encoding, encoding_endianness::little>{};
        auto           memory       = static_cast<const unsigned char*>(_memory);

        if (size >= 4)
        {
            if (memory[0] == 0xFF && memory[1] == 0xFE && memory[2] == 0x00 && memory[3] == 0x00)
                return utf32_little(memory + 4, size - 4, resource);
            else if (memory[0] == 0x00 && memory[1] == 0x00 && memory[2] == 0xFE && memory[3])
                return utf32_big(memory + 4, size - 4, resource);
        }

        return utf32_big(memory, size, resource);
    }
};

/// Creates a buffer with the specified encoding/endianness from raw memory.
template <typename Encoding, encoding_endianness Endianness>
constexpr auto make_buffer_from_raw = _make_buffer<Encoding, Endianness>{};

//=== convenience typedefs ===//
template <typename Encoding = default_encoding, typename MemoryResource = void>
using buffer_lexeme = lexeme_for<buffer<Encoding, MemoryResource>>;

template <typename Tag, typename Encoding = default_encoding, typename MemoryResource = void>
using buffer_error = error_for<buffer<Encoding, MemoryResource>, Tag>;

template <typename Production, typename Encoding = default_encoding, typename MemoryResource = void>
using buffer_error_context = error_context<Production, buffer<Encoding, MemoryResource>>;
} // namespace lexy

#endif // LEXY_INPUT_BUFFER_HPP_INCLUDED

