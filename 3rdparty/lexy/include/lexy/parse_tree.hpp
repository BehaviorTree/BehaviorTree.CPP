// Copyright (C) 2020-2024 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_PARSE_TREE_HPP_INCLUDED
#define LEXY_PARSE_TREE_HPP_INCLUDED

#include <lexy/_detail/assert.hpp>
#include <lexy/_detail/config.hpp>
#include <lexy/_detail/iterator.hpp>
#include <lexy/_detail/memory_resource.hpp>
#include <lexy/grammar.hpp>
#include <lexy/token.hpp>

namespace lexy
{
template <typename Node>
struct parse_tree_input_traits;
}

//=== internal: pt_node ===//
namespace lexy::_detail
{
template <typename Reader>
class pt_node;
template <typename Reader>
struct pt_node_token;
template <typename Reader>
struct pt_node_production;

template <typename Reader>
class pt_node
{
public:
    static constexpr auto type_token      = 0b0u;
    static constexpr auto type_production = 0b1u;

    static constexpr auto role_sibling = 0b0u;
    static constexpr auto role_parent  = 0b1u;

    //=== information about the current node ===//
    unsigned type() const noexcept
    {
        return _value & 0b1;
    }

    auto as_token() noexcept
    {
        return type() == type_token ? static_cast<pt_node_token<Reader>*>(this) : nullptr;
    }
    auto as_production() noexcept
    {
        return type() == type_production ? static_cast<pt_node_production<Reader>*>(this) : nullptr;
    }

    //=== information about the next node ===//
    void set_sibling(pt_node<Reader>* sibling) noexcept
    {
        _value = _make_packed_ptr(sibling, type(), role_sibling);
    }
    void set_parent(pt_node_production<Reader>* parent) noexcept
    {
        _value = _make_packed_ptr(parent, type(), role_parent);
    }

    unsigned next_role() const noexcept
    {
        return (_value & 0b10) >> 1;
    }

    pt_node<Reader>* next_node() const noexcept
    {
        // NOLINTNEXTLINE: We need pointer conversion.
        return reinterpret_cast<pt_node<Reader>*>(_value & ~std::uintptr_t(0b11));
    }

protected:
    explicit pt_node(unsigned type)
    // We initializy it to a null parent pointer.
    // This means it is automatically an empty child range.
    : _value(_make_packed_ptr(nullptr, type, role_parent))
    {}

private:
    static std::uintptr_t _make_packed_ptr(pt_node<Reader>* ptr, unsigned type, unsigned role)
    {
        auto result = reinterpret_cast<std::uintptr_t>(ptr);
        LEXY_PRECONDITION((result & 0b11) == 0);

        result |= (role & 0b1) << 1;
        result |= (type & 0b1);

        return result;
    }

    // Stores an address of the "next" node in the tree.
    // Stores a bit that remembers whether the next node is a sibling or parent (the role).
    // Stores a bit that remembers whether *this* node is a token or production.
    std::uintptr_t _value;
};

template <typename Reader>
struct pt_node_token : pt_node<Reader>
{
    // If it's random access, we store size instead of end.
    static constexpr auto _optimize_end
        = _detail::is_random_access_iterator<typename Reader::iterator>;
    using _end_t
        // If we can optimize it, we store the size as a uint32_t, otherwise the iterator.
        = std::conditional_t<_optimize_end, std::uint_least32_t, typename Reader::iterator>;

    typename Reader::iterator begin;
    _end_t                    end_impl;
    ::uint_least16_t          kind;

    explicit pt_node_token(std::uint_least16_t kind, typename Reader::iterator begin,
                           typename Reader::iterator end) noexcept
    : pt_node<Reader>(pt_node<Reader>::type_token), begin(begin), kind(kind)
    {
        update_end(end);
    }

    typename Reader::iterator end() const noexcept
    {
        if constexpr (_optimize_end)
            return begin + end_impl;
        else
            return end_impl;
    }

    void update_end(typename Reader::iterator end) noexcept
    {
        if constexpr (_optimize_end)
        {
            static_assert(!std::is_pointer_v<typename Reader::iterator>
                          || sizeof(pt_node_token) == 3 * sizeof(void*));

            auto size = std::size_t(end - begin);
            LEXY_PRECONDITION(size <= UINT_LEAST32_MAX);
            end_impl = std::uint_least32_t(size);
        }
        else
        {
            static_assert(!std::is_pointer_v<typename Reader::iterator>
                          || sizeof(pt_node_token) <= 4 * sizeof(void*));

            end_impl = end;
        }
    }
};

template <typename Reader>
struct pt_node_production : pt_node<Reader>
{
    static constexpr std::size_t child_count_bits = sizeof(std::size_t) * CHAR_BIT - 2;

