#include "behaviortree_cpp/exceptions.h"
#include "behaviortree_cpp/utils/shared_library.h"

#include <mutex>
#include <string>

#include <dlfcn.h>

namespace BT
{
SharedLibrary::SharedLibrary() = default;

void SharedLibrary::load(const std::string& path, int)
{
  const std::unique_lock<std::mutex> lock(_mutex);

  if(_handle != nullptr)
  {
    throw RuntimeError("Library already loaded: " + path);
  }

  _handle = dlopen(path.c_str(), RTLD_NOW | RTLD_GLOBAL);
  if(_handle == nullptr)
  {
    const char* err = dlerror();
    throw RuntimeError("Could not load library: " +
                       (err != nullptr ? std::string(err) : path));
  }
  _path = path;
}

void SharedLibrary::unload()
{
  const std::unique_lock<std::mutex> lock(_mutex);

  if(_handle != nullptr)
  {
    dlclose(_handle);
    _handle = nullptr;
  }
}

bool SharedLibrary::isLoaded() const
{
  return _handle != nullptr;
}

void* SharedLibrary::findSymbol(const std::string& name)
{
  const std::unique_lock<std::mutex> lock(_mutex);

  void* result = nullptr;
  if(_handle != nullptr)
  {
    result = dlsym(_handle, name.c_str());
  }
  return result;
}

const std::string& SharedLibrary::getPath() const
{
  return _path;
}

std::string SharedLibrary::prefix()
{
#if BT_OS == BT_OS_CYGWIN
  return "cyg";
#else
  return "lib";
#endif
}

std::string SharedLibrary::suffix()
{
#if BT_OS == BT_OS_MAC_OS_X
#if defined(_DEBUG) && !defined(CL_NO_SHARED_LIBRARY_DEBUG_SUFFIX)
  return "d.dylib";
#else
  return ".dylib";
#endif
#elif BT_OS == BT_OS_HPUX
#if defined(_DEBUG) && !defined(CL_NO_SHARED_LIBRARY_DEBUG_SUFFIX)
  return "d.sl";
#else
  return ".sl";
#endif
#elif BT_OS == BT_OS_CYGWIN
#if defined(_DEBUG) && !defined(CL_NO_SHARED_LIBRARY_DEBUG_SUFFIX)
  return "d.dll";
#else
  return ".dll";
#endif
#else
#if defined(_DEBUG) && !defined(CL_NO_SHARED_LIBRARY_DEBUG_SUFFIX)
  return "d.so";
#else
  return ".so";
#endif
#endif
}

}  // namespace BT
