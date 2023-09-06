#!/usr/bin/env python3

"""
Demo of seamless conversion between C++ and Python types.

NOTE: To run this example, make sure that the path
`sample_nodes/bin/libdummy_nodes_dyn.so` is accessible from the current working
directory. After building the project, this path will exist in your CMake build
root.
"""

from btpy import BehaviorTreeFactory, SyncActionNode, NodeStatus, ports


xml_text = """
 <root BTCPP_format="4" >

     <BehaviorTree ID="MainTree">
         <Sequence name="root">
             <!-- (C++ node) Put a C++ object on the blackboard -->
             <RandomVector      vector="{simple}" />
             <!-- (Py node) Print it from Python -->
             <Print             value="{simple}" />

             <!-- (Py node) Put a Python type onto the blackboard -->
             <PutVector         output="{complex}" />
             <!-- (C++ node) Print it from C++ using native C++ types -->
             <PrintMapOfVectors input="{complex}" />
             <!-- (Py node) Print it from Python -->
             <Print             value="{complex}" />
         </Sequence>
     </BehaviorTree>

 </root>
"""


@ports(outputs=["output"])
class PutVector(SyncActionNode):
    def tick(self):
        # Schema matching std::unordered_map<std::string, Vector3>
        # (defined in dummy_nodes.h, input type of PrintMapOfVectors)
        self.set_output(
            "output",
            {
                "a": {"x": 0.0, "y": 42.0, "z": 9.0},
                "b": {"x": 1.0, "y": -2.0, "z": 1.0},
            },
        )
        return NodeStatus.SUCCESS


@ports(inputs=["value"])
class Print(SyncActionNode):
    def tick(self):
        value = self.get_input("value")
        if value is not None:
            print("Python:", value)
        return NodeStatus.SUCCESS


factory = BehaviorTreeFactory()
factory.register_from_plugin("sample_nodes/bin/libdummy_nodes_dyn.so")
factory.register(PutVector)
factory.register(Print)

tree = factory.create_tree_from_text(xml_text)
tree.tick_while_running()
