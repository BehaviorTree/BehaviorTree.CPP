^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Changelog for package behaviortree_cpp
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

3.6.0 (2021-11-10)
------------------
* Build samples independently of examples (`#315 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/315>`_)
* Fix dependency in package.xml (`#313 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/313>`_)
* Fix doc statement (`#309 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/309>`_)
  Fix sentence
* Fix references to RetryUntilSuccesful (`#308 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/308>`_)
  * Fix github action
  * Fix references to RetryUntilSuccesful
* added subclass RetryNodeTypo (`#295 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/295>`_)
  Co-authored-by: Subaru Arai <SubaruArai@local>
* Fix github action (`#302 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/302>`_)
* Minor spelling correction (`#305 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/305>`_)
  Corrected `the_aswer` to `the_answer`
* Update FallbackNode.md (`#306 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/306>`_)
  typo correction.
* Add signal handler for Windows (`#307 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/307>`_)
* fix
* file renamed and documentation fixed
* Update documentation for reactive sequence (`#286 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/286>`_)
* Update FallbackNode.md (`#287 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/287>`_)
  Fix the pseudocode in the documentation of 'Reactive Fallback' according to its source code.
* Update fallback documentation to V3 (`#288 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/288>`_)
  * Update FallbackNode.md description to V3
  * Fix typo
* Use pedantic for non MSVC builds (`#289 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/289>`_)
* Merge branch 'master' of https://github.com/BehaviorTree/BehaviorTree.CPP
* updated to latest flatbuffers
* Update README.md
* Fix issue `#273 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/273>`_
* remove potential crash when an unfinished tree throws an exception
* remove appveyor
* Merge branch 'git_actions'
* Fixes for compilation on windows. (`#248 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/248>`_)
  * Fix for detecting ZeroMQ on windows
  Naming convention is a bit different for ZeroMQ, specifically on Windows with vcpkg. While ZMQ and ZeroMQ are valid on linux, the ZMQ naming convention only works on linux.
  * Compilation on windows not working with /WX
  * Macro collision on Windows
  On windows, the macros defined in the abstract logger collides with other in windows.h. Made them lowercase to avoid collision
* Remove native support for Conan (`#280 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/280>`_)
* add github workflow
* Registered missing dummy nodes for examples (`#275 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/275>`_)
  * Added CheckTemperature dummy node
  * Added SayHello dummy node
* add zmq.hpp in 3rdparty dirfectory
* add test
* fix some warnings
* Fix bug on halt of delay node (`#272 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/272>`_)
  - When DelayNode is halted and ticked again, it always returned FAILURE since the state of DelayNode was not properly reset.
  - This commit fixes unexpected behavior of DelayNode when it is halted.
  Co-authored-by: Jinwoo Choi <jinwoos.choi@samsung.com>
* Clear all of blackboard's content (`#269 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/269>`_)
* Added printTreeRecursively overload with ostream parameter (`#264 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/264>`_)
  * Added overload to printTreeRecursively
  * Changed include to iosfwd
  * Added test to verify function writes to stream
  * Added call to overload without stream parameter
  * Fixed conversion error
  * Removed overload in favor of default argument
* Fix typo (`#260 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/260>`_)
  Co-authored-by: Francesco Vigni <francesco.vigni@sttech.de>
* Update README.md
* abstract_logger.h: fixed a typo (`#257 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/257>`_)
* Contributors: Adam Sasine, Affonso, Guilherme, Akash, Billy, Cong Liu, Daisuke Nishimatsu, Davide Faconti, Francesco Vigni, Heben, Jake Keller, Per-Arne Andersen, Ross Weir, Steve Macenski, SubaruArai, Taehyeon, Uilian Ries, Yadu, Yuwei Liang, matthews-jca, swarajpeppermint

3.5.6 (2021-02-03)
------------------
* fix issue `#227 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/227>`_
* fix issue `#256 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/256>`_
* Merge branch 'master' of https://github.com/BehaviorTree/BehaviorTree.CPP
* fix issue `#250 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/250>`_
* Fixed typos on SequenceNode.md (`#254 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/254>`_)
* Contributors: Davide Faconti, LucasNolasco

