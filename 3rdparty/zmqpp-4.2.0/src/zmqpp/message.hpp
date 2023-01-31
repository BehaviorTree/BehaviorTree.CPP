/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file is part of zmqpp.
 * Copyright (c) 2011-2015 Contributors as noted in the AUTHORS file.
 */

/**
 * \file
 *
 * \date   9 Aug 2011
 * \author Ben Gray (\@benjamg)
 */

#ifndef ZMQPP_MESSAGE_HPP_
#define ZMQPP_MESSAGE_HPP_

#include <cassert>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>
#include <utility>

#include <zmq.h>

#include "compatibility.hpp"
#include "frame.hpp"
#include "signal.hpp"

namespace zmqpp
{

/**
 * \brief a zmq message with optional multipart support
 *
 * A zmq message is made up of one or more parts which are sent together to
 * the target endpoints. zmq guarantees either the whole message or none
 * of the message will be delivered.
 */
class ZMQPP_EXPORT message
{
public:
	/**
	 * \brief callback to release user allocated data.
	 *
	 * The release function will be called on any void* moved part.
	 * It must be thread safe to the extent that the callback may occur on
	 * one of the context threads.
	 *
	 * The function called will be passed a single variable which is the
	 * pointer to the memory allocated.
	 */
	typedef std::function<void (void*)> release_function;

	message();
	~message();

    template <typename T, typename ...Args>
    message(T const &part, Args &&...args)
        : message()
    {
        add(part, std::forward<Args>(args)...);
    }

	size_t parts() const;
	size_t size(size_t const part) const;
	std::string get(size_t const part) const;

	void get(int8_t& integer, size_t const part) const;
	void get(int16_t& integer, size_t const part) const;
	void get(int32_t& integer, size_t const part) const;
	void get(int64_t& integer, size_t const part) const;	
	void get(signal& sig, size_t const part) const;

	void get(uint8_t& unsigned_integer, size_t const part) const;
	void get(uint16_t& unsigned_integer, size_t const part) const;
	void get(uint32_t& unsigned_integer, size_t const part) const;
	void get(uint64_t& unsigned_integer, size_t const part) const;

	void get(float& floating_point, size_t const part) const;
	void get(double& double_precision, size_t const part) const;
	void get(bool& boolean, size_t const part) const;

	void get(std::string& string, size_t const part) const;

	// Warn: If a pointer type is requested the message (well zmq) still 'owns'
	// the data and will release it when the message object is freed.
	template<typename Type>
	Type get(size_t const part) const
	{
		Type value;
		get(value, part);
		return value;
	}

    template<int part=0, typename T, typename ...Args>
    void extract(T &nextpart, Args &...args) const
    {
        assert(part < parts());
        get(nextpart,part);
        extract<part+1>(args...);
    }

    template<int part=0, typename T>
    void extract(T &nextpart) const
    {
        assert(part < parts());
        get(nextpart,part);
    }

	// Raw get data operations, useful with data structures more than anything else
	// Warn: The message (well zmq) still 'owns' the data and will release it
	// when the message object is freed.
	template<typename Type>
	void get(Type*& value, size_t const part) const
	{
		value = static_cast<Type*>(raw_data(part));
	}

	// Warn: The message (well zmq) still 'owns' the data and will release it
	// when the message object is freed.
	template<typename Type>
	void get(Type** value, size_t const part) const
	{
		*value = static_cast<Type*>(raw_data(part));
	}

	// Move operators will take ownership of message parts without copying
	void move(void* part, size_t const size, release_function const& release);

	// Raw move data operation, useful with data structures more than anything else
	template<typename Object>
	void move(Object *part)
	{
		move(part, sizeof(Object), &deleter_callback<Object>);
	}

	// Copy operators will take copies of any data
	template<typename Type, typename ...Args>
	void add(Type const& part, Args &&...args)
	{
		*this << part;
		add(std::forward<Args>(args)...);
	}

	template<typename Type>
	void add(Type const part)
	{
		*this << part;
	}

	// Copy operators will take copies of any data with a given size
	template<typename Type>
	void add_raw(Type *part, size_t const data_size)
	{
		_parts.push_back( frame( part, data_size ) );
	}

	// Use exact data past, neither zmqpp nor 0mq will copy, alter or delete
	// this data. It must remain as valid for at least the lifetime of the
	// 0mq message, recommended only with const data.
	template<typename Type>
	ZMQPP_DEPRECATED("Use add_nocopy() or add_nocopy_const() instead.")
	void add_const(Type *part, size_t const data_size)
	{
		_parts.push_back( frame( part, data_size, nullptr, nullptr ) );
	}

	/**
	 * Add a no-copy frame.
	 *
	 * This means that neither zmqpp nor libzmq will make a copy of the
	 * data. The pointed-to data must remain valid for the lifetime of
	 * the underlying zmq_msg_t. Note that you cannot always know about
	 * this lifetime, so be careful.
	 *
	 * @param part The pointed-to data that will be send in the message.
	 * @param data_size The number of byte pointed-to by "part".
	 * @param ffn The free function called by libzmq when it doesn't need
	 * your buffer anymore. It defaults to nullptr, meaning your data
	 * will not be freed.
	 * @param hint A hint to help your free function do its job.
	 *
	 * @note This is similar to what `move()` does. While `move()` provide a safe
	 * (wrt to type) deleter (at the cost of 1 memory allocation) add_nocopy
	 * let you pass the low-level callback that libzmq will invoke.
	 *
	 * @note The free function must be thread-safe as it can be invoke from
	 * any libzmq's context threads.
	 *
	 * @see add_nocopy_const
	 * @see move
	 */
	template<typename Type>
	void add_nocopy(Type *part, size_t const data_size,
					zmq_free_fn *ffn = nullptr, void *hint = nullptr)
	{
		static_assert(!std::is_const<Type>::value,
                      "Data part must not be const. Use add_nocopy_const() instead (and read its documentation)");
		_parts.push_back(frame(part, data_size, ffn, hint));
	}

