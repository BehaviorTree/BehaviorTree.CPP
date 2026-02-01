// Copyright (C) 2020-2024 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_EXT_PARSE_TREE_ALGORITHM_HPP_INCLUDED
#define LEXY_EXT_PARSE_TREE_ALGORITHM_HPP_INCLUDED

#include <lexy/parse_tree.hpp>
#include <optional>

namespace lexy_ext
{
/// Returns a range that contains only the token nodes that are descendants of the node.
/// If the node is itself a token, returns a range that contains only the node itself.
template <typename Reader, typename TokenKind, typename MemoryResource>
auto tokens(const lexy::parse_tree<Reader, TokenKind, MemoryResource>&         tree,
            typename lexy::parse_tree<Reader, TokenKind, MemoryResource>::node node)
{
    using tree_t            = lexy::parse_tree<Reader, TokenKind, MemoryResource>;
    using node_t            = typename tree_t::node;
    using traverse_range    = typename tree_t::traverse_range;
    using traverse_iterator = typename traverse_range::iterator;

    class token_range
    {
    public:
        class iterator;
        struct sentinel : lexy::_detail::sentinel_base<sentinel, iterator>
        {};

        class iterator : public lexy::_detail::forward_iterator_base<iterator, node_t, node_t, void>
        {
        public:
            iterator() noexcept = default;

            auto deref() const noexcept
            {
                LEXY_PRECONDITION(_cur != _end);
                LEXY_ASSERT(_cur->event == lexy::traverse_event::leaf, "invalid increment");
                return _cur->node;
            }

            void increment() noexcept
            {
                LEXY_PRECONDITION(_cur != _end);

                // Advance at least once.
                ++_cur;
                // Continue advancing until the next token is found.
                while (_cur != _end && _cur->event != lexy::traverse_event::leaf)
                    ++_cur;
            }

            bool equal(iterator rhs) const noexcept
            {
                return _cur == rhs._cur;
            }
            bool is_end() const noexcept
            {
                return _cur == _end;
            }

        private:
            explicit iterator(const traverse_range& range) noexcept
            : _cur(range.begin()), _end(range.end())
            {
                // Advancing until initial token is found.
                while (_cur != _end && _cur->event != lexy::traverse_event::leaf)
                    ++_cur;
            }

            traverse_iterator _cur;
            traverse_iterator _end;

            friend token_range;
        };

        explicit token_range(const traverse_range& range) noexcept : _begin(range) {}

        bool empty() const noexcept
        {
            return begin() == end();
        }

        iterator begin() const noexcept
        {
            return _begin;
        }

        sentinel end() const noexcept
        {
            return {};
        }

    private:
        iterator _begin;
    };

    return token_range(tree.traverse(node));
}

template <typename Reader, typename TokenKind, typename MemoryResource>
auto tokens(const lexy::parse_tree<Reader, TokenKind, MemoryResource>& tree)
{
    LEXY_PRECONDITION(!tree.empty());
    return tokens(tree, tree.root());
}
} // namespace lexy_ext

namespace lexy_ext
{
/// Returns the node of the tree that covers the position.
/// It is always a token.
template <typename Reader, typename TokenKind, typename MemoryResource>
auto find_covering_node(const lexy::parse_tree<Reader, TokenKind, MemoryResource>& tree,
                        typename Reader::iterator                                  position) ->
    typename lexy::parse_tree<Reader, TokenKind, MemoryResource>::node
{
    LEXY_PRECONDITION(!tree.empty());

    // Just do a linear search over all the tokens.
    // This can be done more efficiently by pruning subtrees, but this should be fast enough.
    for (auto token : tokens(tree))
    {
        if (position < token.lexeme().end())
            // We've reached the first token that reaches past the position.
            // This is the covering one.
            return token;
    }

    LEXY_PRECONDITION(false); // Position out of bounds.
    return tree.root();
}
} // namespace lexy_ext

namespace lexy_ext
{
template <typename Predicate, typename Iterator, typename Sentinel>
class _filtered_node_range
{
    using node_t = decltype(*Iterator());

public:
    class iterator;
    struct sentinel : lexy::_detail::sentinel_base<sentinel, iterator>
    {};

    class iterator : public lexy::_detail::forward_iterator_base<iterator, node_t, node_t, void>
    {
    public:
        iterator() noexcept = default;