3.5.5 (2021-01-27)
------------------
* fix issue `#251 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/251>`_
* Contributors: Davide Faconti

3.5.4 (2020-12-10)
------------------
* Update bt_factory.cpp (`#245 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/245>`_)
* Use the latest version of zmq.hpp
* Improved switching BTs with active Groot monitoring (ZMQ logger destruction) (`#244 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/244>`_)
  * Skip 100ms (max) wait for detached thread
  * add {} to single line if statements
* Update retry_node.cpp
* fix
* fix issue `#230 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/230>`_
* Contributors: Davide Faconti, Florian Gramß, amangiat88

3.5.3 (2020-09-10)
------------------
* fix issue `#228 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/228>`_ . Retry and Repeat node need to halt the child
* better tutorial
* Contributors: Davide Faconti

3.5.2 (2020-09-02)
------------------
* fix warning and follow coding standard
* docs: Small changes to tutorial 02 (`#225 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/225>`_)
  Co-authored-by: Valerio Magnago <valerio.magnago@fraunhofer.it>
* Merge branch 'master' of https://github.com/BehaviorTree/BehaviorTree.CPP
* tutorial 1 fixed
* decreasing warning level to fix issue `#220 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/220>`_
* fix compilation
* Allow BT factory to define clock source for TimerQueue/TimerNode (`#215 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/215>`_)
  * Allow BT factory to define clock source for TimerQueue/TimerNode
  * Fix unit tests
  Co-authored-by: Cam Fulton <cfulton@symbotic.com>
  Co-authored-by: Davide Faconti <davide.faconti@gmail.com>
* Added delay node and wait for enter keypress node (`#182 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/182>`_)
  * Added delay node and wait for enter press node
  * Fixed unsigned int to int conversion bug
  * Added a new timer to keep a track of delay timeout and return RUNNING in the meanwhile
  * Removed wait for keypress node
  * Review changes suggested by gramss
  Co-authored-by: Indraneel Patil <indraneel.p@greyorange.com>
* Update SequenceNode.md (`#211 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/211>`_)
* add failure threshold to parallel node with tests (`#216 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/216>`_)
* Update tutorial_05_subtrees.md
  I believe that the API has been updated. Reflecting the same in this tutorial.
* Contributors: Aayush Naik, Davide Faconti, Indraneel Patil, Renan Salles, Valerio Magnago, Wuqiqi123, fultoncjb

3.5.1 (2020-06-11)
------------------
* trying to fix compilation in eloquent  Minor fix on line 19
* Update README.md
* more badges
* readme updated
* fix ros2 compilation?
* move to github actions
* replace dot by zero in boost version (`#197 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/197>`_)
* Always use nonstd::string_view for binary compatibility (fix issue `#200 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/200>`_)
* Adding ForceRunningNode Decorator (`#192 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/192>`_)
* updated doc
* Add XML parsing support for custom Control Nodes (`#194 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/194>`_)
* Fix typo
* [Windows] Compare `std::type_info` objects to check type. (`#181 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/181>`_)
* Fix pseudocode for ReactiveFallback. (`#191 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/191>`_)
* Contributors: Aayush Naik, Darío Hereñú, Davide Faconti, Francisco Martín Rico, G.Doisy, Sarathkrishnan Ramesh, Sean Yen, Ting Chang

3.5.0 (2020-05-14)
------------------
* added IfThenElse and  WhileDoElse
* issue `#190 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/190>`_
* unit test added
* reverting to a better solution
* RemappedSubTree added
* Fix issue `#188 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/188>`_
* added function const std::string& key (issue `#183 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/183>`_)
* Contributors: Davide Faconti, daf@blue-ocean-robotics.com

* added IfThenElse and  WhileDoElse
* issue `#190 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/190>`_
* unit test added
* reverting to a better solution
* RemappedSubTree added
* Fix issue `#188 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/188>`_
* added function const std::string& key (issue `#183 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/183>`_)
* Contributors: Davide Faconti, daf@blue-ocean-robotics.com

3.1.1 (2019-11-10)
------------------
* fix samples compilation (hopefully)
* Contributors: Davide Faconti

3.1.0 (2019-10-30)
------------------
* Error message corrected
* fix windows and mingw compilation (?)
* Merge pull request `#70 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/70>`_ from Masadow/patch-3
  Added 32bits compilation configuration for msvc
