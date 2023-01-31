/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file is part of zmqpp.
 * Copyright (c) 2011-2015 Contributors as noted in the AUTHORS file.
 */

/*
 *  Created on: 16 Aug 2011
 *      Author: Ben Gray (@benjamg)
 */

#include "exception.hpp"
#include "socket.hpp"
#include "poller.hpp"

#include <zmq.h>

namespace zmqpp
{

poller::poller()
	: _items()
	, _index()
	, _fdindex()
{

}

poller::~poller()
{
	_items.clear();
	_index.clear();
	_fdindex.clear();
}

void poller::add(socket& socket, short const event /* = POLL_IN */)
{
	zmq_pollitem_t const item { socket, 0, event, 0 };

	add(item);
}

void poller::add(raw_socket_t const descriptor, short const event /* = POLL_IN */)
{
	zmq_pollitem_t const item { nullptr, descriptor, event, 0 };

	add(item);
}

void poller::add(zmq_pollitem_t const& item)
{
	size_t index = _items.size();

	_items.push_back(item);
	if (nullptr == item.socket)
		_fdindex[item.fd] = index;
	else
		_index[item.socket] = index;
}

bool poller::has(socket_t const& socket)
{
	return _index.find(socket) != _index.end();
}

bool poller::has(raw_socket_t const descriptor)
{
	return _fdindex.find(descriptor) != _fdindex.end();
}

bool poller::has(zmq_pollitem_t const& item)
{
	if (nullptr != item.socket)
		return _index.find(item.socket) != _index.end();
	return _fdindex.find(item.fd) != _fdindex.end();
}

void poller::reindex(size_t const index)
{
	if ( nullptr != _items[index].socket )
	{
		auto found = _index.find( _items[index].socket );
		if (_index.end() == found) { throw exception("unable to reindex socket in poller"); }
		found->second = index;
	}
	else
	{
		auto found = _fdindex.find( _items[index].fd );
		if (_fdindex.end() == found) { throw exception("unable to reindex file descriptor in poller"); }
		found->second = index;
	}
}

void poller::remove(socket_t const& socket)
{
	zmq_pollitem_t const item{ socket, 0, 0, 0 };

	remove(item);
}

void poller::remove(raw_socket_t const descriptor)
{
	zmq_pollitem_t const item{ nullptr, descriptor, 0, 0 };

	remove(item);
}

void poller::remove(zmq_pollitem_t const& item)
{
    if (nullptr == item.socket)
      return remove(item.fd);
    
    auto found = _index.find(item.socket);
    if (_index.end() == found) { return; }
    
    if ( _items.size() - 1 == found->second )
      {
	_items.pop_back();
	_index.erase(found);
	return;
      }
    
    std::swap(_items[found->second], _items.back());
    _items.pop_back();
    
    auto index = found->second;
    _index.erase(found);
    
    reindex( index );
}

void poller::check_for(socket const& socket, short const event)
{
	auto found = _index.find(socket);
	if (_index.end() == found)
	{
		throw exception("this socket is not represented within this poller");
	}

	_items[found->second].events = event;
}

void poller::check_for(raw_socket_t const descriptor, short const event)
{
	auto found = _fdindex.find(descriptor);
	if (_fdindex.end() == found)
	{
		throw exception("this standard socket is not represented within this poller");
	}

	_items[found->second].events = event;
}

void poller::check_for(zmq_pollitem_t const& item, short const event)
{

        if (nullptr == item.socket)
            check_for(item.fd, event);
        else
        {
            auto found = _index.find(item.socket);
            if (_index.end() == found)
            {
                throw exception("this socket is not represented within this poller");
            }

            _items[found->second].events = event;

        }
}

bool poller::poll(long timeout /* = WAIT_FOREVER */)
{
	int result = zmq_poll(_items.data(), _items.size(), timeout);
	if (result < 0)
	{
		if(EINTR == zmq_errno())
		{
			return false;
		}

		throw zmq_internal_exception();
	}

	return (result > 0);
}

short poller::events(socket const& socket) const
{
	auto found = _index.find(socket);
	if (_index.end() == found)
	{
		throw exception("this socket is not represented within this poller");
	}

	return _items[found->second].revents;
}

short poller::events(raw_socket_t const descriptor) const
{
	auto found = _fdindex.find(descriptor);
	if (_fdindex.end() == found)
	{
		throw exception("this standard socket is not represented within this poller");
	}

	return _items[found->second].revents;
}

short poller::events(zmq_pollitem_t const& item) const
{
        if (nullptr == item.socket)
        {
            return events(item.fd);
        }
        auto found = _index.find(item.socket);
        if (_index.end() == found)
        {
            throw exception("this socket is not represented within this poller");
        }

        return _items[found->second].revents;
}

}
