#include <string>
#include <mutex>
#include <Windows.h>
#include "behaviortree_cpp/utils/shared_library.h"
#include "behaviortree_cpp/exceptions.h"

namespace BT
{
SharedLibrary::SharedLibrary()
{
  _handle = nullptr;
}

void SharedLibrary::load(const std::string& path, int)
{
  std::unique_lock<std::mutex> lock(_mutex);

  _handle = LoadLibrary(path.c_str());
  if(!_handle)
  {
    throw RuntimeError("Could not load library: " + path);
  }
  _path = path;
}

void SharedLibrary::unload()
{
  std::unique_lock<std::mutex> lock(_mutex);

  if(_handle)
  {
    FreeLibrary((HMODULE)_handle);
    _handle = 0;
  }
  _path.clear();
}

bool SharedLibrary::isLoaded() const
{
  return _handle != nullptr;
}

void* SharedLibrary::findSymbol(const std::string& name)
{
  std::unique_lock<std::mutex> lock(_mutex);

  if(_handle)
  {
#if defined(_WIN32_WCE)
    std::wstring uname;
    UnicodeConverter::toUTF16(name, uname);
    return (void*)GetProcAddressW((HMODULE)_handle, uname.c_str());
#else
    return (void*)GetProcAddress((HMODULE)_handle, name.c_str());
#endif
  }

  return 0;
}

const std::string& SharedLibrary::getPath() const
{
  return _path;
}

std::string SharedLibrary::prefix()
{
  return "";
}

std::string SharedLibrary::suffix()
{
#if defined(_DEBUG)
  return "d.dll";
#else
  return ".dll";
#endif
}

}  // namespace BT
