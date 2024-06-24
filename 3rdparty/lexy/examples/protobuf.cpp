// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

// This examples parses a protobuf message in the binary encoding.
// It then prints the basic structure of it, without knowing the message definition.

#include <cinttypes>
#include <cstdint>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

#include <lexy/action/parse.hpp> // lexy::parse
#include <lexy/callback.hpp>     // value callbacks
#include <lexy/dsl.hpp>          // lexy::dsl::*
#include <lexy/input/file.hpp>   // lexy::read_file
#include <lexy/visualize.hpp>    // lexy::visualize

#include <lexy_ext/report_error.hpp> // lexy_ext::report_error

// Stores a generic protobuf message.
namespace ast
{
// wire type 0
struct field_varint
{
    std::uint64_t value;

    void print() const
    {
        std::printf("%" PRIu64, value);
    }
};

// wire type 5
struct field_32
{
    std::uint32_t value;

    void print() const
    {
        auto as_unsigned = value;
        auto as_signed   = (value << 1) ^ (value >> 31);
        auto as_float    = lexy::bit_cast<float>(value);
        std::printf("%" PRIu32 " / %" PRIi32 " / %f", as_unsigned, as_signed, as_float);
    }
};

// wire type 2
struct field_64
{
    std::uint64_t value;

    void print() const
    {
        auto as_unsigned = value;
        auto as_signed   = (value << 1) ^ (value >> 63);
        auto as_float    = lexy::bit_cast<double>(value);
        std::printf("%" PRIu64 " / %" PRIi64 " / %f", as_unsigned, as_signed, as_float);
    }
};

// wire type 2
struct field_bytes
{
    // Essentially a string_view into a buffer.
    lexy::buffer_lexeme<lexy::byte_encoding> value;

    void print() const
    {
        // We print the value as raw bytes.
        // It could be a string or a nested message, but without the schema, we can't know it.
        lexy::visualize(stdout, value);
    }
};

using field_value = std::variant<field_varint, field_32, field_64, field_bytes>;

struct field
{
    std::uint64_t number;
    field_value   value;

    void print() const
    {
        std::printf("%" PRIu64 ": ", number);
        std::visit([](const auto& value) { value.print(); }, value);
        std::printf("\n");
    }
};

using message = std::vector<field>;
} // namespace ast

// The binary encoding of a protobuf message.
// Follows the documentation here: https://developers.google.com/protocol-buffers/docs/encoding
namespace grammar
{
namespace dsl = lexy::dsl;

// Builds a varint.
// This does not handle overflow (it simply computes the value module 2**64),
// nor overlong encodings for simplicity.
// (This would be a nested class of varint, but C++'s constexpr requirements can't handle it).
class varint_builder
{
public:
    constexpr varint_builder() : result(0), bit_shift(0) {}

    void add_prefix_byte(std::uint8_t prefix)
    {
        auto value = prefix & 0b0111'1111u;
        result |= (value << bit_shift);
        bit_shift += 7;
    }

    std::uint64_t finish(std::uint8_t last)
    {
        result |= (static_cast<std::uint64_t>(last) << bit_shift);
        return result;
    }

private:
    std::uint64_t result;
    unsigned      bit_shift;
};

// A Base 128 varint.
struct varint
{
    struct missing_byte
    {
        static constexpr auto name = "missing varint byte";
    };

    static constexpr auto rule = [] {
        // The last byte has the MSB set to zero.
        auto last_byte = dsl::bits(dsl::bit::_0, dsl::bit::any<7>).error<missing_byte>;
        // Other bytes have the MSB set to one.
        auto prefix_byte = dsl::bits(dsl::bit::_1, dsl::bit::any<7>).error<missing_byte>;

        // A varint is a list of prefix bytes terminated by the last byte.
        // We convert each one into integers.
        return dsl::terminator(dsl::bint8(last_byte)).opt_list(dsl::bint8(prefix_byte));
    }();

