//
// SharedLibrary.h
//
// $Id: //poco/1.3/Foundation/include/Poco/SharedLibrary.h#1 $
//
// Library: Foundation
// Package: SharedLibrary
// Module:  SharedLibrary
//
// Definition of the SharedLibrary class.
//
// Copyright (c) 2004-2006, Applied Informatics Software Engineering GmbH.
// and Contributors.
//
// Permission is hereby granted, free of charge, to any person or organization
// obtaining a copy of the software and accompanying documentation covered by
// this license (the "Software") to use, reproduce, display, distribute,
// execute, and transmit the Software, and to prepare derivative works of the
// Software, and to permit third-parties to whom the Software is furnished to
// do so, all subject to the following:
//
// The copyright notices in the Software and this entire statement, including
// the above license grant, this restriction and the following disclaimer,
// must be included in all copies of the Software, in whole or in part, and
// all derivative works of the Software, unless such copies or derivative
// works are solely in the form of machine-executable object code generated by
// a source language processor.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
// SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
// FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//

#ifndef Foundation_SharedLibrary_INCLUDED
#define Foundation_SharedLibrary_INCLUDED

#include "platform.hpp"
#include <string>
#include <mutex>

namespace BT
{
class SharedLibrary
/// The SharedLibrary class dynamically
/// loads shared libraries at run-time.
{
public:
  enum Flags
  {
    /// On platforms that use dlopen(), use RTLD_GLOBAL. This is the default
    /// if no flags are given.
    ///
    /// This flag is ignored on platforms that do not use dlopen().
    SHLIB_GLOBAL = 1,

    /// On platforms that use dlopen(), use RTLD_LOCAL instead of RTLD_GLOBAL.
    ///
    /// Note that if this flag is specified, RTTI (including dynamic_cast and throw) will
    /// not work for types defined in the shared library with GCC and possibly other
    /// compilers as well. See http://gcc.gnu.org/faq.html#dso for more information.
    ///
    /// This flag is ignored on platforms that do not use dlopen().
    SHLIB_LOCAL = 2
  };

  /// Creates a SharedLibrary object.
  SharedLibrary();

  /// Creates a SharedLibrary object and loads a library
  /// from the given path, using the given flags.
  /// See the Flags enumeration for valid values.
  SharedLibrary(const std::string& path, int flags = 0);

  /// Destroys the SharedLibrary. The actual library
  /// remains loaded.
  virtual ~SharedLibrary() = default;

  /// Loads a shared library from the given path,
  /// using the given flags. See the Flags enumeration
  /// for valid values.
  /// Throws a LibraryAlreadyLoadedException if
  /// a library has already been loaded.
  /// Throws a LibraryLoadException if the library
  /// cannot be loaded.
  void load(const std::string& path, int flags = 0);

  /// Unloads a shared library.
  void unload();

  /// Returns true iff a library has been loaded.
  bool isLoaded() const;

  /// Returns true iff the loaded library contains
  /// a symbol with the given name.
  bool hasSymbol(const std::string& name);

  /// Returns the address of the symbol with
  /// the given name. For functions, this
  /// is the entry point of the function.
  /// Throws a NotFoundException if the symbol
  /// does not exist.
  void* getSymbol(const std::string& name);

  /// Returns the path of the library, as
  /// specified in a call to load() or the
  /// constructor.
  const std::string& getPath() const;

  /// Returns the platform-specific filename prefix
  /// for shared libraries.
  /// Most platforms would return "lib" as prefix, while
  /// on Cygwin, the "cyg" prefix will be returned.
  static std::string prefix();

  /// Returns the platform-specific filename suffix
  /// for shared libraries (including the period).
  /// In debug mode, the suffix also includes a
  /// "d" to specify the debug version of a library.
  static std::string suffix();

  /// Returns the platform-specific filename
  /// for shared libraries by prefixing and suffixing name
  /// with prefix() and suffix()
  static std::string getOSName(const std::string& name);

private:
  SharedLibrary(const SharedLibrary&);
  SharedLibrary& operator=(const SharedLibrary&);

  void* findSymbol(const std::string& name);

  std::string _path;
  void* _handle = nullptr;
  std::mutex _mutex;
};

}  // namespace BT

#endif  // Foundation_SharedLibrary_INCLUDED
