// Copyright (C) 2020-2024 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_INPUT_PARSE_TREE_INPUT_HPP_INCLUDED
#define LEXY_INPUT_PARSE_TREE_INPUT_HPP_INCLUDED

#include <lexy/error.hpp>
#include <lexy/grammar.hpp>
#include <lexy/input/base.hpp>
#include <lexy/lexeme.hpp>

#if !LEXY_EXPERIMENTAL
#    error "lexy::parse_tree_input is experimental"
#endif

namespace lexy
{
template <typename Node>
struct parse_tree_input_traits;

struct _parse_tree_eof // not real EOF, just no more siblings
{
    template <typename Node>
    friend constexpr bool operator==(const Node& node, _parse_tree_eof) noexcept
    {
        return parse_tree_input_traits<Node>::is_null(node);
    }
    template <typename Node>
    friend constexpr bool operator==(_parse_tree_eof, const Node& node) noexcept
    {
        return parse_tree_input_traits<Node>::is_null(node);
    }

    template <typename Node>
    friend constexpr bool operator!=(const Node& node, _parse_tree_eof) noexcept
    {
        return !parse_tree_input_traits<Node>::is_null(node);
    }
    template <typename Node>
    friend constexpr bool operator!=(_parse_tree_eof, const Node& node) noexcept
    {
        return !parse_tree_input_traits<Node>::is_null(node);
    }
};

template <typename Node>
class parse_tree_encoding
{
    using _traits = parse_tree_input_traits<Node>;

public:
    using char_encoding = typename _traits::char_encoding;
    using char_type     = typename char_encoding::char_type;
    using value_type    = Node;

    static LEXY_CONSTEVAL auto eof()
    {
        return _parse_tree_eof{};
    }

    template <typename NodeKind>
    static bool match(const Node& node, const NodeKind& node_kind)
    {
        return _traits::has_kind(node, node_kind);
    }
};
template <typename Node>
constexpr auto is_node_encoding<parse_tree_encoding<Node>> = true;

template <typename Node>
class _ptr // parse tree reader
{
    using _traits = parse_tree_input_traits<Node>;

public:
    using encoding = parse_tree_encoding<Node>;
    using iterator = typename _traits::iterator;

    struct marker
    {
        Node _parent = _traits::null();
        Node _cur    = _traits::null();

        constexpr iterator position() const noexcept
        {
            return _cur == _parse_tree_eof{} ? _traits::position_end(_parent)
                                             : _traits::position_begin(_cur);
        }
    };

    constexpr explicit _ptr(const Node& root) noexcept
    : _parent(root), _cur(_traits::first_child(root))
    {}

    constexpr _ptr child_reader() const& noexcept
    {
        return _ptr(_cur);
    }
    constexpr auto lexeme_reader() const& noexcept
    {
        auto lexeme = _traits::lexeme(_cur);
        return _range_reader<typename encoding::char_encoding>(lexeme.begin(), lexeme.end());
    }

    constexpr const Node& peek() const noexcept
    {
        return _cur;
    }

    constexpr void bump() noexcept
    {
        LEXY_PRECONDITION(_cur != _parse_tree_eof{});
        _cur = _traits::sibling(_cur);
    }

    constexpr marker current() const noexcept
    {
        return {_parent, _cur};
    }
    constexpr void reset(marker m) noexcept
    {
        _cur = m._cur;
    }

    constexpr iterator position() const noexcept
    {
        return current().position();
    }

private:
    Node _parent;
    Node _cur;
};

template <typename Node>
class parse_tree_input
{
public:
    using encoding   = parse_tree_encoding<Node>;
    using value_type = Node;

    //=== constructors ===//
    constexpr parse_tree_input() noexcept : _root(nullptr) {}

    constexpr explicit parse_tree_input(Node root) noexcept : _root(LEXY_MOV(root)) {}

    template <typename ParseTree, typename = std::enable_if_t<std::is_same_v<
                                      Node, LEXY_DECAY_DECLTYPE(LEXY_DECLVAL(ParseTree).root())>>>
    constexpr explicit parse_tree_input(const ParseTree& tree) noexcept : _root(tree.root())
    {}

    //=== access ===//
    constexpr const Node& root() const noexcept
    {
        return _root;
    }

    //=== reader ===//
    constexpr auto reader() const& noexcept
    {
        return _ptr<Node>(_root);
    }

private:
    Node _root;
};

template <typename ParseTree>
parse_tree_input(const ParseTree&)
    -> parse_tree_input<LEXY_DECAY_DECLTYPE(LEXY_DECLVAL(ParseTree).root())>;

//=== convenience typedefs ===//
template <typename Node>
using parse_tree_lexeme = lexeme_for<parse_tree_input<Node>>;

template <typename Tag, typename Node>
using parse_tree_error = error_for<parse_tree_input<Node>, Tag>;

template <typename Node>
using parse_tree_error_context = error_context<parse_tree_input<Node>>;
} // namespace lexy

#endif // LEXY_INPUT_PARSE_TREE_INPUT_HPP_INCLUDED