    const char* const* id;
    std::size_t        child_count : child_count_bits;
    std::size_t        token_production : 1;
    std::size_t        first_child_adjacent : 1;

    explicit pt_node_production(production_info info) noexcept
    : pt_node<Reader>(pt_node<Reader>::type_production), id(info.id), child_count(0),
      token_production(info.is_token), first_child_adjacent(true)
    {
        static_assert(sizeof(pt_node_production) == 3 * sizeof(void*));
        LEXY_PRECONDITION(!info.is_transparent);
    }

    pt_node<Reader>* first_child()
    {
        auto memory = static_cast<void*>(this + 1);
        if (child_count == 0)
        {
            // We don't have a child at all.
            return nullptr;
        }
        else if (first_child_adjacent)
        {
            // The first child is stored immediately afterwards.
            return static_cast<pt_node<Reader>*>(memory);
        }
        else
        {
            // We're only storing a pointer to the first child immediately afterwards.
            return *static_cast<pt_node<Reader>**>(memory);
        }
    }
};
} // namespace lexy::_detail

//=== internal: pt_buffer ===//
namespace lexy::_detail
{
// Basic stack allocator to store all the nodes of a tree.
template <typename MemoryResource>
class pt_buffer
{
    using resource_ptr = _detail::memory_resource_ptr<MemoryResource>;

    static constexpr std::size_t block_size = 4096 - sizeof(void*);

    struct block
    {
        block*        next;
        unsigned char memory[block_size];

        static block* allocate(resource_ptr resource)
        {
            auto memory = resource->allocate(sizeof(block), alignof(block));
            auto ptr    = ::new (memory) block; // Don't initialize array!
            ptr->next   = nullptr;
            return ptr;
        }

        static block* deallocate(resource_ptr resource, block* ptr)
        {
            auto next = ptr->next;
            resource->deallocate(ptr, sizeof(block), alignof(block));
            return next;
        }

        unsigned char* end() noexcept
        {
            return &memory[block_size];
        }
    };

public:
    //=== constructors/destructors/assignment ===//
    explicit constexpr pt_buffer(MemoryResource* resource) noexcept
    : _resource(resource), _head(nullptr), _cur_block(nullptr), _cur_pos(nullptr)
    {}

    pt_buffer(pt_buffer&& other) noexcept
    : _resource(other._resource), _head(other._head), _cur_block(other._cur_block),
      _cur_pos(other._cur_pos)
    {
        other._head = other._cur_block = nullptr;
        other._cur_pos                 = nullptr;
    }

    ~pt_buffer() noexcept
    {
        auto cur = _head;
        while (cur != nullptr)
            cur = block::deallocate(_resource, cur);
    }

    pt_buffer& operator=(pt_buffer&& other) noexcept
    {
        lexy::_detail::swap(_resource, other._resource);
        lexy::_detail::swap(_head, other._head);
        lexy::_detail::swap(_cur_block, other._cur_block);
        lexy::_detail::swap(_cur_pos, other._cur_pos);
        return *this;
    }

    //=== allocation ===//
    // Allocates the first block for the buffer.
    // Must be called before everything else.
    // (If done in the constructor, it would require a move that does allocation which we don't
    // want).
    // If called after being initialized, destroys all nodes without releasing memory.
    void reset()
    {
        if (!_head)
            _head = block::allocate(_resource);

        _cur_block = _head;
        _cur_pos   = &_cur_block->memory[0];
    }

    void reserve(std::size_t size)
    {
        if (remaining_capacity() < size)
        {
            auto next        = block::allocate(_resource);
            _cur_block->next = next;
            _cur_block       = next;
            _cur_pos         = &_cur_block->memory[0];
        }
    }

    template <typename T, typename... Args>
    T* allocate(Args&&... args)
    {
        static_assert(std::is_trivially_copyable_v<T>);
        static_assert(alignof(T) == alignof(void*));
        constexpr auto size = sizeof(T); // NOLINT: It's fine, we want to allocate for a pointer.
        LEXY_PRECONDITION(_cur_block);   // Forgot to call .init().
        LEXY_PRECONDITION(remaining_capacity() >= size); // Forgot to call .reserve().

        auto memory = _cur_pos;
        _cur_pos += size;
        return ::new (static_cast<void*>(memory)) T(LEXY_FWD(args)...);
    }

