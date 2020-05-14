^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Changelog for package behaviortree_cpp
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

3.2.0 (2020-03-22)
------------------
* root_node removed in favour of a method. tickRoot() added
* Moving to c++14
* fixed compilation on ROS2 and ubuntu 18.94
* fix compilation and unit tests
* Boost coroutine (`#164 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/164>`_)
  Remove the faulty coroutine that created significant problems in favour of boost::coroutine2 (use older boost::coroutine as a fallback).
* doc fix
* Fix issue `#140 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/140>`_
* Merge branch 'master' of github.com:BehaviorTree/BehaviorTree.CPP
* (issue `#163 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/163>`_) fix ded code
* Merge pull request `#162 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/162>`_ from unvestigate/master
  Tree is declared as a struct, so it needs to be forward-declared as a…
* Tree is declared as a struct, so it needs to be forward-declared as a struct too.
* Merge pull request `#161 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/161>`_ from unvestigate/master
  A couple of small changes needed to build on MSVC2015
* The __cplusplus macro does not work properly prior to MSVC2017 15.7 Preview 3: https://devblogs.microsoft.com/cppblog/msvc-now-correctly-reports-__cplusplus/
* MSVC2015 seems to need an explicit operator== for comparing against literal strings.
* [Breaking API change] make TreeNode::setStatus() protected
  Users are not supposed to set the status of a node manually from the
  outside. This might be a source of hard to debug errors as seen in
  Navigation2 of ROS2.
  If this change breaks your code, there is an high probability that your
  code was already broken.
* fix warning
* comments
* Update action_node.h
* Fix bug in default port values
* fix issue `#141 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/141>`_
* fix unit tests
* cosmetic: names changed
* change API of haltChildren()
* fix unittest switch should halt
* halt the SwitchNode correctly
* add unittest switch_node
* Merge pull request `#155 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/155>`_ from BehaviorTree/imgbot
  [ImgBot] Optimize images
* Merge pull request `#156 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/156>`_ from HansRobo/patch-1
  Fix error message
* Modify documentation images.
  SequenceNode:
  AimTo -> AimAt
  "Aim at a target" might sound more appropriate in this example than "aim to reach a goal".
  SequenceStar:
  Sequence -> ReactiveSequence
  The text says "On the other hand, isBatteryOK must be checked at every tick,
  for this reason its parent must be a ReactiveSequence."
  Modify the image to match the text.
* Fix bug `#149 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/149>`_
* Merge pull request `#152 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/152>`_ from happykeyboard/add_unix_BUILD_SHARED
  Added BUILD_SHARED_LIBS option to cmake. If set to "OFF", static libr…
* Added BUILD_SHARED_LIBS option to cmake. If set to "OFF", static library will be generated
  for a UNIX build
  If BUILD_UNIT_TESTS is off, do not search for gtest library, and do not include tests subdirectory
* experimental integration of Switch ControlNode
* bug fix
* Added SubTtreeWrapper
* Merge pull request `#150 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/150>`_ from seanyen/patch-1
  Install library to portable locations
* Install library to portable locations
* Merge pull request `#126 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/126>`_ from Jesus05/patch-1
  (3dparty coroutine) ifdef MSV_VER to WIN32
* Merge pull request `#135 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/135>`_ from 3wnbr1/master
  Add macOS support
* Merge pull request `#138 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/138>`_ from HansRobo/fix/remerge\_`#53 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/53>`_
  Add `#53 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/53>`_ content
* Merge pull request `#142 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/142>`_ from RavenX8/patch-1
  Fixed VS2017/2019 compile error
* Merge pull request `#145 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/145>`_ from renan028/fix_retry_node_negative_tries
  fix RetryNode loop that should be an infinity loop if max_attempts\_ =…
* Update t11_runtime_ports.cpp
* make easier to create ports at run-time
* fix RetryNode loop that should be an infinity loop if max_attempts\_ == -1
  As documentation said:
  "Use -1 to create an infinite loop."
* Update basic_types.cpp
  Added missing include for std::setlocale. This fixes the following error in Visual Studio:
  https://ci.appveyor.com/project/facontidavide59577/behaviortree-cpp/build/job/d1ttd2w84nvnqo2e#L52
* Merge pull request `#139 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/139>`_ from scgroot/master
  Fixed compiling for c++17
* Fixed compiling for c++17
* Add `#53 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/53>`_ content
* Merge pull request `#136 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/136>`_ from msadowski/msadowski-readme-fix
  Fix some typos in readme
* Fix some typos in readme
* Add macOS support
* fix issue `#120 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/120>`_
* update flatbuffers and avoid ambiguities
* Update README.md
* (3dparty coroutine) ifdef MSV_VER to WIN32
  On Windows not only MSVC compilator.
* Contributors: 3wnbr1, Christopher Torres, Davide Faconti, HansRobo, ImgBotApp, Jesus, Kotaro Yoshimoto, Mateusz Sadowski, Peter Polidoro, Sean Yen, Sebastian Ahlman, Steffen Groot, Vadim Linevich, renan028

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
