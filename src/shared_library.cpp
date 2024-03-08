#include "behaviortree_cpp/utils/shared_library.h"
#include "behaviortree_cpp/exceptions.h"

BT::SharedLibrary::SharedLibrary(const std::string& path, int flags)
{
  load(path, flags);
}

void* BT::SharedLibrary::getSymbol(const std::string& name)
{
  void* result = findSymbol(name);
  if(result)
    return result;
  else
    throw RuntimeError("[SharedLibrary::getSymbol]: can't find symbol ", name);
}

bool BT::SharedLibrary::hasSymbol(const std::string& name)
{
  return findSymbol(name) != nullptr;
}

std::string BT::SharedLibrary::getOSName(const std::string& name)
{
  return prefix() + name + suffix();
}