    void* top()
    {
        return _cur_pos;
    }

    void unwind(void* marker) noexcept
    {
        auto pos = static_cast<unsigned char*>(marker);

        // Note: this is not guaranteed to work by the standard;
        // We'd have to go through std::less instead.
        // However, on all implementations I care about, std::less just does < anyway.
        if (_cur_block->memory <= pos && pos < _cur_block->end())
            // We're still in the same block, just reset position.
            _cur_pos = pos;
        else
            // Reset to the beginning of the current block only.
            // This can waste memory, but this is not a problem here:
            // unwind() is only used to backtrack a production, which happens after a couple of
            // tokens only; the memory waste is directly proportional to the lookahead length.
            _cur_pos = _cur_block->memory;
    }

private:
    std::size_t remaining_capacity() const noexcept
    {
        return std::size_t(_cur_block->end() - _cur_pos);
    }

    LEXY_EMPTY_MEMBER resource_ptr _resource;
    block*                         _head;

    block*         _cur_block;
    unsigned char* _cur_pos;
};
} // namespace lexy::_detail

//=== parse_tree ===//
namespace lexy
{
template <typename Reader, typename TokenKind>
class _pt_node_kind;
template <typename Reader, typename TokenKind>
class _pt_node;

template <typename Reader, typename TokenKind = void, typename MemoryResource = void>
class parse_tree
{
    static_assert(lexy::is_char_encoding<typename Reader::encoding>);

public:
    //=== construction ===//
    class builder;

    constexpr parse_tree() : parse_tree(_detail::get_memory_resource<MemoryResource>()) {}
    constexpr explicit parse_tree(MemoryResource* resource)
    : _buffer(resource), _root(nullptr), _size(0), _depth(0)
    {}

    //=== container access ===//
    bool empty() const noexcept
    {
        return _root == nullptr;
    }

    std::size_t size() const noexcept
    {
        return _size;
    }

    std::size_t depth() const noexcept
    {
        LEXY_PRECONDITION(!empty());
        return _depth;
    }

    void clear() noexcept
    {
        _buffer.reset();
        _root = nullptr;
    }

    //=== node access ===//
    using node_kind = _pt_node_kind<Reader, TokenKind>;
    using node      = _pt_node<Reader, TokenKind>;

    node root() const noexcept
    {
        LEXY_PRECONDITION(!empty());
        return node(_root);
    }

    //=== traverse ===//
    class traverse_range;

    traverse_range traverse(const node& n) const noexcept
    {
        return traverse_range(n);
    }
    traverse_range traverse() const noexcept
    {
        if (empty())
            return traverse_range();
        else
            return traverse_range(root());
    }

    //=== remaining input ===//
    lexy::lexeme<Reader> remaining_input() const noexcept
    {
        if (empty())
            return {};

        auto token = _root->next_node()->as_token();
        return {token->begin, token->end()};
    }

private:
    _detail::pt_buffer<MemoryResource>   _buffer;
    _detail::pt_node_production<Reader>* _root;
    std::size_t                          _size;
    std::size_t                          _depth;
};

template <typename Input, typename TokenKind = void, typename MemoryResource = void>
using parse_tree_for = lexy::parse_tree<lexy::input_reader<Input>, TokenKind, MemoryResource>;

template <typename Reader, typename TokenKind, typename MemoryResource>
class parse_tree<Reader, TokenKind, MemoryResource>::builder
{
public:
    class marker
    {
    public:
        marker() : marker(nullptr, 0) {}

    private:
        // Where to unwind when done.
        void* unwind_pos;
        // The current production node.
        // nullptr if using the container API.
        _detail::pt_node_production<Reader>* prod;
        // The number of children we've already added.
        std::size_t child_count;
        // The first and last child of the container.
        _detail::pt_node<Reader>* first_child;
        _detail::pt_node<Reader>* last_child;

        // For a production node, depth of the production node.
        // For a container node, depth of the children.
        std::size_t cur_depth;
        // The maximum local depth seen in the subtree beginning at the marker.
        std::size_t local_max_depth;

        explicit marker(void* unwind_pos, std::size_t cur_depth,
                        _detail::pt_node_production<Reader>* prod = nullptr)
        : unwind_pos(unwind_pos), prod(prod), child_count(0), first_child(nullptr),
          last_child(nullptr), cur_depth(cur_depth), local_max_depth(cur_depth)
        {}

