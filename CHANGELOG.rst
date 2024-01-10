^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Changelog for package behaviortree_cpp
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

4.4.3 (2023-12-19)
------------------
* Merge pull request #709 from galou/unset_blackboard
* fix issue `#725 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/725>`_ : SetBlackboard can copy entries
* add more unit tests
* fix typos `#721 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/721>`_
* fix: guard macro declaration to prevent redefinition warning
* fix: Rename scoped lock so it doesn't hide the outer lock triggering a compiler warning
* add private ports to exclude from autoremapping `#706 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/706>`_
* fix issue `#713 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/713>`_:  getNodesByPath should be const
* Contributors: Davide Faconti, Nestor Gonzalez, Tony Paulussen

4.4.2 (2023-11-28)
------------------
* fix issue `#702 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/702>`_ : output ports require {}
* Merge pull request `#691 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/691>`_ from galou/small_refactor_and_doc
  Small code refactor, log- and doc changes
* Merge pull request `#701 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/701>`_ from tony-p/fix/file-loggers-protected
  fix: ensure public get config overload is used
* ci: use pixi github action
* fix: ensure public get config overload is used
* Small code refactor, log- and doc changes
* Contributors: Davide Faconti, Gaël Écorchard, Tony Paulussen

4.4.1 (2023-11-12)
------------------
* erase server_port+1
* add reset by default in base classes (fix `#694 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/694>`_)
* fix issue `#696 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/696>`_ (wrong autoremapping)
* Remove traces of SequenceStar
* fix `#685 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/685>`_ (timeout in ZMP publisher)
* clang: fix warning
  fix warning: lambda capture 'this' is not used
* Use feature test macro to check availability of `std::from_chars`
* fix warning in older compilers
* Contributors: Christoph Hertzberg, Davide Faconti, Gaël Écorchard, Shen Xingjian, Sid

4.4.0 (2023-10-16)
------------------
* Update ex05_subtree_model.cpp
* added any::stringToNumber
* added SubTree model example
* unit test for issue 660
* adding SubTree model
* minor changes
* change blackboard entry
* Update simple_string.hpp
* SimpleString: fix warning by checking upper size limit (`#666 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/666>`_)
* Contributors: Adam Boseley, Davide Faconti

4.3.8 (2023-10-09)
------------------
* ReactiveSequence and ReactiveFallback will behave more similarly to 3.8
* bug fix in wakeUpSignal
* ignore newlines in script
* stop ordering ports in TreeNodesModel
* add a specific tutorial for plugins
* Contributors: Davide Faconti

4.3.7 (2023-09-12)
------------------
* Test and fix issue `#653 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/653>`_: AnyTypeAllowed by default
* more time margin for Windows tests
* Add support for successful conda builds (`#650 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/650>`_)
* fix: Update how unit tests are executed in the github workflow so they are actually run on windows (`#647 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/647>`_)
* Add unit test related to SequenceWithMemory `#636 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/636>`_
* Contributors: Davide Faconti, tony-p

4.3.6 (2023-08-31)
------------------
* Simplify the visualization of custom type in Groot2 and improved tutorial 12
* fix compilation warnings
* Apply changes in ReactiveSequence to ReactiveFallback too
* test that logging works correctly with ReactiveSequence `#643 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/643>`_
* reduce the number of times preconditions scripts are executed
* PauseWithRetry test added
* Contributors: Davide Faconti

4.3.5 (2023-08-14)
------------------
* fix issue `#621 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/621>`_: ConsumeQueue
* feat: add template specialization for convertFromString deque (`#628 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/628>`_)
* unit test added
* Update groot2_publisher.h (`#630 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/630>`_)
* unit test issue `#629 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/629>`_
* WhileDoElseNode can have 2 or 3 children (`#625 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/625>`_)
* fix issue `#624 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/624>`_ : add TimeoutNode::halt()
* fix recording_fist_time issue on windows (`#618 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/618>`_)
* Contributors: Aglargil, Davide Faconti, Michael Terzer, benyamin saedi, muritane

4.3.4 (2023-07-25)
------------------
* Fix error #617 in TestNode
* minitrace updated
* fix issue #615 : don't execute preconditions if state is RUNNING
* README.md
* fix issue `#605 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/605>`_: strip whitespaces and better error message
* Export cxx-standard with target. (`#604 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/604>`_)
* feature `#603 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/603>`_: add static method [std::string description()] to manifest
* fix issue with move semantic
* Contributors: Davide Faconti, Sebastian Kasperski

4.3.3 (2023-07-05)
------------------
* bug fix `#601 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/601>`_: onHalted not called correctly in Control Nodes
* Groot recording (`#598 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/598>`_)
  * add recording to groot publisher
  * fixed
  * protocols compatibility
  * reply with first timestamp
  * remove prints
