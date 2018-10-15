# About this library

This  __C++__ library provides a framework to create BehaviorTrees.
It was designed to be flexible, easy to use and fast.

Even if our main use-case is __robotics__, you can use this library to build
__AI for games__, or to replace Finite State Machines in you application.

__BehaviorTree.CPP__ has many interesting features, when compared to other implementations:

- It makes asynchronous Actions, i.e. non-blocking, a first-class citizen.
- It allows the creation of trees at run-time, using a textual representation (XML).
- You can link staticaly you custom TreeNodes or convert them into plugins 
which are loaded at run-time.
- It includes a __logging/profiling__ infrastructure that allows the user 
to visualize, record, replay and analyze state transitions.

Documentation
------------

https://eurecat.github.io/BehaviorTree.CPP

Reference
------------
Please refer to the following paper when using the library:

**How Behavior Trees Modularize Hybrid Control Systems and Generalize Sequential Behavior Compositions, the Subsumption Architecture,
and Decision Trees.** Michele Colledanchise and Petter Ogren. IEEE Transaction on Robotics 2017.

Bibtex entry:

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

Further readings
---------------

The book Behavior Trees in Robotics and AI, published by CRC Press Taylor & Francis, is available for purchase
(ebook and hardcover) on the CRC Press Store or Amazon. The Preprint version (free) is available here: https://arxiv.org/abs/1709.00084

Tutorials available at https://btirai.github.io/
  
Aknowledgement
------------
This library is the result of the join effort between **Eurecat** (main autjor, Davide Faconti) and the
**Italian Institute of Technology** (Michele Colledanchise).

It is one of the main components of [MOOD2Be](https://eurecat.org/es/portfolio-items/mood2be/),
and it is developed at [Eurecat](https://eurecat.org) by Davide Faconti.

MOOD2Be is one of the six **Integrated Technical Projects (ITPs)** selected from the 
[RobMoSys first open call](https://robmosys.eu/itp/) and it received funding from the European
Union’s Horizon 2020 Research and Innovation Programme.

License
-------
The MIT License (MIT)

Copyright (c) 2014-2018 Michele Colledanchise
Copyright (c) 2018 Davide Faconti

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