        void clear()
        {
            first_child = nullptr;
            last_child  = nullptr;
            child_count = 0;
        }

        void insert(_detail::pt_node<Reader>* child)
        {
            if (first_child == nullptr)
            {
                // Initialize the pointers.
                first_child = last_child = child;
            }
            else
            {
                // last_child gets a sibling pointer.
                last_child->set_sibling(child);
                // child is now the last child.
                last_child = child;
            }

            ++child_count;
        }
        void insert_list(std::size_t length, _detail::pt_node<Reader>* first,
                         _detail::pt_node<Reader>* last)
        {
            if (length == 0)
                return;

            if (first_child == nullptr)
            {
                first_child = first;
                last_child  = last;
            }
            else
            {
                last_child->set_sibling(first);
                last_child = last;
            }

            child_count += length;
        }

        void insert_children_into(_detail::pt_node_production<Reader>* parent)
        {
            LEXY_PRECONDITION(parent->child_count == 0);
            if (child_count == 0)
                return;

            if (first_child == parent + 1)
            {
                parent->first_child_adjacent = true;
            }
            else
            {
                // This case happens either if the first child is in a new block or if we're
                // dealing with a container node. In either case, we've already reserved memory
                // for a pointer.
                auto memory = static_cast<void*>(parent + 1);
                ::new (memory) _detail::pt_node<Reader>*(first_child);
                parent->first_child_adjacent = false;
            }

            // last_child needs a pointer to the production.
            last_child->set_parent(parent);

            constexpr auto mask
                = ((std::size_t(1) << _detail::pt_node_production<Reader>::child_count_bits) - 1);
            parent->child_count = child_count & mask;
        }

        void update_size_depth(std::size_t& size, std::size_t& max_depth)
        {
            size += child_count;

            if (cur_depth == local_max_depth && child_count > 0)
                // We have children we haven't yet accounted for.
                ++local_max_depth;

            if (max_depth < local_max_depth)
                max_depth = local_max_depth;
        }

        friend builder;
    };

    //=== root node ===//
    explicit builder(parse_tree&& tree, production_info production) : _result(LEXY_MOV(tree))
    {
        // Empty the initial parse tree.
        _result._buffer.reset();

        // Allocate a new root node.
        // No need to reserve for the initial node.
        _result._root
            = _result._buffer.template allocate<_detail::pt_node_production<Reader>>(production);
        _result._size  = 1;
        _result._depth = 0;

        // Begin construction at the root.
        _cur = marker(_result._buffer.top(), 0, _result._root);
    }
    explicit builder(production_info production) : builder(parse_tree(), production) {}

    [[deprecated("Pass the remaining input, or `input.end()` if there is none.")]] parse_tree&&
        finish() &&
    {
        return LEXY_MOV(*this).finish(lexy::lexeme<Reader>());
    }
    parse_tree&& finish(typename Reader::iterator end) &&
    {
        return LEXY_MOV(*this).finish({end, end});
    }
    parse_tree&& finish(lexy::lexeme<Reader> remaining_input) &&
    {
        LEXY_PRECONDITION(_cur.prod == _result._root);

        _cur.insert_children_into(_cur.prod);
        _cur.update_size_depth(_result._size, _result._depth);

        _result._buffer.reserve(sizeof(_detail::pt_node_token<Reader>));
        auto node = _result._buffer
                        .template allocate<_detail::pt_node_token<Reader>>(lexy::eof_token_kind,
                                                                           remaining_input.begin(),
                                                                           remaining_input.end());
        _result._root->set_sibling(node);

        return LEXY_MOV(_result);
    }

    //=== production nodes ===//
    auto start_production(production_info production)
    {
        if (production.is_transparent)
            // Don't need to add a new node for a transparent production.
            return _cur;

        // Allocate a node for the production and append it to the current child list.
        // We reserve enough memory to allow for a trailing pointer.
        // This is only necessary if the first child is in a new block,
        // in which case we won't overwrite it.
        _result._buffer.reserve(sizeof(_detail::pt_node_production<Reader>)
                                + sizeof(_detail::pt_node<Reader>*));
        auto node
            = _result._buffer.template allocate<_detail::pt_node_production<Reader>>(production);
        // Note: don't append the node yet, we might still backtrack.

        // Subsequent insertions are to the new node, so update marker and return old one.
        auto old = LEXY_MOV(_cur);
        _cur     = marker(node, old.cur_depth + 1, node);
        return old;
    }