    static constexpr auto value = [] {
        // The rule passes each prefix byte to the sink.
        // It then invokes the callback with either `(nullopt, last_byte)` if no prefix byte,
        // or `(sink-result, last_byte)` if there were prefix byte.

        // As sink, we create a builder and call `add_prefix_byte()` for every byte that is part of
        // the prefix list.
        auto sink = lexy::fold_inplace<varint_builder>({}, &varint_builder::add_prefix_byte);

        auto callback = lexy::callback<std::uint64_t>(
            // If we had no list, we create an empty builder and finish.
            [](lexy::nullopt, std::uint8_t prefix) { return varint_builder().finish(prefix); },
            // Otherwise, we finish the existing builder.
            &varint_builder::finish);

        return sink >> callback;
    }();
};

// Just parse a varint and convert to the field type.
struct field_varint
{
    static constexpr auto rule  = dsl::p<varint>;
    static constexpr auto value = lexy::construct<ast::field_varint>;
};

// A little endian 32-bit number.
struct field_32
{
    static constexpr auto rule  = dsl::little_bint32;
    static constexpr auto value = lexy::construct<ast::field_32>;
};

// A little endian 64-bit number.
struct field_64
{
    static constexpr auto rule  = dsl::little_bint64;
    static constexpr auto value = lexy::construct<ast::field_64>;
};

// N bytes, where N is given by a varint.
struct field_bytes
{
    static constexpr auto rule  = dsl::repeat(dsl::p<varint>).capture(dsl::byte);
    static constexpr auto value = lexy::construct<ast::field_bytes>;
};

// The key of a field.
struct field_key
{
    struct result
    {
        std::uint64_t number;
        int           type;
    };

    // It's just a varint.
    static constexpr auto rule = dsl::p<varint>;
    // But we split it into the number and wire type.
    static constexpr auto value = lexy::callback<result>([](std::uint64_t v) {
        return result{v >> 3, static_cast<int>(v & 0b111)};
    });
};

// A field needs to dispatch a production based on the wire type.
// We can't express that with the DSL, so need to manually parse it using the scanner.
struct field : lexy::scan_production<ast::field>
{
    struct unknown_field_type
    {
        static constexpr auto name = "unknown field type";
    };

    // This function defines how we're parsing it.
    // It also gets acess to any parse state passed to the action,
    // so it could e.g. get a schema and receive additional type information based on that.
    template <typename Reader, typename Context>
    static scan_result scan(lexy::rule_scanner<Context, Reader>& scanner)
    {
        // We first parse a key.
        auto key = scanner.parse(field_key{});
        if (!scanner)
            return lexy::scan_failed;

        // And then parse the corresponding value.
        lexy::scan_result<ast::field_value> value;
        switch (key.value().type)
        {
        case 0:
            scanner.parse(value, dsl::p<field_varint>);
            break;
        case 1:
            scanner.parse(value, dsl::p<field_64>);
            break;
        case 2:
            scanner.parse(value, dsl::p<field_bytes>);
            break;
        case 5:
            scanner.parse(value, dsl::p<field_32>);
            break;

        default:
            // We have an unknown wire type.
            // Calling `fatal_error()` will put the scanner in a failed state, caught by the check
            // below.
            scanner.fatal_error(unknown_field_type{}, scanner.begin(), scanner.position());
            break;
        }
        if (!scanner)
            return lexy::scan_failed;

        return ast::field{key.value().number, value.value()};
    }
};

// A message is a list of fields until we reach EOF.
struct message
{
    static constexpr auto rule  = dsl::terminator(dsl::eof).opt_list(dsl::p<field>);
    static constexpr auto value = lexy::as_list<ast::message>;
};
} // namespace grammar

#ifndef LEXY_TEST
int main(int argc, char** argv)
{
    if (argc < 2)
    {
        std::fprintf(stderr, "usage: %s <filename>", argv[0]);
        return 1;
    }

    // We're reading the file in binary.
    auto file = lexy::read_file<lexy::byte_encoding>(argv[1]);
    if (!file)
    {
        std::fprintf(stderr, "file '%s' not found", argv[1]);
        return 1;
    }

    auto result = lexy::parse<grammar::message>(file.buffer(), lexy_ext::report_error);
    if (!result)
        return 2;

    for (auto& field : result.value())
    {
        field.print();
        std::printf("\n");
    }
}
#endif

