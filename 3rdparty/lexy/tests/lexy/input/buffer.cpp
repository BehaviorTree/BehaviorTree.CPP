// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#include <lexy/input/buffer.hpp>

#include <doctest/doctest.h>

#if defined(__has_include) && __has_include(<memory_resource>)
#    include <memory_resource>
#    define LEXY_HAS_RESOURCE 1
#else
#    define LEXY_HAS_RESOURCE 0
#endif

TEST_CASE("buffer")
{
    static const char str[] = {'a', 'b', 'c'};
    struct view_type
    {
        auto data() const
        {
            return str;
        }

        std::size_t size() const
        {
            return 3;
        }
    };

    auto verify = [](const auto& buffer) {
        CHECK(buffer.size() == 3);

        CHECK(buffer.data()[0] == 'a');
        CHECK(buffer.data()[1] == 'b');
        CHECK(buffer.data()[2] == 'c');
    };

    SUBCASE("constructor, default encoding, default resource")
    {
        const lexy::buffer ptr_size(str, 3);
        verify(ptr_size);

        const lexy::buffer ptr_ptr(str, str + 3);
        verify(ptr_ptr);

        const lexy::buffer view(view_type{});
        verify(view);

        if constexpr (LEXY_HAS_EMPTY_MEMBER)
            CHECK(sizeof(ptr_size) == 2 * sizeof(void*));

        lexy::buffer<>::builder builder(3);
        std::memcpy(builder.data(), str, builder.size());
        verify(LEXY_MOV(builder).finish());
    }
#if LEXY_HAS_RESOURCE
    SUBCASE("constructor, default encoding, custom resource")
    {
        const lexy::buffer ptr_size(str, 3, std::pmr::new_delete_resource());
        verify(ptr_size);

        const lexy::buffer ptr_ptr(str, str + 3, std::pmr::new_delete_resource());
        verify(ptr_ptr);

        const lexy::buffer view(view_type{}, std::pmr::new_delete_resource());
        verify(view);

        CHECK(sizeof(ptr_size) == 3 * sizeof(char*));

        decltype(ptr_size)::builder builder(3, std::pmr::new_delete_resource());
        std::memcpy(builder.data(), str, builder.size());
        verify(LEXY_MOV(builder).finish());
    }
#endif
    SUBCASE("constructor, custom encoding, default resource")
    {
        static const auto ustr = reinterpret_cast<const unsigned char*>(str);
        struct uview_type
        {
            auto data() const
            {
                return ustr;
            }

            std::size_t size() const
            {
                return 3;
            }
        };

        const lexy::buffer ptr_size(ustr, 3);
        CHECK(std::is_same_v<decltype(ptr_size)::encoding, lexy::byte_encoding>);
        verify(ptr_size);

        const lexy::buffer ptr_ptr(ustr, ustr + 3);
        CHECK(std::is_same_v<decltype(ptr_ptr)::encoding, lexy::byte_encoding>);
        verify(ptr_ptr);

        const lexy::buffer view(uview_type{});
        CHECK(std::is_same_v<decltype(view)::encoding, lexy::byte_encoding>);
        verify(view);

        const lexy::buffer<lexy::byte_encoding> ptr_size_conv(str, 3);
        verify(ptr_size_conv);

        const lexy::buffer<lexy::byte_encoding> ptr_ptr_conv(str, str + 3);
        verify(ptr_ptr);

        const lexy::buffer<lexy::byte_encoding> view_conv(view_type{});
        verify(view_conv);

        lexy::buffer<lexy::byte_encoding>::builder builder(3);
        std::memcpy(builder.data(), str, builder.size());
        verify(LEXY_MOV(builder).finish());
    }
#if LEXY_HAS_RESOURCE
    SUBCASE("constructor, custom encoding, custom resource")
    {
        static const auto ustr = reinterpret_cast<const unsigned char*>(str);
        struct uview_type
        {
            auto data() const
            {
                return ustr;
            }

            std::size_t size() const
            {
                return 3;
            }
        };

        const lexy::buffer ptr_size(ustr, 3, std::pmr::new_delete_resource());
        CHECK(std::is_same_v<decltype(ptr_size)::encoding, lexy::byte_encoding>);
        verify(ptr_size);

        const lexy::buffer ptr_ptr(ustr, ustr + 3, std::pmr::new_delete_resource());
        CHECK(std::is_same_v<decltype(ptr_ptr)::encoding, lexy::byte_encoding>);
        verify(ptr_ptr);

        const lexy::buffer view(uview_type{}, std::pmr::new_delete_resource());
        CHECK(std::is_same_v<decltype(view)::encoding, lexy::byte_encoding>);
        verify(view);

        using buffer_type = lexy::buffer<lexy::byte_encoding, std::pmr::memory_resource>;
        const buffer_type ptr_size_conv(str, 3, std::pmr::new_delete_resource());
        verify(ptr_size_conv);

        const buffer_type ptr_ptr_conv(str, str + 3, std::pmr::new_delete_resource());
        verify(ptr_ptr);

        const buffer_type view_conv(view_type{}, std::pmr::new_delete_resource());
        verify(view_conv);

        buffer_type::builder builder(3, std::pmr::new_delete_resource());
        std::memcpy(builder.data(), str, builder.size());
        verify(LEXY_MOV(builder).finish());
    }
#endif

    SUBCASE("copy constructor")
    {
        const lexy::buffer original(str, str + 3);

        const lexy::buffer copy(original); // NOLINT
        verify(copy);

        lexy::_detail::default_memory_resource other_resource;
        const lexy::buffer                     copy_resource(original, &other_resource);
        verify(copy_resource);
    }
    SUBCASE("move constructor")
    {
        lexy::buffer original(str, str + 3);

        lexy::buffer move(LEXY_MOV(original));
        verify(move);
        CHECK(original.size() == 0);
    }
    SUBCASE("copy assignment")
    {
        const lexy::buffer other(str, str + 3);

        lexy::buffer buffer{};
        CHECK(buffer.size() == 0);
        buffer = other;
        verify(buffer);
    }
    SUBCASE("move assignment")
    {
        lexy::buffer other(str, str + 3);

        lexy::buffer buffer{};
        CHECK(buffer.size() == 0);
        buffer = LEXY_MOV(other);
        verify(buffer);
        CHECK(other.size() == 0);
    }

    SUBCASE("reader, no sentinel")
    {
        const lexy::buffer buffer(str, 3);

        auto reader = buffer.reader();
        CHECK(reader.position() == buffer.data());
        CHECK(reader.peek() == 'a');

        reader.bump();
        CHECK(reader.position() == buffer.data() + 1);
        CHECK(reader.peek() == 'b');

        reader.bump();
        CHECK(reader.position() == buffer.data() + 2);
        CHECK(reader.peek() == 'c');

        reader.bump();
        CHECK(reader.position() == buffer.data() + 3);
        CHECK(reader.peek() == lexy::default_encoding::eof());
    }
    SUBCASE("reader, sentinel")
    {
        const lexy::buffer<lexy::ascii_encoding> buffer(str, 3);

        auto reader = buffer.reader();
        CHECK(reader.position() == buffer.data());
        CHECK(reader.peek() == 'a');

        reader.bump();
        CHECK(reader.position() == buffer.data() + 1);
        CHECK(reader.peek() == 'b');

        reader.bump();
        CHECK(reader.position() == buffer.data() + 2);
        CHECK(reader.peek() == 'c');

        reader.bump();
        CHECK(reader.position() == buffer.data() + 3);
        CHECK(reader.peek() == lexy::ascii_encoding::eof());
    }
}

