^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Changelog for package behaviortree_cpp
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

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