    void finish_production(marker&& m)
    {
        LEXY_PRECONDITION(_cur.prod || m.prod == _cur.prod);
        if (m.prod == _cur.prod)
            // We're finishing with a transparent production, do nothing.
            return;

        _cur.update_size_depth(_result._size, m.local_max_depth);
        _cur.insert_children_into(_cur.prod);

        // Insert the production node into the parent and continue with it.
        m.insert(_cur.prod);
        _cur = LEXY_MOV(m);
    }

    void cancel_production(marker&& m)
    {
        LEXY_PRECONDITION(_cur.prod || m.prod == _cur.prod);
        if (_cur.prod == m.prod)
            // We're backtracking a transparent production, do nothing.
            return;

        _result._buffer.unwind(_cur.unwind_pos);
        // Continue with parent.
        _cur = LEXY_MOV(m);
    }

    //=== container nodes ===//
    marker start_container()
    {
        auto unwind_pos = _result._buffer.top();
        if (_cur.prod && _cur.child_count == 0)
        {
            // If our parent production doesn't have any children,
            // we might need space for a first child pointer.
            //
            // If our parent is another container, its start_container() function took care of it.
            // If our parent already has a child, the first_child pointer is taken care of.
            _result._buffer.template allocate<_detail::pt_node<Reader>*>(nullptr);
        }

        // Create a new container marker and activate it.
        auto old = LEXY_MOV(_cur);
        _cur     = marker(unwind_pos, old.cur_depth);
        return old;
    }

    void set_container_production(production_info production)
    {
        LEXY_PRECONDITION(!_cur.prod);
        if (production.is_transparent)
            // If the production is transparent, we do nothing.
            return;

        // Allocate a new node for the production.
        // We definitely need space for node pointer at this point,
        // as the first child precedes it.
        _result._buffer.reserve(sizeof(_detail::pt_node_production<Reader>)
                                + sizeof(_detail::pt_node<Reader>*));
        auto node
            = _result._buffer.template allocate<_detail::pt_node_production<Reader>>(production);
        _result._buffer.template allocate<_detail::pt_node<Reader>*>(nullptr);

        // Create a new container that will contain the production as its only child.
        // As such, it logically starts at the same position and depth as the current container.
        auto new_container = marker(_cur.unwind_pos, _cur.cur_depth);
        new_container.insert(node);

        // The production contains all the children.
        _cur.insert_children_into(node);

        // The local_max_depth of the new container is determined by the old maximum depth + 1.
        new_container.local_max_depth = [&] {
            if (_cur.cur_depth == _cur.local_max_depth && _cur.child_count > 0)
                // There are children we haven't yet accounted for.
                return _cur.local_max_depth + 1 + 1;
            else
                return _cur.local_max_depth + 1;
        }();
        _result._size += _cur.child_count;

        // And we continue with the current container.
        _cur = new_container;
    }

    void finish_container(marker&& m)
    {
        LEXY_PRECONDITION(!_cur.prod);

        // Insert the children of our container into the parent.
        m.insert_list(_cur.child_count, _cur.first_child, _cur.last_child);

        // We can't update size yet, it would be double counted.
        // We do need to update the max depth if necessary, however.
        std::size_t size = 0;
        _cur.update_size_depth(size, m.local_max_depth);

        // Continue with the parent.
        _cur = LEXY_MOV(m);
    }

    void cancel_container(marker&& m)
    {
        LEXY_PRECONDITION(!_cur.prod);

        // Deallocate everything we've inserted.
        _result._buffer.unwind(_cur.unwind_pos);
        // Continue with parent.
        _cur = LEXY_MOV(m);
    }

    //=== token nodes ===//
    void token(token_kind<TokenKind> _kind, typename Reader::iterator begin,
               typename Reader::iterator end)
    {
        if (_kind.ignore_if_empty() && begin == end)
            return;

        auto kind = token_kind<TokenKind>::to_raw(_kind);

        // We merge error tokens.
        if (kind == lexy::error_token_kind && _cur.last_child && _cur.last_child->as_token()
            && _cur.last_child->as_token()->kind == lexy::error_token_kind)
        {
            // No need to allocate a new node, just extend the previous node.
            _cur.last_child->as_token()->update_end(end);
        }
        else
        {
            // Allocate and append.
            _result._buffer.reserve(sizeof(_detail::pt_node_token<Reader>));
            auto node
                = _result._buffer.template allocate<_detail::pt_node_token<Reader>>(kind, begin,
                                                                                    end);
            _cur.insert(node);
        }
    }

