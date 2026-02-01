// Copyright (C) 2020-2024 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_DETAIL_BUFFER_BUILDER_HPP_INCLUDED
#define LEXY_DETAIL_BUFFER_BUILDER_HPP_INCLUDED

#include <cstring>
#include <lexy/_detail/assert.hpp>
#include <lexy/_detail/config.hpp>
#include <lexy/_detail/iterator.hpp>
#include <new>

namespace lexy::_detail
{
// Builds a buffer: it has a read are and a write area.
// The characters in the read area are already valid and can be read.
// The characters in the write area are not valid, but can be written too.
template <typename T>
class buffer_builder
{
    static_assert(std::is_trivial_v<T>);

    static constexpr std::size_t total_size_bytes = 1024;
    static constexpr std::size_t stack_buffer_size
        = (total_size_bytes - 3 * sizeof(T*)) / sizeof(T);
    static constexpr auto growth_factor = 2;

public:
    buffer_builder() noexcept : _data(_stack_buffer), _read_size(0), _write_size(stack_buffer_size)
    {
        static_assert(sizeof(*this) == total_size_bytes, "invalid buffer size calculation");
    }

    ~buffer_builder() noexcept
    {
        // Free memory if we allocated any.
        if (_data != _stack_buffer)
            ::operator delete(_data);
    }

    buffer_builder(const buffer_builder&)            = delete;
    buffer_builder& operator=(const buffer_builder&) = delete;

    // The total capacity: read + write.
    std::size_t capacity() const noexcept
    {
        return _read_size + _write_size;
    }

    // The read area.
    const T* read_data() const noexcept
    {
        return _data;
    }
    std::size_t read_size() const noexcept
    {
        return _read_size;
    }

    // The write area.
    T* write_data() noexcept
    {
        return _data + _read_size;
    }
    std::size_t write_size() const noexcept
    {
        return _write_size;
    }

    // Clears the read area.
    void clear() noexcept
    {
        _write_size += _read_size;
        _read_size = 0;
    }

    // Takes the first n characters of the write area and appends them to the read area.
    void commit(std::size_t n) noexcept
    {
        LEXY_PRECONDITION(n <= _write_size);
        _read_size += n;
        _write_size -= n;
    }

    // Increases the write area, invalidates all pointers.
    void grow()
    {
        const auto cur_cap = capacity();
        const auto new_cap = growth_factor * cur_cap;

        // Allocate new memory.
        auto memory = static_cast<T*>(::operator new(new_cap * sizeof(T)));
        // Copy the read area into the new memory.
        std::memcpy(memory, _data, _read_size);

        // Release the old memory, if there was any.
        if (_data != _stack_buffer)
            ::operator delete(_data);

        // Update for the new area.
        _data = memory;
        // _read_size hasn't been changed
        _write_size = new_cap - _read_size;
    }

    //=== iterator ===//
    // Stable iterator over the memory.
    class stable_iterator : public forward_iterator_base<stable_iterator, const T>
    {
    public:
        constexpr stable_iterator() = default;

        explicit constexpr stable_iterator(const _detail::buffer_builder<T>& buffer,
                                           std::size_t                       idx) noexcept
        : _buffer(&buffer), _idx(idx)
        {}

        constexpr const T& deref() const noexcept
        {
            LEXY_PRECONDITION(_idx != _buffer->read_size());
            return _buffer->read_data()[_idx];
        }

        constexpr void increment() noexcept
        {
            LEXY_PRECONDITION(_idx != _buffer->read_size());
            ++_idx;
        }

        constexpr bool equal(stable_iterator rhs) const noexcept
        {
            if (!_buffer || !rhs._buffer)
                return !_buffer && !rhs._buffer;
            else
            {
                LEXY_PRECONDITION(_buffer == rhs._buffer);
                return _idx == rhs._idx;
            }
        }

        constexpr std::size_t index() const noexcept
        {
            return _idx;
        }

    private:
        const _detail::buffer_builder<T>* _buffer = nullptr;
        std::size_t                       _idx    = 0;
    };

private:
    T*          _data;
    std::size_t _read_size;
    std::size_t _write_size;
    T           _stack_buffer[stack_buffer_size];
};
} // namespace lexy::_detail

#endif // LEXY_DETAIL_BUFFER_BUILDER_HPP_INCLUDED

