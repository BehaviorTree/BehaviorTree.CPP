BT++
====

A behavior tree library in `C++`.

SUPPORTING NODES
----------------
**Selector:** Selector nodes are used to find and execute the first child that does not fail. A Selector node will return immediately with a status code of success or running when one of its children returns success or running. The children are ticked in order of importance, from `left` to `right`.

**Sequence:** Sequence nodes are used to find and execute the first child that has not yet succeeded. A sequence node will return immediately with a status code of `failure` or `running` when one of its children returns failure or running. The children are ticked in order, from `left` to `right`.

**Parallel:** The parallel node ticks its children in parallel and returns success if `M ≤ N` children return success, it returns failure if `N − M + 1` children return failure, and it returns running otherwise.

**Decorator:** The decorator node manipulates the return status of its child according to the policy defined by the user (e.g. it inverts the success/failure status of the child). In this library the decorators implemented are the two common ones: *Decorator Retry* which retries the execution of a node if this fails; and *Decorator Negation* That inverts the Success/Failure outcome.

**Action:** An Action node performs an action, and returns Success if the action is completed, Failure if it can not be completed and Running if completion is under way.

**Condition:** A Condition node determines if a desired condition `c` has been met. Conditions are technically a subset of the Actions, but are given a separate category and graphical symbol to improve readability of the BT and emphasize the fact that they never return running and do not change any internal states/variables of the BT.

DEPENDENCIES
------------

Regarding visualization purposes:
* [Opengl](www.opengl.org)
* [Glut](www.opengl.org/resources/libraries/glut)

Regarding thread usage:
* [Boost](www.boost.org)

INSTALATION
-----------

The first step to use BT++ is to retrieve its source code. You can either download it 
here (https://github.com/miccol/Behavior-Tree) or clone the repository:

`$ cd /path/to/folder` <br/>
`$ git clone https://github.com/miccol/Behavior−Tree.git`

Once you have the repository. Compile the library:

`$ cd /path/to/folder/` <br/>
`$ mv Behavior−Tree behavior tree` <br/>
`$ cd behavior tree` <br/>
`$ mkdir ./build` <br/>
`$ cd build` <br/>
`$ cmake ..` <br/>
`$ make` <br/>

Check the installation by running a example sample.

`$ cd build/sample` <br/>
`$ ./test` <br/>

LICENSE
-------
The MIT License (MIT)

Copyright (c) 2015 Michele Colledanchise

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