#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include <gtest/gtest.h>

#include "filesystem/path.h"

class Environment : public testing::Environment
{
public:
  Environment(int argc, char** argv)
  {
    if(argc >= 1)
    {
      executable_path = filesystem::path(argv[0]).make_absolute();
    }
  }

  // the absolute path to the test executable
  filesystem::path executable_path;
};

// for accessing the environment within a test
extern Environment* environment;

#endif