* make Tree non copyable
* fix `#114 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/114>`_
* Merge branch 'master' of https://github.com/BehaviorTree/BehaviorTree.CPP
* critical bug fix affecting AsyncActionNode
  When a Tree is copied, all the thread related to AsyncActionNode where
  invoked.
  As a consequence, they are never executed, despite the fact that the
  value RUNNING is returned.
* Fix issue `#109 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/109>`_
* fix `#111 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/111>`_
* Merge pull request `#108 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/108>`_ from daniel-serrano/add-RobMoSys-acknowledgement
  Add robmosys acknowledgement
* Add robomosys acknowledgement as requested
* Add robomosys acknowledgement as requested
* added more comments (issue `#102 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/102>`_)
* Update README.md
* Add files via upload
* Merge pull request `#96 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/96>`_ from LoyVanBeek/patch-1
  Fix typo
* Update tutorial_04_sequence_star.md
* fix compilation
* removing backward_cpp
  Motivation: backward_cpp is SUPER useful, but it is a library to use at
  the application level. It makes no sense to add it at the library level.
* Merge pull request `#95 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/95>`_ from LoyVanBeek/patch-1
  Remove 0 in front of http://... URL to publication
* Remove 0 in front of http://... URL to publication
  Hopefully, this makes the link correctly click-able when rendered to HTML
* fix issue `#84 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/84>`_ (Directories)
* add infinite loop to Repeat and Retry (issue `#80 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/80>`_)
* fix unit test
* issue `#82 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/82>`_
* fix issue `#82 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/82>`_
* Added 32bits compilation configuration for msvc
* Contributors: Daniel Serrano, Davide Facont, Davide Faconti, Jimmy Delas, Loy

3.0.7 (2019-04-02)
------------------
* this should fix issue with tinyXML2 once and for all (maybe...)
* improvement #79
* doc fix
* Deprecating <remap> tag in SubTree
* fix windows compilation
* Update README.md
* back to c++11
* Contributors: Davide Faconti, Ferran Roure

3.0.4 (2019-03-19)
------------------
* fix issue #72 with sibling subtrees
* Update .travis.yml
* Contributors: Davide Faconti

