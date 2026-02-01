// Copyright (C) 2020-2024 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#include <cstddef>
#include <lexy/input/file.hpp>

#include <cerrno>
#include <cstdio>
#include <lexy/_detail/buffer_builder.hpp>

#if defined(__unix__) || defined(__APPLE__)

#    include <fcntl.h>
#    include <sys/mman.h>
#    include <unistd.h>

namespace
{
class raii_fd
{
public:
    explicit raii_fd(int file) noexcept : _file(file) {}

    raii_fd(const raii_fd&)            = delete;
    raii_fd& operator=(const raii_fd&) = delete;

    ~raii_fd() noexcept
    {
        if (_file >= 0)
            ::close(_file);
    }

    operator int() const noexcept
    {
        return _file;
    }

private:
    int _file;
};

lexy::file_error get_file_error() noexcept
{
    switch (errno)
    {
    case ENOENT:
    case ENOTDIR:
    case ELOOP:
        return lexy::file_error::file_not_found;

    case EACCES:
    case EPERM:
        return lexy::file_error::permission_denied;

    default:
        return lexy::file_error::os_error;
    }
}

constexpr std::size_t small_file_size  = std::size_t(4) * 1024;
constexpr std::size_t medium_file_size = std::size_t(32) * 1024;
} // namespace

lexy::file_error lexy::_detail::read_file(const char* path, file_callback cb, void* user_data)
{
    raii_fd fd(::open(path, O_RDONLY));
    if (fd < 0)
        return get_file_error();

    auto off = ::lseek(fd, 0, SEEK_END);
    if (off == static_cast<::off_t>(-1))
        return lexy::file_error::os_error;
    auto size = static_cast<std::size_t>(off);

    if (size <= small_file_size)
    {
        if (::lseek(fd, 0, SEEK_SET) != 0)
            return lexy::file_error::os_error;

        char buffer[small_file_size]; // Don't initialize.
        if (::read(fd, buffer, size) != static_cast<::ssize_t>(size))
            return lexy::file_error::os_error;

        cb(user_data, buffer, size);
    }
    else if (size <= medium_file_size)
    {
        if (::lseek(fd, 0, SEEK_SET) != 0)
            return lexy::file_error::os_error;

        lexy::buffer<>::builder builder(size);
        if (::read(fd, builder.data(), builder.size()) != static_cast<::ssize_t>(size))
            return lexy::file_error::os_error;

        cb(user_data, builder.data(), builder.size());
    }
    else
    {
        auto memory = ::mmap(nullptr, size, PROT_READ, MAP_PRIVATE, fd, 0);
        if (memory == MAP_FAILED) // NOLINT: int-to-ptr conversion happens in header
            return lexy::file_error::os_error;

        cb(user_data, reinterpret_cast<const char*>(memory), size);

        ::munmap(memory, size);
    }

    return lexy::file_error::_success;
}

#else // portable read_file() using C I/O

namespace
{
class raii_file
{
public:
    explicit raii_file(std::FILE* file) noexcept : _file(file) {}

    raii_file(const raii_file&)            = delete;
    raii_file& operator=(const raii_file&) = delete;

    ~raii_file() noexcept
    {
        if (_file)
            std::fclose(_file);
    }

    operator std::FILE*() const noexcept
    {
        return _file;
    }

private:
    std::FILE* _file;
};

lexy::file_error get_file_error() noexcept
{
    switch (errno)
    {
    case ENOENT:
    case ENOTDIR:
    case ELOOP:
        return lexy::file_error::file_not_found;

    case EACCES:
    case EPERM:
        return lexy::file_error::permission_denied;

    default:
        return lexy::file_error::os_error;
    }
}
} // namespace

lexy::file_error lexy::_detail::read_file(const char* path, file_callback cb, void* user_data)
{
    // Open file.
    raii_file file(std::fopen(path, "rb"));
    if (!file)
        return get_file_error();

    // Determine correct file size.
    if (std::fseek(file, 0, SEEK_END) != 0)
        return lexy::file_error::os_error;

    auto size = std::ftell(file);
    if (size == -1)
        return lexy::file_error::os_error;

    if (std::fseek(file, 0, SEEK_SET) != 0)
        return lexy::file_error::os_error;

    // Now read the entire file into a  buffer.
    lexy::buffer<>::builder builder{std::size_t(size)};
    if (std::fread(builder.data(), sizeof(char), builder.size(), file) != builder.size())
        return lexy::file_error::os_error;
    LEXY_ASSERT(std::fgetc(file) == EOF, "we haven't read everything?!");

    // Pass to callback.
    // Note that this isn't ideal, as we're having one unnecessary copy plus allocation.
    cb(user_data, builder.data(), builder.size());
    return file_error::_success;
}

#endif

// When reading from stdin, performance doesn't really matter.
// As such, we use the simple portable way of the C I/O routines.
lexy::file_error lexy::_detail::read_stdin(file_callback cb, void* user_data)
{
    // We can't use ftell() to get file size
    // So instead use a conservative loop.
    lexy::_detail::buffer_builder<char> builder;
    while (true)
    {
        const auto buffer_size = builder.write_size();
        LEXY_ASSERT(buffer_size > 0, "buffer empty?!");

        // Read into the entire write area of the buffer from stdin,
        // commiting what we've just read.
        const auto read = std::fread(builder.write_data(), sizeof(char), buffer_size, stdin);
        builder.commit(read);

        // Check whether we have exhausted the file.
        if (read < buffer_size)
        {
            if (std::ferror(stdin) != 0)
                // We have a read error.
                return lexy::file_error::os_error;

            // We should have reached the end.
            LEXY_ASSERT(std::feof(stdin), "why did fread() not read enough?");
            break;
        }

        // We've filled the entire buffer and need more space.
        // This grow might be unnecessary if we're just so happen to reach EOF with the next
        // input, but checking this requires reading more input.
        builder.grow();
    }

    // Pass final buffer to callback.
    cb(user_data, builder.read_data(), builder.read_size());
    return file_error::_success;
}

