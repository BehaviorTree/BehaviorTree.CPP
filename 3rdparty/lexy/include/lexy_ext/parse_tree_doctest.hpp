// Copyright (C) 2020-2024 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_EXT_PARSE_TREE_DOCTEST_HPP_INCLUDED
#define LEXY_EXT_PARSE_TREE_DOCTEST_HPP_INCLUDED

#include <cctype>
#include <cstdio>
#include <doctest/doctest.h>
#include <lexy/parse_tree.hpp>

namespace lexy_ext
{
template <typename TokenKind = void>
class parse_tree_desc
{
public:
    parse_tree_desc() : _level(0)
    {
        _tree += "\n";
    }

    parse_tree_desc(const char* root_name) : parse_tree_desc()
    {
        production(root_name);
    }
    template <typename RootProduction, typename = lexy::production_rule<RootProduction>>
    parse_tree_desc(RootProduction) : parse_tree_desc(lexy::production_name<RootProduction>())
    {}

    template <typename Iterator>
    parse_tree_desc& token(lexy::token_kind<TokenKind> kind, Iterator begin, Iterator end)
    {
        prefix();

        _tree += doctest::String(kind.name());

        _tree += ": \"";
        for (auto iter = begin; iter != end; ++iter)
        {
            auto c = *iter;
            if (c == '"')
            {
                _tree += "\\\"";
            }
            else if (std::isprint(c))
            {
                char str[] = {char(c), 0};
                _tree += str;
            }
            else
            {
                char buffer[16];
                auto size = std::snprintf(buffer, 16, "\\{%x}", unsigned(c) % 0xFF);
                LEXY_ASSERT(0 <= size && size < 16, "formatting error");
                _tree += buffer;
            }
        }
        _tree += "\"\n";
        return *this;
    }
    template <typename CharT>
    parse_tree_desc& token(lexy::token_kind<TokenKind> kind, const CharT* spelling)
    {
        auto end = spelling;
        while (*end)
            ++end;

        return token(kind, spelling, end);
    }
    template <typename CharT>
    parse_tree_desc& token(const CharT* spelling)
    {
        return token(lexy::unknown_token_kind, spelling);
    }
    template <typename CharT>
    parse_tree_desc& literal(const CharT* spelling)
    {
        return token(lexy::literal_token_kind, spelling);
    }
    template <typename CharT>
    parse_tree_desc& digits(const CharT* spelling)
    {
        return token(lexy::digits_token_kind, spelling);
    }
    template <typename CharT>
    parse_tree_desc& whitespace(const CharT* spelling)
    {
        return token(lexy::whitespace_token_kind, spelling);
    }

    parse_tree_desc& eof()
    {
        return token(lexy::eof_token_kind, "");
    }

    parse_tree_desc& production(const char* name)
    {
        prefix();
        _tree += doctest::String(name);
        _tree += ":\n";

        ++_level;

        return *this;
    }
    template <typename Production, typename = lexy::production_rule<Production>>
    parse_tree_desc& production(Production)
    {
        return production(lexy::production_name<Production>());
    }

    parse_tree_desc& finish()
    {
        --_level;
        return *this;
    }

    friend doctest::String toString(const parse_tree_desc& expected)
    {
        return expected._tree + doctest::String("         ");
    }

    template <typename Reader, typename MemoryResource>
    friend bool operator==(const parse_tree_desc&                                     desc,
                           const lexy::parse_tree<Reader, TokenKind, MemoryResource>& tree)
    {
        using string_maker
            = doctest::StringMaker<lexy::parse_tree<Reader, TokenKind, MemoryResource>>;
        return toString(desc) == string_maker::convert(tree);
    }
    template <typename Reader, typename MemoryResource>
    friend bool operator==(const lexy::parse_tree<Reader, TokenKind, MemoryResource>& tree,
                           const parse_tree_desc&                                     desc)
    {
        using string_maker
            = doctest::StringMaker<lexy::parse_tree<Reader, TokenKind, MemoryResource>>;
        return toString(desc) == string_maker::convert(tree);
    }

private:
    void prefix()
    {
        // First indent to align output regardless of level.
        _tree += "            ";

        // Then indent child nodes.
        if (_level > 0)
        {
            for (auto i = 0; i != _level - 1; ++i)
                _tree += "  ";
            _tree += "- ";
        }
    }

    doctest::String _tree;
    int             _level;
};
} // namespace lexy_ext

namespace doctest
{
template <typename Reader, typename TokenKind, typename MemoryResource>
struct StringMaker<lexy::parse_tree<Reader, TokenKind, MemoryResource>>
{
    using parse_tree = lexy::parse_tree<Reader, TokenKind, MemoryResource>;

    static String convert(const parse_tree& tree)
    {
        lexy_ext::parse_tree_desc<TokenKind> builder;

        for (auto [event, node] : tree.traverse())
            switch (event)
            {
            case lexy::traverse_event::enter:
                builder.production(node.kind().name());
                break;
            case lexy::traverse_event::exit:
                builder.finish();
                break;

            case lexy::traverse_event::leaf: {
                auto token = node.token();
                builder.token(token.kind(), token.lexeme().begin(), token.lexeme().end());
                break;
            }
            }

        return toString(builder);
    }
};
} // namespace doctest

#endif // LEXY_EXT_PARSE_TREE_DOCTEST_HPP_INCLUDED