3.0.3 (2019-03-12)
------------------
* moving to C++14... deal with it
* updated tinyXML2. Should fix several issues too
* add "d" to debug library on Windows
* fixed compilation error on Windows x64 (issue #63)
* Improved MSVC compilation
  Added _CRT_SECURE_NO_WARNINGS flag for msvc compilation
* adding TreeNode::modifyPortsRemapping that might be useful in the future
* Merge pull request #64 from luminize/patch-1
  docs/xml_format.md
* Merge pull request #65 from luminize/patch-2
  docs/tutorial_01_first_tree.md: fix typo
* docs/tutorial_01_first_tree.md: fix typo
* fix compilation in Windows/Release
* remove a warning in Windows
* Update README.md
* Merge branch 'windows_compilation'
* fix issue #63 : compile on windows
* Update .travis.yml
* Create .appveyor.yml
* fix compilation on windows
* fix potential issue
* bug fix
* Update README.md
* Contributors: Bas de Bruijn,  Davide Faconti, Jimmy Delas, hlzl

3.0.2 (2019-03-04)
------------------
* make flatbuffers visible to other project (such as Groot)
* docs fix
* Contributors: Davide Faconti

3.0.0 (2019-02-27)
------------------
* Merge branch 'ver_3'. Too many changes to count...
* Contributors: Davide Facont, Davide Faconti, ImgBotApp, Victor Lopez

2.5.1 (2019-01-14)
------------------
* fix installation directory
* #39 Fix Conan version (#42)
  Signed-off-by: Uilian Ries <uilianries@gmail.com>
* Update .travis.yml
* Conan package distribution (#39)
* Non-functional refactoring of xml_parsing to clean up the code
* cosmetic changes in the code of BehaviorTreeFactory
* XML schema. Related to enchancement #40
* call setRegistrationName() for built-in Nodes
  The methos is called by BehaviorTreefactory, therefore it
  registrationName is empty if trees are created programmatically.
* Reset reference count when destroying logger (issue #38)
* Contributors: Davide Facont, Davide Faconti, Uilian Ries

2.5.0 (2018-12-12)
------------------
* Introducing SyncActionNode that is more self explaining and less ambiguous
* fix potential problem related to ControlNode::haltChildren()
* Adding example/test of navigation and recovery behavior. Related to issue #36
* Contributors: Davide Faconti

2.4.4 (2018-12-12)
------------------
* adding virtual TreeNode::onInit() [issue #33]
* fix issue #34 : if you don't implement convertFromString, it will compile but it may throw
* Pretty demangled names and obsolate comments removed
* bug fixes
* more comments
* [enhancement #32]: add CoroActionNode and rename ActionNode as "AsynActionNode"
  The name ActionNode was confusing and it has been deprecated.
* Update README.md
* removed old file
* Fix issue #31 : convertFromString mandatory for TreeNode::getParam, not Blackboard::get
* Cherry piking changes from PR #19 which solve issue #2 CONAN support
* Contributors: Davide Faconti

2.4.3 (2018-12-07)
------------------
* Merge branch 'master' into ros2
* removed old file
* Fix issue #31 : convertFromString mandatory for TreeNode::getParam, not Blackboard::get
* 2.4.3
* version bump
* Merge pull request #30 from nuclearsandwich/patch-1
  Fix typo in package name.
* Remove extra find_package(ament_cmake_gtest).
  This package should only be needed if BUILD_TESTING is on and is
  find_package'd below if ament_cmake is found and BUILD_TESTING is on.
* Fix typo in package name.
* added video to readme
* Cherry piking changes from PR #19 which solve issue #2 CONAN support
* Merge pull request #29 from nuclearsandwich/ament-gtest-dep
  Add test dependency on ament_cmake_gtest.
* Add test dependency on ament_cmake_gtest.
* fix travis removing CI
* Contributors: Davide Faconti, Steven! Ragnarök

2.4.2 (2018-12-05)
------------------
* support ament
* change to ament
* Contributors: Davide Faconti

2.4.1 (2018-12-05)
------------------
* fix warnings and dependencies in ROS, mainly related to ZMQ
* Contributors: Davide Faconti

2.4.0 (2018-12-05)
------------------
* Merge pull request #27 from mjeronimo/bt-12-4-2018
  Add support for ament/colcon build
* updated documentation
* Merge pull request #25 from BehaviorTree/include_xml
  Add the ability to include an XML from another one
* <include> supports ROS package getPath (issue #17)
* Trying to fix writeXML (issue #24)
* New feature: include XMl from other XMLs (issue #17)
* more verbose error message
* adding unit tests for Repeat and Retry nodes #23
* Bug fix in Retry and Repeat Decorators (needs unit test)
* Throw if the parameter in blackboard can't be read
* Try to prevent error #22 in user code
* changed the protocol of the XML
* fixing issue #22
* Contributors: Davide Faconti, Michael Jeronimo

2.3.0 (2018-11-28)
------------------
* Fix: registerBuilder did not register the manifest. It was "broken" as public API method
* Use the Pimpl idiom to hide zmq from the header file
* move header of minitrace in the cpp file
* Fixed a crash occuring when you didn't initialized a Tree object (#20)
* Fix issue #16
* add ParallelNode to pre-registered entries in factory (issue #13)
* removed M_PI
* Update the documentation
* Contributors: Davide Faconti, Jimmy Delas

2.2.0 (2018-11-20)
------------------
* fix typo
* method contains() added to BlackBoard
* back compatible API change to improve the wrapping of legacy code (issue #15)
  Eventually, SimpleAction, SimpleDecorators and SimpleCondition can use
  blackboard and NodeParameters too.
* reduce potential memory allocations using string_view
* fix important issue with SubtreeNode
* Read at every tick the parameter if Blackboard is used
* Adding NodeParameters to ParallelNode
* travis update
* merge pull request #14 related to #10 (with some minor changes)
* Fix issue #8 and warning reported in #4
  Fixed problem of visibility with TinyXML2
* Contributors: Davide Faconti, Uilian Ries 

2.1.0 (2018-11-16)
------------------
* version 2.1. New directory structure
* Contributors: Davide Faconti