    //=== accessors ===//
    std::size_t current_child_count() const noexcept
    {
        return _cur.child_count;
    }

private:
    parse_tree _result;
    marker     _cur;
};

template <typename Reader, typename TokenKind>
class _pt_node_kind
{
public:
    bool is_token() const noexcept
    {
        return _ptr->as_token() != nullptr;
    }
    bool is_production() const noexcept
    {
        return _ptr->as_production() != nullptr;
    }

    bool is_root() const noexcept
    {
        // Root node has a next node (the remaining input node) which has no next node.
        // We assume that _ptr is never the remaining input node, so we know that we have a next
        // node.
        return _ptr->next_node()->next_node() == nullptr;
    }
    bool is_token_production() const noexcept
    {
        return is_production() && _ptr->as_production()->token_production;
    }

    const char* name() const noexcept
    {
        if (auto prod = _ptr->as_production())
            return *prod->id;
        else if (auto token = _ptr->as_token())
            return token_kind<TokenKind>::from_raw(token->kind).name();
        else
        {
            LEXY_ASSERT(false, "unreachable");
            return nullptr;
        }
    }

    friend bool operator==(_pt_node_kind lhs, _pt_node_kind rhs)
    {
        if (lhs.is_token() && rhs.is_token())
            return lhs._ptr->as_token()->kind == rhs._ptr->as_token()->kind;
        else
            return lhs._ptr->as_production()->id == rhs._ptr->as_production()->id;
    }
    friend bool operator!=(_pt_node_kind lhs, _pt_node_kind rhs)
    {
        return !(lhs == rhs);
    }

    friend bool operator==(_pt_node_kind nk, token_kind<TokenKind> tk)
    {
        if (auto token = nk._ptr->as_token())
            return token_kind<TokenKind>::from_raw(token->kind) == tk;
        else
            return false;
    }
    friend bool operator==(token_kind<TokenKind> tk, _pt_node_kind nk)
    {
        return nk == tk;
    }
    friend bool operator!=(_pt_node_kind nk, token_kind<TokenKind> tk)
    {
        return !(nk == tk);
    }
    friend bool operator!=(token_kind<TokenKind> tk, _pt_node_kind nk)
    {
        return !(nk == tk);
    }

    friend bool operator==(_pt_node_kind nk, production_info info)
    {
        return nk.is_production() && nk._ptr->as_production()->id == info.id;
    }
    friend bool operator==(production_info info, _pt_node_kind nk)
    {
        return nk == info;
    }
    friend bool operator!=(_pt_node_kind nk, production_info info)
    {
        return !(nk == info);
    }
    friend bool operator!=(production_info info, _pt_node_kind nk)
    {
        return !(nk == info);
    }

private:
    explicit _pt_node_kind(_detail::pt_node<Reader>* ptr) : _ptr(ptr) {}

    _detail::pt_node<Reader>* _ptr;

    friend _pt_node<Reader, TokenKind>;
};

template <typename Reader, typename TokenKind>
class _pt_node
{
public:
    void* address() const noexcept
    {
        return _ptr;
    }

    auto kind() const noexcept
    {
        return _pt_node_kind<Reader, TokenKind>(_ptr);
    }

    auto parent() const noexcept
    {
        if (kind().is_root())
            // The root has itself as parent.
            return *this;

        // If we follow the sibling pointer, we reach a parent pointer.
        auto cur = _ptr;
        while (cur->next_role() == _detail::pt_node<Reader>::role_sibling)
            cur = cur->next_node();
        return _pt_node(cur->next_node());
    }

    class children_range
    {
    public:
        class iterator : public _detail::forward_iterator_base<iterator, _pt_node, _pt_node, void>
        {
        public:
            iterator() noexcept : _cur(nullptr) {}

            auto deref() const noexcept
            {
                return _pt_node(_cur);
            }

            void increment() noexcept
            {
                _cur = _cur->next_node();
            }

            bool equal(iterator rhs) const noexcept
            {
                return _cur == rhs._cur;
            }

        private:
            explicit iterator(_detail::pt_node<Reader>* ptr) noexcept : _cur(ptr) {}

            _detail::pt_node<Reader>* _cur;

            friend children_range;
        };

        bool empty() const noexcept
        {
            return size() == 0;
        }