* Fix error when building static library (`#599 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/599>`_)
* fix warnings
* 4.3.2
* prepare release
* fix `#595 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/595>`_ : improvement in blackboard/scripting types (`#597 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/597>`_)
* Merge branch 'master' of github.com:BehaviorTree/BehaviorTree.CPP
* Merge branch 'parallel_all'
* Fix Issue 593 (`#594 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/594>`_): support skipping in Parallel node
* fix ParallelAll
* adding ParallelAll, WIP
* Contributors: Davide Faconti, Oleksandr Perepadia

4.3.2 (2023-06-27)
------------------
* fix `#595 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/595>`_ : improvement in blackboard/scripting types (`#597 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/597>`_)
* Fix Issue 593 (`#594 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/594>`_): support skipping in Parallel node
* adding ParallelAll
* Contributors: Davide Faconti

4.3.1 (2023-06-21)
------------------
* fix issue `#592 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/592>`_
* use lambda in tutorial
* add script condition
* "fix" issue `#587 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/587>`_: ReactiveSequence should set conditions to IDLE
* better error message
* Fix issue `#585 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/585>`_
* Contributors: Davide Faconti

4.3.0 (2023-06-13)
------------------
* use PImpl in multiple classes
* updated FileLogger2
* better error messages
* blackboard refactoring to fix buggy _autoremap
* improved support for default values
* fix error and add nodiscard
* Fix `#580 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/580>`_ : more informative error when not specializing BT::toStr
* add builtin models to WriteTreeToXML
* add simple example to generate logs
* add Sleep Node
* Fix `#271 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/271>`_: better error message
* remove EOL ros2 from CI
* Contributors: Davide Faconti

4.2.1 (2023-06-07)
------------------
* Fix `#570 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/570>`_: string_view set in blackboard
* Fix missing attribute in generated XML (writeTreeNodesModelXML)
* Allow registration of TestNode
* Contributors: Davide Faconti, Oleksandr Perepadia

4.2.0 (2023-05-23)
------------------
* add more informative IDLE status
* more informative error message when trying to register virtual classes
* fixes and simpler getAnyLocked
* add Tree::getNodesByPath
* add FileLogger2
* change getPortAny name and fic loop_node
* Lexy updated to release 2022.12.1
* do not skip pre-post condition in substituted tick
* added Loop node
* deprecating getAny
* revert new behavior of Sequence and Fallback
* add resetChild to all the decorators that missed it
* Add test related to issue `#539 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/539>`_
* related to `#555 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/555>`_
* Critical bug fix in XML exporting
* Fix writeTreeNodesModelXML
* fix ament not registering executables as tests
* fix std::system_error in TimeoutNode
* minor changes, mostly comments
* add version string
* old ZMQ publisher removed
* Add RunOnce, based on `#472 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/472>`_
* Contributors: Alberto Soragna, Davide Faconti, Gaël Écorchard, Mithun Kinarullathil, Sergei Molchanov

4.1.1 (2023-03-29)
------------------
* adding sqlite logger
* fix warning
* better cmake
* ManualSelector removed
* magic_enum updated
* fix issue `#530 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/530>`_: use convertFromString in scripting assignments
* added unit test
* files moved
* fix groot2 publisher
* minor fixes in blackboard
* fix XML: Subtree should remember the remapped ports
* add the ability to load substitution rules from JSON
* Update README.md
* Contributors: Davide Faconti

4.1.0 (2023-03-18)
------------------
* temporary disable codeql
* Groot2 interface (`#528 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/528>`_)
  * refactored groot2 interface
  * protocol updated
* merging groot2 publisher
* add observer
* prepare 4.1
* Update README.md
* fix issue `#525 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/525>`_ when ReactiveSequence contains skipped children
* fix reactive sequence (issue `#526 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/526>`_ and `#525 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/525>`_)
* better test
* add cast to ENUMS in ports
* changes ported from 4.1
* fix samples
* better include paths
* Control node and Decorators RUNNING before first child
* blackboard: update getKeys and add mutex to scripting
* add [[nodiscard]] and some othe minor changes
* add screenshot
* change the behavior of tickOnce to actually loop is wake up signal is… (`#522 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/522>`_)
  * change the behavior of tickOnce to actually loop is wake up signal is received
  * fix warning
* Cmake conan (`#521 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/521>`_)
  * boost coroutine substituted with minicoro. 3rd party updates
  * cmake refactoring + conan
  * fix cmake
  * fix build with conan and change CI
* fix CI in ROS1 (`#519 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/519>`_)
* fix alloc-dealloc-mismatch for _storage.str.data (`#518 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/518>`_)
* Fix issue `#515 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/515>`_: reactive sequence not skipped correctly
* Fix issue `#517 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/517>`_
* Merge branch 'master' of github.com:BehaviorTree/BehaviorTree.CPP
* fix issue `#492 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/492>`_ (Threads::Threads)
* Fix boost dependency in package.xml (`#512 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/512>`_)
  `libboost-coroutine-dev` has been merged into rosdistro on February 21st
  2023. Link to merge request: https://github.com/ros/rosdistro/pull/35789/.
