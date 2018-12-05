^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Changelog for package behaviortree_cpp
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

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
