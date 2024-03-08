#pragma once

#include <string>
#include <cstring>
#include <stdexcept>
#include <limits>
#include <cstdint>
#include <string_view>

namespace SafeAny
{

// Read only version of String that has size 16 bytes and can store
// in-place strings with size up to 15 bytes.

// Inspired by https://github.com/elliotgoodrich/SSO-23

class SimpleString
{
public:
  SimpleString(const std::string& str) : SimpleString(str.data(), str.size())
  {}

  SimpleString(const std::string_view& str) : SimpleString(str.data(), str.size())
  {}

  SimpleString(const SimpleString& other) : SimpleString(other.data(), other.size())
  {}

  SimpleString& operator=(const SimpleString& other)
  {
    this->~SimpleString();
    createImpl(other.data(), other.size());
    return *this;
  }

  SimpleString(SimpleString&& other) : SimpleString(nullptr, 0)
  {
    std::swap(_storage, other._storage);
  }

  SimpleString& operator=(SimpleString&& other)
  {
    this->~SimpleString();

    std::swap(_storage, other._storage);
    return *this;
  }

  SimpleString(const char* input_data) : SimpleString(input_data, strlen(input_data))
  {}

  SimpleString(const char* input_data, std::size_t size)
  {
    createImpl(input_data, size);
  }

  ~SimpleString()
  {
    if(!isSOO())
    {
      delete[] _storage.str.data;
    }
    _storage.soo.capacity_left = CAPACITY;
  }

  std::string toStdString() const
  {
    return size() > 0 ? std::string(data(), size()) : std::string();
  }
  std::string_view toStdStringView() const
  {
    return size() > 0 ? std::string_view(data(), size()) : std::string_view();
  }

  const char* data() const
  {
    if(isSOO())
    {
      return _storage.soo.data;
    }
    else
    {
      return _storage.str.data;
    }
  }

  std::size_t size() const
  {
    if(isSOO())
    {
      return CAPACITY - _storage.soo.capacity_left;
    }
    else
    {
      return _storage.str.size & LONG_MASK;
    }
  }

  bool operator==(const SimpleString& other) const
  {
    size_t N = size();
    return other.size() == N && std::strncmp(data(), other.data(), N) == 0;
  }

  bool operator!=(const SimpleString& other) const
  {
    size_t N = size();
    return other.size() != N || std::strncmp(data(), other.data(), N) != 0;
  }

  bool operator<=(const SimpleString& other) const
  {
    return std::strcmp(data(), other.data()) <= 0;
  }

  bool operator>=(const SimpleString& other) const
  {
    return std::strcmp(data(), other.data()) >= 0;
  }

  bool operator<(const SimpleString& other) const
  {
    return std::strcmp(data(), other.data()) < 0;
  }

  bool operator>(const SimpleString& other) const
  {
    return std::strcmp(data(), other.data()) > 0;
  }

  bool isSOO() const
  {
    return !(_storage.soo.capacity_left & IS_LONG_BIT);
  }

private:
  struct String
  {
    char* data;
    std::size_t size;
  };

  constexpr static std::size_t CAPACITY = 15;  // sizeof(String) - 1);
  constexpr static std::size_t IS_LONG_BIT = 1 << 7;
  constexpr static std::size_t LONG_MASK = (~std::size_t(0)) >> 1;
  constexpr static std::size_t MAX_SIZE = 100UL * 1024UL * 1024UL;

  union
  {
    String str;

    struct SOO
    {
      char data[CAPACITY];
      uint8_t capacity_left;
    } soo;
  } _storage;

private:
  void createImpl(const char* input_data, std::size_t size)
  {
    if(size > MAX_SIZE)
    {
      throw std::invalid_argument("size too large for a simple string");
    }

    if(size > CAPACITY)
    {
      _storage.str.size = size;
      _storage.soo.capacity_left = IS_LONG_BIT;
      _storage.str.data = new char[size + 1];
      std::memcpy(_storage.str.data, input_data, size);
      _storage.str.data[size] = '\0';
    }
    else
    {
      _storage.soo.capacity_left = uint8_t(CAPACITY - size);
      if(size > 0)
      {
        std::memcpy(_storage.soo.data, input_data, size);
      }
      if(size < CAPACITY)
      {
        _storage.soo.data[size] = '\0';
      }
    }
  }
};

}  // namespace SafeAny