        std::size_t size() const noexcept
        {
            if (auto prod = _node->as_production())
                return prod->child_count;
            else
                return 0;
        }

        iterator begin() const noexcept
        {
            if (auto prod = _node->as_production(); prod && prod->first_child())
                return iterator(prod->first_child());
            else
                return end();
        }
        iterator end() const noexcept
        {
            // The last child has a next pointer back to the parent,
            // so if we keep following it, we'll end up here.
            return iterator(_node);
        }

    private:
        explicit children_range(_detail::pt_node<Reader>* node) : _node(node)
        {
            LEXY_PRECONDITION(node);
        }

        _detail::pt_node<Reader>* _node;

        friend _pt_node;
    };

    auto children() const noexcept
    {
        return children_range(_ptr);
    }

    class sibling_range
    {
    public:
        class iterator : public _detail::forward_iterator_base<iterator, _pt_node, _pt_node, void>
        {
        public:
            iterator() noexcept : _cur() {}

            auto deref() const noexcept
            {
                return _pt_node(_cur);
            }

            void increment() noexcept
            {
                if (_cur->next_role() == _detail::pt_node<Reader>::role_parent)
                    // We're pointing to the parent, go to first child instead.
                    _cur = _cur->next_node()->as_production()->first_child();
                else
                    // We're pointing to a sibling, go there.
                    _cur = _cur->next_node();
            }

            bool equal(iterator rhs) const noexcept
            {
                return _cur == rhs._cur;
            }

        private:
            explicit iterator(_detail::pt_node<Reader>* ptr) noexcept : _cur(ptr) {}

            _detail::pt_node<Reader>* _cur;

            friend sibling_range;
        };

        bool empty() const noexcept
        {
            return begin() == end();
        }

        iterator begin() const noexcept
        {
            // We begin with the next node after ours.
            // If we don't have siblings, this is our node itself.
            return ++iterator(_node);
        }
        iterator end() const noexcept
        {
            // We end when we're back at the node.
            return iterator(_node);
        }

    private:
        explicit sibling_range(_detail::pt_node<Reader>* node) noexcept : _node(node) {}

        _detail::pt_node<Reader>* _node;

        friend _pt_node;
    };

    auto siblings() const noexcept
    {
        return sibling_range(_ptr);
    }

    bool is_last_child() const noexcept
    {
        // We're the last child if our pointer points to the parent.
        return _ptr->next_role() == _detail::pt_node<Reader>::role_parent;
    }

    auto position() const noexcept -> typename Reader::iterator
    {
        // Find the first descendant that is a token.
        auto cur = _ptr;
        while (cur->type() == _detail::pt_node<Reader>::type_production)
        {
            cur = cur->as_production()->first_child();
            LEXY_PRECONDITION(cur);
        }

        return cur->as_token()->begin;
    }

    auto lexeme() const noexcept
    {
        if (auto token = _ptr->as_token())
            return lexy::lexeme<Reader>(token->begin, token->end());
        else
            return lexy::lexeme<Reader>();
    }

    auto covering_lexeme() const noexcept
    {
        if (auto token = _ptr->as_token())
            return lexy::lexeme<Reader>(token->begin, token->end());

        auto begin = position();

        auto sibling = _ptr;
        while (true)
        {
            auto next_role = sibling->next_role();
            sibling        = sibling->next_node();
            // If we went to parent, we need to continue finding siblings.
            if (next_role == _detail::pt_node<Reader>::role_sibling)
                break;
        }
        auto end = _pt_node(sibling).position();

        LEXY_PRECONDITION(begin == end || end != typename Reader::iterator());
        return lexy::lexeme<Reader>(begin, end);
    }

    auto token() const noexcept
    {
        LEXY_PRECONDITION(kind().is_token());

        auto token = _ptr->as_token();
        auto kind  = token_kind<TokenKind>::from_raw(token->kind);
        return lexy::token<Reader, TokenKind>(kind, token->begin, token->end());
    }

    friend bool operator==(_pt_node lhs, _pt_node rhs) noexcept
    {
        return lhs._ptr == rhs._ptr;
    }
    friend bool operator!=(_pt_node lhs, _pt_node rhs) noexcept
    {
        return lhs._ptr != rhs._ptr;
    }

private:
    explicit _pt_node(_detail::pt_node<Reader>* ptr) noexcept : _ptr(ptr) {}

    _detail::pt_node<Reader>* _ptr;