* fix compilation
* revert breaking change
* Merge branch 'master' of github.com:BehaviorTree/BehaviorTree.CPP
* make default value of port optional, to allow empty strings
* Contributors: Alberto Soragna, Bart Keulen, Davide Faconti

4.0.2 (2023-02-17)
------------------
* fix issue `#501 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/501>`_
* fix issue `#505 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/505>`_
* solve issue `#506 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/506>`_
* prevent useless exception catcking
* fix issue `#507 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/507>`_
* adding the uid to the log to uniquely identify the nodes (`#502 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/502>`_)
* fix in SharedLibrary and cosmetic changes to the code
* using tinyxml ErrorStr() instead of ErrorName() to get more info about missing file (`#497 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/497>`_)
* Fixed use of ros_pkg for ROS1 applications (`#483 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/483>`_)
* Fix error message StdCoutLogger -> MinitraceLogger (`#495 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/495>`_)
* Fix boost dependency in package.xml (`#493 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/493>`_)
  Co-authored-by: Bart Keulen <b.keulen@avular.com>
* support Enums in string conversion
* fix issue 489
* updated example. Demonstrate pass by reference
* lexy updated
* rename haltChildren to resetChildren
* revert `#329 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/329>`_
* Merge branch 'master' of github.com:BehaviorTree/BehaviorTree.CPP
* Small improvements (`#479 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/479>`_)
  * Make message for allowed port names more explicit
  Also throw an exception for unknown port direction rather than using
  `PortDirection::INOUT`.
  * Small code improvements
  * Remove code without effect
* Fix some renaming for V4 (`#480 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/480>`_)
* Define NodeConfiguration for BT3 compatibility (`#477 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/477>`_)
* Implement `#404 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/404>`_ to solve `#435 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/435>`_ (gtest not found)
* fix issue `#474 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/474>`_ Make libraries dependencies private
* fix issue `#413 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/413>`_ (Delay logic)
* change suggested in `#444 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/444>`_
* add XML converter
* Add CodeQL workflow (`#471 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/471>`_)
* Update README.md
* Contributors: Ana, Bart Keulen, Christian Henkel, Davide Faconti, Gaël Écorchard, Jorge, Mahmoud Farshbafdoustar, Norawit Nangsue

4.0.1 (2022-11-19)
------------------
* version 4.X
* Contributors: Adam Aposhian, Adam Sasine, Alberto Soragna, Ali Aydın KÜÇÜKÇÖLLÜ, AndyZe, Davide Faconti, Dennis, Gaël Écorchard, Jafar, Joseph Schornak, Luca Bonamini, Paul Bovbel, SubaruArai, Tim Clephas, Will

3.7.0 (2022-05-23)
-----------
* add netlify stuff
* Event based trigger introduced
  Added a new mechanism to emit "state changed" events that can "wake up" a tree.
  In short, it just provide an interruptible "sleep" function.
* Fixed bug where including relative paths would fail to find the correct file (`#358 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/358>`_)
  * Added unit tests to verify current behavior
  * Fixed bug where including relative paths would fail to find the correct file
  * Added gtest environment to access executable path
  This path lets tests access files relative to the executable for better transportability
  * Changed file commandto add_custom_target
  The file command only copies during the cmake configure step. If source files change, file is not ran again
* Added pure CMake action to PR checks (`#378 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/378>`_)
  * Added CMake CI to PR checks
  * Renamed action to follow pattern
* updated documentation
* add the ability to register multiple BTs (`#373 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/373>`_)
* Update ros1.yaml
* fix `#338 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/338>`_
* fix issue `#330 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/330>`_
* fix issue `#360 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/360>`_
* Merge branch 'master' of github.com:BehaviorTree/BehaviorTree.CPP
* Update Tutorial 2 Docuemtation (`#372 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/372>`_)
* Update tutorial_09_coroutines.md (`#359 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/359>`_)
  Minor fix, renamed Timepoint to TimePoint.
* Export dependency on ament_index_cpp (`#362 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/362>`_)
  To make dependent packages try to link ament_index_cpp, export the
  dependency explicitly.
* Change order of lock to prevent deadlock. (`#368 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/368>`_)
  Resolves `#367 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/367>`_.
* Fix `#320 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/320>`_ : forbit refrences in Any
* Update action_node.h
* Contributors: Adam Sasine, Davide Faconti, Fabian Schurig, Griswald Brooks, Hyeongsik Min, Robodrome, imgbot[bot], panwauu

