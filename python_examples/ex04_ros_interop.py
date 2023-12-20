#!/usr/bin/env python3

"""
Demonstrates interop of BehaviorTree.CPP Python bindings and ROS2 via rclpy.

You can publish the transform expected in the tree below using this command:

    ros2 run tf2_ros static_transform_publisher \
        --frame-id odom --child-frame-id base_link \
        --x 1.0 --y 2.0
"""

import rclpy
from rclpy.node import Node
from tf2_ros.buffer import Buffer
from tf2_ros.transform_listener import TransformListener

from btpy import (
    BehaviorTreeFactory,
    StatefulActionNode,
    SyncActionNode,
    NodeStatus,
    ports,
)


xml_text = """
 <root BTCPP_format="4" >

     <BehaviorTree ID="MainTree">
         <ReactiveSequence name="root">
             <Print value="{tf}" />
             <GetRosTransform frame_id="odom" child_frame_id="base_link" tf="{tf}" />
         </ReactiveSequence>
     </BehaviorTree>

 </root>
"""


@ports(inputs=["frame_id", "child_frame_id"], outputs=["tf"])
class GetRosTransform(StatefulActionNode):
    def __init__(self, name, config, node):
        super().__init__(name, config)

        self.node = node
        self.tf_buffer = Buffer()
        self.tf_listener = TransformListener(self.tf_buffer, self.node)

    def on_start(self):
        return NodeStatus.RUNNING

    def on_running(self):
        frame_id = self.get_input("frame_id")
        child_frame_id = self.get_input("child_frame_id")

        time = self.node.get_clock().now()
        if self.tf_buffer.can_transform(frame_id, child_frame_id, time):
            tf = self.tf_buffer.lookup_transform(frame_id, child_frame_id, time)
            self.set_output("tf", tf)

        return NodeStatus.RUNNING

    def on_halted(self):
        pass


@ports(inputs=["value"])
class Print(SyncActionNode):
    def tick(self):
        value = self.get_input("value")
        if value is not None:
            print(value)
        return NodeStatus.SUCCESS


rclpy.init()
node = Node("ex04_ros_interop")

factory = BehaviorTreeFactory()
factory.register(GetRosTransform, node)
factory.register(Print)

tree = factory.create_tree_from_text(xml_text)

node.create_timer(0.01, lambda: tree.tick_once())
rclpy.spin(node)