    friend parse_tree<Reader, TokenKind>;
    friend parse_tree_input_traits<_pt_node<Reader, TokenKind>>;
};

enum class traverse_event
{
    /// We're visiting a production node before all its children.
    enter,
    /// We're visiting a production node after all its children.
    exit,
    /// We're visiting a token.
    leaf,
};

template <typename Reader, typename TokenKind, typename MemoryResource>
class parse_tree<Reader, TokenKind, MemoryResource>::traverse_range
{
public:
    using event = traverse_event;

    struct _value_type
    {
        traverse_event   event;
        parse_tree::node node;
    };

    class iterator : public _detail::forward_iterator_base<iterator, _value_type, _value_type, void>
    {
    public:
        iterator() noexcept = default;

        _value_type deref() const noexcept
        {
            return {_ev, node(_cur)};
        }

        void increment() noexcept
        {
            if (_ev == traverse_event::enter)
            {
                auto child = _cur->as_production()->first_child();
                if (child)
                {
                    // We go to the first child next.
                    if (child->as_token())
                        _ev = traverse_event::leaf;
                    else
                        _ev = traverse_event::enter;

                    _cur = child;
                }
                else
                {
                    // Don't have children, exit.
                    _ev = traverse_event::exit;
                }
            }
            else
            {
                // We follow the next pointer.

                if (_cur->next_role() == _detail::pt_node<Reader>::role_parent)
                    // We go back to a production for the second time.
                    _ev = traverse_event::exit;
                else if (_cur->next_node()->as_production())
                    // We're having a production as sibling.
                    _ev = traverse_event::enter;
                else
                    // Token as sibling.
                    _ev = traverse_event::leaf;

                _cur = _cur->next_node();
            }
        }

        bool equal(iterator rhs) const noexcept
        {
            return _ev == rhs._ev && _cur == rhs._cur;
        }

    private:
        _detail::pt_node<Reader>* _cur = nullptr;
        traverse_event            _ev;

        friend traverse_range;
    };

    bool empty() const noexcept
    {
        return _begin == _end;
    }

    iterator begin() const noexcept
    {
        return _begin;
    }

    iterator end() const noexcept
    {
        return _end;
    }

private:
    traverse_range() noexcept = default;
    traverse_range(node n) noexcept
    {
        if (n.kind().is_token())
        {
            _begin._cur = n._ptr;
            _begin._ev  = traverse_event::leaf;

            _end = _detail::next(_begin);
        }
        else
        {
            _begin._cur = n._ptr;
            _begin._ev  = traverse_event::enter;

            _end._cur = n._ptr;
            _end._ev  = traverse_event::exit;
            ++_end; // half-open range
        }
    }

    iterator _begin, _end;

    friend parse_tree;
};
} // namespace lexy

#if LEXY_EXPERIMENTAL
namespace lexy
{
template <typename Reader, typename TokenKind>
struct parse_tree_input_traits<_pt_node<Reader, TokenKind>>
{
    using _node = _pt_node<Reader, TokenKind>;

    using char_encoding = typename Reader::encoding;

    static bool is_null(_node cur) noexcept
    {
        return cur._ptr == nullptr;
    }

    static _node null() noexcept
    {
        return _node(nullptr);
    }

    static _node first_child(_node cur) noexcept
    {
        LEXY_PRECONDITION(!is_null(cur));
        if (auto prod = cur._ptr->as_production())
            return _node(prod->first_child());
        else
            return _node(nullptr);
    }

    static _node sibling(_node cur) noexcept
    {
        LEXY_PRECONDITION(!is_null(cur));
        return cur._ptr->next_role() == _detail::pt_node<Reader>::role_sibling
                   ? _node(cur._ptr->next_node())
                   : _node(nullptr);
    }

    template <typename Kind>
    static bool has_kind(_node cur, const Kind& kind) noexcept
    {
        return !is_null(cur) && cur.kind() == kind;
    }

    using iterator = typename Reader::iterator;

    static iterator position_begin(_node cur) noexcept
    {
        LEXY_PRECONDITION(!is_null(cur));
        return cur.position();
    }
    static iterator position_end(_node cur) noexcept
    {
        LEXY_PRECONDITION(!is_null(cur));
        return cur.covering_lexeme().end();
    }

    static auto lexeme(_node cur) noexcept
    {
        LEXY_PRECONDITION(!is_null(cur));
        return cur.lexeme();
    }
};
} // namespace lexy
#endif

#endif // LEXY_PARSE_TREE_HPP_INCLUDED

