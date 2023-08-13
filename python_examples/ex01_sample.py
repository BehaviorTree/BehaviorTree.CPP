#!/usr/bin/env python3

"""
Demo adapted from [1].

To run, ensure that the `btpy_cpp` Python extension is on your `PYTHONPATH`
variable. It is probably located in your build directory if you're building from
source.

[1]: https://github.com/BehaviorTree/btcpp_sample
"""

from btpy import BehaviorTreeFactory, SyncActionNode, NodeStatus, ports


xml_text = """
 <root BTCPP_format="4" >

     <BehaviorTree ID="MainTree">
        <Sequence name="root">
            <AlwaysSuccess/>
            <SaySomething   message="this works too" />
            <ThinkWhatToSay text="{the_answer}"/>
            <SaySomething   message="{the_answer}" />
        </Sequence>
     </BehaviorTree>

 </root>
"""


@ports(inputs=["message"])
class SaySomething(SyncActionNode):
    def tick(self):
        msg = self.get_input("message")
        print(msg)
        return NodeStatus.SUCCESS


@ports(outputs=["text"])
class ThinkWhatToSay(SyncActionNode):
    def tick(self):
        self.set_output("text", "The answer is 42")
        return NodeStatus.SUCCESS


factory = BehaviorTreeFactory()
factory.register(SaySomething)
factory.register(ThinkWhatToSay)

tree = factory.create_tree_from_text(xml_text)
tree.tick_while_running()