        auto deref() const noexcept
        {
            LEXY_PRECONDITION(_cur != _end);
            return *_cur;
        }

        void increment() noexcept
        {
            LEXY_PRECONDITION(_cur != _end);

            // Advance at least once.
            ++_cur;
            // Continue advancing until the next matching is found.
            while (_cur != _end && !_predicate(*_cur))
                ++_cur;
        }

        bool equal(iterator rhs) const noexcept
        {
            return _cur == rhs._cur;
        }
        bool is_end() const noexcept
        {
            return _cur == _end;
        }

    private:
        explicit iterator(Predicate&& pred, Iterator cur, Sentinel end) noexcept
        : _cur(cur), _end(end), _predicate(LEXY_MOV(pred))
        {
            // Advancing until initial token is found.
            while (_cur != _end && !_predicate(*_cur))
                ++_cur;
        }

        Iterator                    _cur;
        LEXY_EMPTY_MEMBER Sentinel  _end;
        LEXY_EMPTY_MEMBER Predicate _predicate;

        friend _filtered_node_range;
    };

    explicit _filtered_node_range(Predicate&& pred, Iterator begin, Sentinel end) noexcept
    : _begin(LEXY_MOV(pred), begin, end)
    {}

    bool empty() const noexcept
    {
        return begin() == end();
    }

    iterator begin() const noexcept
    {
        return _begin;
    }

    sentinel end() const noexcept
    {
        return {};
    }

private:
    iterator _begin;
};

template <typename Predicate, typename Iterator, typename Sentinel>
_filtered_node_range(Predicate&& pred, Iterator begin, Sentinel end) noexcept
    -> _filtered_node_range<std::decay_t<Predicate>, Iterator, Sentinel>;

/// Returns the children that of node that match the predicate.
///
/// If predicate is a token kind, keeps only children of the same token kind.
/// If predicate is a production, keeps only children of that production.
/// Otherwise, predicate is a function object that is invoked with the node.
template <typename Reader, typename TokenKind, typename MemoryResource, typename Predicate>
auto children(const lexy::parse_tree<Reader, TokenKind, MemoryResource>&,
              typename lexy::parse_tree<Reader, TokenKind, MemoryResource>::node node,
              Predicate                                                          predicate)
{
    if constexpr (std::is_constructible_v<lexy::token_kind<TokenKind>, Predicate>)
        return _filtered_node_range([kind = predicate](auto n) { return n.kind() == kind; },
                                    node.children().begin(), node.children().end());
    else if constexpr (lexy::is_production<Predicate>)
        return _filtered_node_range([](auto n) { return n.kind() == Predicate{}; },
                                    node.children().begin(), node.children().end());
    else
        return _filtered_node_range(LEXY_MOV(predicate), node.children().begin(),
                                    node.children().end());
}

/// Returns the first child that matches predicate, if there is any.
template <typename Reader, typename TokenKind, typename MemoryResource, typename Predicate>
auto child(const lexy::parse_tree<Reader, TokenKind, MemoryResource>&         tree,
           typename lexy::parse_tree<Reader, TokenKind, MemoryResource>::node node,
           Predicate                                                          predicate)
    -> std::optional<typename lexy::parse_tree<Reader, TokenKind, MemoryResource>::node>
{
    auto range = children(tree, node, predicate);
    if (range.empty())
        return std::nullopt;
    else
        return *range.begin();
}
} // namespace lexy_ext

namespace lexy_ext
{
/// Returns the position of a node.
///
/// For a token node, this is the beginning of its lexeme.
/// For a production node, this is the position designated by a `dsl::position` rule, or otherwise
/// the position of its first non-empty child. If a production node is empty, it returns a default
/// constructed iterator.
template <typename Reader, typename TokenKind, typename MemoryResource>
auto node_position(const lexy::parse_tree<Reader, TokenKind, MemoryResource>&         tree,
                   typename lexy::parse_tree<Reader, TokenKind, MemoryResource>::node node) ->
    typename Reader::iterator
{
    if (auto pos_node = child(tree, node, lexy::position_token_kind))
        return pos_node->lexeme().begin();
    else if (auto tokens = lexy_ext::tokens(tree, node); tokens.empty())
        // The node does not have a token as descendant.
        return {};
    else
        return tokens.begin()->lexeme().begin();
}
} // namespace lexy_ext

#endif // LEXY_EXT_PARSE_TREE_ALGORITHM_HPP_INCLUDED