3.6.1 (2022-03-06)
------------------
* remove windows tests
* fix thread safety
* fix CI
* Don't restart SequenceStar on halt (`#329 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/329>`_)
  * Add more SequenceStar tests
  * Fix typo in test name
  * Don't reset SequenceStar on halt
* [docs] add missing node `SmashDoor` (`#342 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/342>`_)
* ROS2 include ros_pkg attribute support (`#351 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/351>`_)
  * ROS2 include pkg support
  * ros2 build fixed
  Co-authored-by: Benjamin Linne <benjamin.linne.civ@army.mil>
* [ImgBot] Optimize images (`#334 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/334>`_)
  *Total -- 90.34kb -> 61.77kb (31.63%)
  /docs/images/Tutorial1.svg -- 10.08kb -> 6.33kb (37.19%)
  /docs/images/FetchBeerFails.svg -- 9.00kb -> 5.93kb (34.13%)
  /docs/images/FetchBeer2.svg -- 21.19kb -> 14.41kb (32%)
  /docs/images/Tutorial2.svg -- 34.19kb -> 23.75kb (30.54%)
  /docs/images/DecoratorEnterRoom.svg -- 15.88kb -> 11.35kb (28.54%)
  Co-authored-by: ImgBotApp <ImgBotHelp@gmail.com>
* [Docs] BT_basics fix typo (`#343 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/343>`_)
* [docs] Clarify sentence (`#344 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/344>`_)
  `... will sleep up to 8 hours or less, if he/she is fully rested.` was not clear. It can also be understood as `If he/she is fully rested, the character will sleep ...`
* [docs] match text to graphics (`#340 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/340>`_)
* Docs: BT_basics fix typo (`#337 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/337>`_)
* Merge branch 'master' of github.com:BehaviorTree/BehaviorTree.CPP
* fix svg
* Fix CMake ENABLE_COROUTINES flag with Boost < 1.59 (`#335 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/335>`_)
  Co-authored-by: Cam Fulton <cfulton@symbotic.com>
* Add ENABLE_COROUTINES CMake option (`#316 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/316>`_)
  * Add DISABLE_COROUTINES CMake option
  * Change convention of CMake coroutine flag to ENABLE
  Co-authored-by: Cam Fulton <cfulton@symbotic.com>
* [ImgBot] Optimize images (`#333 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/333>`_)
  *Total -- 152.97kb -> 114.57kb (25.1%)
  /docs/images/ReactiveSequence.svg -- 7.58kb -> 4.59kb (39.47%)
  /docs/images/SequenceNode.svg -- 11.28kb -> 7.12kb (36.87%)
  /docs/images/SequenceStar.svg -- 11.22kb -> 7.09kb (36.8%)
  /docs/images/DecoratorEnterRoom.svg -- 20.71kb -> 13.30kb (35.77%)
  /docs/images/FallbackBasic.svg -- 19.09kb -> 12.64kb (33.79%)
  /docs/images/FetchBeer.svg -- 24.30kb -> 16.36kb (32.66%)
  /docs/images/SequenceBasic.svg -- 6.32kb -> 5.49kb (13.04%)
  /docs/images/Tutorial1.svg -- 6.67kb -> 5.94kb (10.98%)
  /docs/images/FetchBeerFails.svg -- 6.46kb -> 5.83kb (9.76%)
  /docs/images/FetchBeer2.svg -- 14.99kb -> 13.76kb (8.18%)
  /docs/images/Tutorial2.svg -- 24.35kb -> 22.44kb (7.85%)
  Co-authored-by: ImgBotApp <ImgBotHelp@gmail.com>
* doc fix
* Merge branch 'new_doc'
* remove deprecated code
* updated documentation
* [Fix] Fix cmake version warning and -Wformat warning (`#319 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/319>`_)
  Co-authored-by: Homalozoa <xuhaiwang@xiaomi.com>
* Update README.md
* Fix Windows shared lib build (`#323 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/323>`_)
* fix shadowed variable in string_view.hpp (`#327 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/327>`_)
* Build Sample Nodes By Default to Fix Github Action (`#332 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/332>`_)
  * Fix github action
  * Change working directory in github action step
  * Build samples by default
* Added BlackboardCheckBool decorator node (`#326 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/326>`_)
  * Added tests for BlackboardCheck decorator node
  * Added BlackboardCheckBool decorator node
* Fixed typo "Exeption" -> "Exception" (`#331 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/331>`_)
* WIP
* fix `#325 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/325>`_
* Contributors: Adam Sasine, Affonso, Guilherme, Alberto Soragna, Davide Faconti, Homalozoa X, Jake Keller, Philippe Couvignou, Tobias Fischer, benjinne, fultoncjb, goekce, imgbot[bot]

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
* Always use std::string_view for binary compatibility (fix issue `#200 <https://github.com/BehaviorTree/BehaviorTree.CPP/issues/200>`_)
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
