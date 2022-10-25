![License MIT](https://img.shields.io/github/license/BehaviorTree/BehaviorTree.CPP?color=blue)
![Version](https://img.shields.io/badge/version-4.0-blue.svg)
[![cmake](https://github.com/BehaviorTree/BehaviorTree.CPP/actions/workflows/cmake.yml/badge.svg)](https://github.com/BehaviorTree/BehaviorTree.CPP/actions/workflows/cmake.yml)
[![ros1](https://github.com/BehaviorTree/BehaviorTree.CPP/workflows/ros1/badge.svg?branch=master)](https://github.com/BehaviorTree/BehaviorTree.CPP/actions?query=workflow%3Aros1)
[![ros2](https://github.com/BehaviorTree/BehaviorTree.CPP/workflows/ros2/badge.svg?branch=master)](https://github.com/BehaviorTree/BehaviorTree.CPP/actions?query=workflow%3Aros2)
[![LGTM Grade](https://img.shields.io/lgtm/grade/cpp/github/BehaviorTree/BehaviorTree.CPP)](https://lgtm.com/projects/g/BehaviorTree/BehaviorTree.CPP/context:cpp)
![Discourse topics](https://img.shields.io/discourse/topics?server=https%3A%2F%2Fdiscourse.behaviortree.dev)

# BehaviorTree.CPP 4.0

<p align="center"><img width=350 src="animated.svg"></p>

This  __C++ 17__ library provides a framework to create BehaviorTrees.
It was designed to be flexible, easy to use, reactive and fast.

Even if our main use-case is __robotics__, you can use this library to build
__AI for games__, or to replace Finite State Machines.

There are few features that make __BehaviorTree.CPP__ unique, when compared to other implementations:

- It makes __asynchronous Actions__, i.e. non-blocking, a first-class citizen.

- You can build __reactive__ behaviors that execute multiple Actions concurrently.

- Trees are defined using a Domain Specific __scripting language__ (based on XML), and can be loaded at run-time; in other words, even if written in C++, the morphology of the Trees is _not_ hard-coded.

- You can statically link your custom TreeNodes or convert them into __plugins__
and load them at run-time.

- It provides a type-safe and flexible mechanism to do __Dataflow__ between
  Nodes of the Tree.

- It includes a __logging/profiling__ infrastructure that allows the user 
to visualize, record, replay and analyze state transitions.

## Documentation and Community

You can learn about the main concepts, the API and the tutorials here: https://www.behaviortree.dev/

If the documentation doesn't answer your questions and/or you want to
connect with the other **BT.CPP** users, visit https://discourse.behaviortree.dev/

## Previous version

Version 3.8 of the software can be found in the branch 
[v3.8](https://github.com/BehaviorTree/BehaviorTree.CPP/tree/v3.8).

That branch might receive bug fixes, but the new features will be implemented
only in the master branch.

## Commercial support

Are you using BT.CPP in your commercial product and you need technical support / consulting?
You can contact the main author dfaconti@aurynrobotics.com to discuss your use case and needs.

# Design principles

The main goal of this project is to create a Behavior Tree implementation
that uses the principles of Model Driven Development to separate the role 
of the __Component Developer__ from the __Behavior Designer__.

In practice, this means that:

- Custom TreeNodes must be reusable building blocks. 
 You should be able to implement them once and reuse them to build many behaviors.

- To build a Behavior Tree out of TreeNodes, the Behavior Designer must 
not need to read nor to modify the C++ source code..

- Complex Behaviours must be composable using Subtrees. 

# GUI Editor

Editing a BehaviorTree is as simple as editing a XML file in your favourite text editor.

If you are looking for a more fancy graphical user interface (and I know you do) check 
[Groot](https://github.com/BehaviorTree/Groot) out.

![Groot screenshot](docs/groot-screenshot.png)

# How to compile (Ubuntu)

Please note that **Ubuntu 18.04 is not supported anymore in version 4.X**. Ubuntu 20.04 or later is required.

First, install the following dependencies (optional, but recommended):

     sudo apt-get install libzmq3-dev libboost-dev
     
To compile and install the library, from the BehaviorTree.CPP folder, execute:

     mkdir build; cd build
     cmake ..
     make
     sudo make install

If you want to use BT.CPP in your application a typical **CMakeLists.txt** file 
will look like this:

```cmake
cmake_minimum_required(VERSION 3.10.2)
project(hello_BT)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
find_package(behaviortree_cpp)

add_executable(${PROJECT_NAME} "hello_BT.cpp")
target_link_libraries(${PROJECT_NAME} BT::behaviortree_cpp)
```

# License

The MIT License (MIT)

Copyright (c) 2014-2018 Michele Colledanchise

Copyright (c) 2018-2019 Davide Faconti, Eurecat

Copyright (c) 2019-2022 Davide Faconti

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