	/**
	 * Add a no-copy frame where pointed-to data are const.
	 *
	 * This means that neither zmqpp nor libzmq will make a copy of the
	 * data. The pointed-to data must remain valid for the lifetime of
	 * the underlying zmq_msg_t. Note that you cannot always know about
	 * this lifetime, so be careful.
	 *
	 * @warning About constness: The library will cast away constness from
	 * your pointer. However, it promises that both libzmq and zmqpp will
	 * not alter the pointed-to data. *YOU* must however be careful: zmqpp or libzmq
	 * will happily return a non-const pointer to your data. It's your responsibility
	 * to not modify it.
	 *
	 * @param part The pointed-to data that will be send in the message.
	 * @param data_size The number of byte pointed-to by "part".
	 * @param ffn The free function called by libzmq when it doesn't need
	 * your buffer anymore. It defaults to nullptr, meaning your data
	 * will not be freed.
	 * @param hint A hint to help your free function do its job.
	 *
	 * @note The free function must be thread-safe as it can be invoke from
	 * any libzmq's context threads.
	 *
	 * @see add_nocopy
	 */
	template<typename Type>
	void add_nocopy_const(const Type *part, size_t const data_size,
				   zmq_free_fn *ffn = nullptr, void *hint = nullptr)
	{
		add_nocopy(const_cast<typename std::remove_const<Type *>::type>(part),
				   data_size, ffn, hint);
	}

	// Stream reader style
	void reset_read_cursor();

	template<typename Type>
	message& operator>>(Type& value)
	{
		get(value, _read_cursor++);
		return *this;
	}

	// Stream writer style - these all use copy styles
	message& operator<<(int8_t const integer);
	message& operator<<(int16_t const integer);
	message& operator<<(int32_t const integer);
	message& operator<<(int64_t const integer);
	message& operator<<(signal const sig);

	message& operator<<(uint8_t const unsigned_integer);
	message& operator<<(uint16_t const unsigned_integer);
	message& operator<<(uint32_t const unsigned_integer);
	message& operator<<(uint64_t const unsigned_integer);

	message& operator<<(float const floating_point);
	message& operator<<(double const double_precision);
	message& operator<<(bool const boolean);

	message& operator<<(char const* c_string);
	message& operator<<(std::string const& string);

	// Queue manipulation
	void push_front(void const* part, size_t const size);

	// TODO: unify conversion of types with the stream operators
	void push_front(int8_t const integer);
	void push_front(int16_t const integer);
	void push_front(int32_t const integer);
	void push_front(int64_t const integer);
	void push_front(signal const sig);

	void push_front(uint8_t const unsigned_integer);
	void push_front(uint16_t const unsigned_integer);
	void push_front(uint32_t const unsigned_integer);
	void push_front(uint64_t const unsigned_integer);

	void push_front(float const floating_point);
	void push_front(double const double_precision);
	void push_front(bool const boolean);

	void push_front(char const* c_string);
	void push_front(std::string const& string);

	void pop_front();

	void push_back(void const* part, size_t const data_size)
	{
		add_raw( part, data_size );
	}

	template<typename Type>
	void push_back(Type const part)
	{
		*this << part;
	}

	void pop_back();

	void remove(size_t const part);

	// Move supporting
	message(message&& source) NOEXCEPT;
	message& operator=(message&& source) NOEXCEPT;

	// Copy support
	message copy() const;
	void copy(message const& source);

	// Used for internal tracking
	void sent(size_t const part);

	// Access to raw zmq details
	void const* raw_data(size_t const part = 0) const;
	zmq_msg_t& raw_msg(size_t const part = 0);
	zmq_msg_t& raw_new_msg();
	zmq_msg_t& raw_new_msg(size_t const reserve_data_size);
	
	/**
	 * Check if the message is a signal.
	 * If the message has 1 part, has the correct size and if the 7 first bytes match
	 * the signal header we consider the message a signal.
	 * @return true if the message is a signal, false otherwise
	 */
	bool is_signal() const;

	/**
	 * Gets the read cursor. For using get_raw() with stream-style reading.
	 */
	size_t read_cursor() const NOEXCEPT { return _read_cursor; }

	/**
	 * Gets the remaining number of parts in the message.
	 */
	size_t remaining() const NOEXCEPT { return  _parts.size() - _read_cursor; }

	/**
	 * Moves the read cursor to the next element.
	 * @return the new read_cursor
	 */
	size_t next() NOEXCEPT { return ++_read_cursor; }


#if (ZMQ_VERSION_MAJOR == 4 && ZMQ_VERSION_MINOR >= 1)
	/**
	* Attemps to retrieve a metadata property from a message.
	* The underlying call is `zmq_msg_gets()`.
	*
	* @note The message MUST have at least one frame, otherwise this wont work.
	*/
	bool get_property(const std::string &property, std::string &out);
#endif

private:
	typedef std::vector<frame> parts_type;
	parts_type _parts;
	size_t _read_cursor;

	// Disable implicit copy support, code must request a copy to clone
	message(message const&) NOEXCEPT ZMQPP_EXPLICITLY_DELETED;
	message& operator=(message const&) NOEXCEPT ZMQPP_EXPLICITLY_DELETED;

	static void release_callback(void* data, void* hint);

	template<typename Object>
	static void deleter_callback(void* data)
	{
		delete static_cast<Object*>(data);
	}
};

}

#endif /* ZMQPP_MESSAGE_HPP_ */
