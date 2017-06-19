<img width="100" alt="portfolio_view" src="http://michelecolledanchise.com/BTPPLOGO.png"> BT++ ![License MIT](https://img.shields.io/dub/l/vibe-d.svg)
====
![Version](https://img.shields.io/badge/version-v1.4-green.svg) <br/> 
A behavior tree library in `C++`.

REFERENCE
------------
Please refer to the following paper when using the library:

**How Behavior Trees Modularize Hybrid Control Systems and Generalize Sequential Behavior Compositions, the Subsumption Architecture, and Decision Trees.** Michele Colledanchise and Petter Ogren. IEEE Transaction on Robotics 2017.

bibtex entry:

`@ARTICLE{TRO17Colledanchise,` <br/>
`author={M. Colledanchise and P. Ögren},` <br/>
`journal={IEEE Transactions on Robotics},` <br/>
`title={{How Behavior Trees Modularize Hybrid Control Systems and Generalize Sequential Behavior Compositions, the Subsumption Architecture, and Decision Trees}},` <br/> 
`year={2017},` <br/>
`volume={33},` <br/>
`number={2},` <br/>
`pages={372-389},` <br/>
`keywords={Computer architecture;Decision trees;High definition video;Robot control;Switches;Behavior trees (BTs);decision trees;finite state machines (FSMs);hybrid dynamical systems (HDSs);modularity;sequential behavior compositions;subsumption architecture}, ` <br/>
`doi={10.1109/TRO.2016.2633567},` <br/>
`ISSN={1552-3098},` <br/>
`month={April},}`<br/>


DEPENDENCIES
------------

Regarding visualization purposes:
* [OpenGL](https://www.opengl.org/)
* [Glut](https://www.opengl.org/resources/libraries/glut/)

Regarding tests:
* [GTests](https://github.com/google/googletest)

BT NODES SUPPORT
----------------
**Fallback:** Fallback nodes are used to find and execute the first child that does not fail. A Selector node will return immediately with a status code of success or running when one of its children returns success or running. The children are ticked in order of importance, from `left` to `right`.

**Sequence:** Sequence nodes are used to find and execute the first child that has not yet succeeded. A sequence node will return immediately with a status code of `failure` or `running` when one of its children returns failure or running. The children are ticked in order, from `left` to `right`.

**Parallel:** The parallel node ticks its children in parallel and returns success if `M ≤ N` children return success, it returns failure if `N − M + 1` children return failure, and it returns running otherwise.

**Decorator:** The decorator node manipulates the return status of its child according to the policy defined by the user (e.g. it inverts the success/failure status of the child). In this library the decorators implemented are the two common ones: *Decorator Retry* which retries the execution of a node if this fails; and *Decorator Negation* That inverts the Success/Failure outcome.

**Action:** An Action node performs an action, and returns Success if the action is completed, Failure if it can not be completed and Running if completion is under way.

**Condition:** A Condition node determines if a desired condition `c` has been met. Conditions are technically a subset of the Actions, but are given a separate category and graphical symbol to improve readability of the BT and emphasize the fact that they never return running and do not change any internal states/variables of the BT.

A user manual is available in the project folder ([BTppUserManual.pdf](https://github.com/miccol/Behavior-Tree/blob/master/BTppUserManual.pdf)).

SETUP
-----------

The first step to use BT++ is to retrieve its source code. You can either download it 
here (https://github.com/miccol/Behavior-Tree) or clone the repository:

`$ cd /path/to/folder` <br/>
`$ git clone https://github.com/miccol/Behavior−Tree.git`

Once you have the repository, compile the library:

`$ cd /path/to/folder/` <br/>
`$ mkdir ./build` <br/>
`$ cd build` <br/>
`$ cmake ..` <br/>
`$ make` <br/>

Check the installation by running a sample example.

`$ cd /path/to/folder/` <br/>
`$ cd build/sample` <br/>
`$ ./btpp_example` <br/>

Note that the local installation generates the shared library in `/path/to/folder/build/lib`.

INSTALL THE LIBRARY SYSTEM-WIDE (tested on Ubuntu 14.04 only)
-------------------------------

If you would like to install the library system-wide, then run:

`$ cd /path/to/folder/` <br/>
`$ cd build` <br/>
`$ sudo make install` <br/>

On Ubuntu 14.04, this will install the shared library (libbtpp.so) in `/usr/local/lib`. <br/>
The repository [my-behavior-tree-project](https://github.com/miccol/my-behavior-tree-project) shows an example on how to use the library once system-wide installed.


CREATE YOUR OWN ACTION NODE
------
1) Implement your action node class extending the abstract class `BT::ActionNode`.
2) Implement the method `BT::ReturnStatus Tick()` with the code you want to execute while the action is running. Use the method `is_halted()` to check if the action has been prempted. When the execution of your action finished, return `BT::SUCCESS` or `BT::FAILURE` accordingly.  
3) Implement the method `void Halt()` with the code you want to execute when the action gets preempted (halted).
See the file `src/example.cpp` for an example.

CREATE YOUR OWN CONDITION NODE
------
1) Implement your condition node class extending the abstract class `BT::ConditionNode`.
2) Implement the method `BT::ReturnStatus Tick()` with the code you want to execute to check the condition. Return `BT::SUCCESS` or `BT::FAILURE` accordingly.  
See the file `src/example.cpp` for an example.

LICENSE
-------
The MIT License (MIT)

Copyright (c) 2014-2017 Michele Colledanchise

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