TEST_CASE("make_buffer")
{
    const unsigned char no_bom_str[] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77};

    SUBCASE("default_encoding")
    {
        auto little
            = lexy::make_buffer_from_raw<lexy::default_encoding,
                                         lexy::encoding_endianness::little>(no_bom_str,
                                                                            sizeof(no_bom_str));
        CHECK(little.size() == 8);
        CHECK(little.data()[0] == 0x00);
        CHECK(little.data()[1] == 0x11);
        CHECK(little.data()[2] == 0x22);
        CHECK(little.data()[3] == 0x33);
        CHECK(little.data()[4] == 0x44);

        auto big = lexy::make_buffer_from_raw<lexy::default_encoding,
                                              lexy::encoding_endianness::big>(no_bom_str,
                                                                              sizeof(no_bom_str));
        CHECK(big.size() == 8);
        CHECK(big.data()[0] == 0x00);
        CHECK(big.data()[1] == 0x11);
        CHECK(big.data()[2] == 0x22);
        CHECK(big.data()[3] == 0x33);
        CHECK(big.data()[4] == 0x44);

        auto bom = lexy::make_buffer_from_raw<lexy::default_encoding,
                                              lexy::encoding_endianness::bom>(no_bom_str,
                                                                              sizeof(no_bom_str));
        CHECK(bom.size() == 8);
        CHECK(bom.data()[0] == 0x00);
        CHECK(bom.data()[1] == 0x11);
        CHECK(bom.data()[2] == 0x22);
        CHECK(bom.data()[3] == 0x33);
        CHECK(bom.data()[4] == 0x44);
    }
    SUBCASE("utf8_encoding")
    {
        auto little
            = lexy::make_buffer_from_raw<lexy::utf8_encoding,
                                         lexy::encoding_endianness::little>(no_bom_str,
                                                                            sizeof(no_bom_str));
        CHECK(little.size() == 8);
        CHECK(little.data()[0] == 0x00);
        CHECK(little.data()[1] == 0x11);
        CHECK(little.data()[2] == 0x22);
        CHECK(little.data()[3] == 0x33);
        CHECK(little.data()[4] == 0x44);

        auto big = lexy::make_buffer_from_raw<lexy::utf8_encoding,
                                              lexy::encoding_endianness::big>(no_bom_str,
                                                                              sizeof(no_bom_str));
        CHECK(big.size() == 8);
        CHECK(big.data()[0] == 0x00);
        CHECK(big.data()[1] == 0x11);
        CHECK(big.data()[2] == 0x22);
        CHECK(big.data()[3] == 0x33);
        CHECK(big.data()[4] == 0x44);

        auto no_bom
            = lexy::make_buffer_from_raw<lexy::utf8_encoding,
                                         lexy::encoding_endianness::bom>(no_bom_str,
                                                                         sizeof(no_bom_str));
        CHECK(no_bom.size() == 8);
        CHECK(no_bom.data()[0] == 0x00);
        CHECK(no_bom.data()[1] == 0x11);
        CHECK(no_bom.data()[2] == 0x22);
        CHECK(no_bom.data()[3] == 0x33);
        CHECK(no_bom.data()[4] == 0x44);

        const unsigned char bom_str[] = {0xEF, 0xBB, 0xBF, 0x00, 0x11, 0x22, 0x33, 0x44};

        auto bom
            = lexy::make_buffer_from_raw<lexy::utf8_encoding,
                                         lexy::encoding_endianness::bom>(bom_str, sizeof(bom_str));
        CHECK(bom.size() == 5);
        CHECK(bom.data()[0] == 0x00);
        CHECK(bom.data()[1] == 0x11);
        CHECK(bom.data()[2] == 0x22);
        CHECK(bom.data()[3] == 0x33);
        CHECK(bom.data()[4] == 0x44);
    }
    SUBCASE("utf16_encoding")
    {
        auto little
            = lexy::make_buffer_from_raw<lexy::utf16_encoding,
                                         lexy::encoding_endianness::little>(no_bom_str,
                                                                            sizeof(no_bom_str));
        CHECK(little.size() == 4);
        CHECK(little.data()[0] == 0x1100);
        CHECK(little.data()[1] == 0x3322);
        CHECK(little.data()[2] == 0x5544);
        CHECK(little.data()[3] == 0x7766);

        auto big = lexy::make_buffer_from_raw<lexy::utf16_encoding,
                                              lexy::encoding_endianness::big>(no_bom_str,
                                                                              sizeof(no_bom_str));
        CHECK(big.size() == 4);
        CHECK(big.data()[0] == 0x0011);
        CHECK(big.data()[1] == 0x2233);
        CHECK(big.data()[2] == 0x4455);
        CHECK(big.data()[3] == 0x6677);

        auto empty_bom = lexy::make_buffer_from_raw<lexy::utf16_encoding,
                                                    lexy::encoding_endianness::bom>(no_bom_str, 0);
        CHECK(empty_bom.size() == 0);

        auto no_bom
            = lexy::make_buffer_from_raw<lexy::utf16_encoding,
                                         lexy::encoding_endianness::bom>(no_bom_str,
                                                                         sizeof(no_bom_str));
        CHECK(no_bom.size() == 4);
        CHECK(no_bom.data()[0] == 0x0011);
        CHECK(no_bom.data()[1] == 0x2233);
        CHECK(no_bom.data()[2] == 0x4455);
        CHECK(no_bom.data()[3] == 0x6677);

        const unsigned char little_bom_str[] = {0xFF, 0xFE, 0x00, 0x11, 0x22, 0x33};
        auto                little_bom
            = lexy::make_buffer_from_raw<lexy::utf16_encoding,
                                         lexy::encoding_endianness::bom>(little_bom_str,
                                                                         sizeof(little_bom_str));
        CHECK(little_bom.size() == 2);
        CHECK(little_bom.data()[0] == 0x1100);
        CHECK(little_bom.data()[1] == 0x3322);

        const unsigned char big_bom_str[] = {0xFE, 0xFF, 0x00, 0x11, 0x22, 0x33};
        auto                big_bom
            = lexy::make_buffer_from_raw<lexy::utf16_encoding,
                                         lexy::encoding_endianness::bom>(big_bom_str,
                                                                         sizeof(big_bom_str));
        CHECK(big_bom.size() == 2);
        CHECK(big_bom.data()[0] == 0x0011);
        CHECK(big_bom.data()[1] == 0x2233);
    }
    SUBCASE("utf32_encoding")
    {
        auto little
            = lexy::make_buffer_from_raw<lexy::utf32_encoding,
                                         lexy::encoding_endianness::little>(no_bom_str,
                                                                            sizeof(no_bom_str));
        CHECK(little.size() == 2);
        CHECK(little.data()[0] == 0x33221100);
        CHECK(little.data()[1] == 0x77665544);

        auto big = lexy::make_buffer_from_raw<lexy::utf32_encoding,
                                              lexy::encoding_endianness::big>(no_bom_str,
                                                                              sizeof(no_bom_str));
        CHECK(big.size() == 2);
        CHECK(big.data()[0] == 0x00112233);
        CHECK(big.data()[1] == 0x44556677);

        auto empty_bom = lexy::make_buffer_from_raw<lexy::utf32_encoding,
                                                    lexy::encoding_endianness::bom>(no_bom_str, 0);
        CHECK(empty_bom.size() == 0);

        auto no_bom
            = lexy::make_buffer_from_raw<lexy::utf32_encoding,
                                         lexy::encoding_endianness::bom>(no_bom_str,
                                                                         sizeof(no_bom_str));
        CHECK(no_bom.size() == 2);
        CHECK(no_bom.data()[0] == 0x00112233);
        CHECK(no_bom.data()[1] == 0x44556677);

        const unsigned char little_bom_str[] = {0xFF, 0xFE, 0x00, 0x00, 0x00, 0x11, 0x22, 0x33};
        auto                little_bom
            = lexy::make_buffer_from_raw<lexy::utf32_encoding,
                                         lexy::encoding_endianness::bom>(little_bom_str,
                                                                         sizeof(little_bom_str));
        CHECK(little_bom.size() == 1);
        CHECK(little_bom.data()[0] == 0x33221100);

        const unsigned char big_bom_str[] = {0x00, 0x00, 0xFE, 0xFF, 0x00, 0x11, 0x22, 0x33};
        auto                big_bom
            = lexy::make_buffer_from_raw<lexy::utf32_encoding,
                                         lexy::encoding_endianness::bom>(big_bom_str,
                                                                         sizeof(big_bom_str));
        CHECK(big_bom.size() == 1);
        CHECK(big_bom.data()[0] == 0x00112233);
    }
}

