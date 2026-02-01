#pragma once

#include "behaviortree_cpp/bt_factory.h"
#include "behaviortree_cpp/flatbuffers/BT_logger_generated.h"

namespace BT
{

using SerializedTransition = std::array<uint8_t, 12>;

inline Serialization::NodeType convertToFlatbuffers(BT::NodeType type)
{
  switch(type)
  {
    case BT::NodeType::ACTION:
      return Serialization::NodeType::ACTION;
    case BT::NodeType::DECORATOR:
      return Serialization::NodeType::DECORATOR;
    case BT::NodeType::CONTROL:
      return Serialization::NodeType::CONTROL;
    case BT::NodeType::CONDITION:
      return Serialization::NodeType::CONDITION;
    case BT::NodeType::SUBTREE:
      return Serialization::NodeType::SUBTREE;
    case BT::NodeType::UNDEFINED:
      return Serialization::NodeType::UNDEFINED;
  }
  return Serialization::NodeType::UNDEFINED;
}

inline Serialization::NodeStatus convertToFlatbuffers(BT::NodeStatus type)
{
  switch(type)
  {
    case BT::NodeStatus::SKIPPED:
    case BT::NodeStatus::IDLE:
      return Serialization::NodeStatus::IDLE;
    case BT::NodeStatus::SUCCESS:
      return Serialization::NodeStatus::SUCCESS;
    case BT::NodeStatus::RUNNING:
      return Serialization::NodeStatus::RUNNING;
    case BT::NodeStatus::FAILURE:
      return Serialization::NodeStatus::FAILURE;
  }
  return Serialization::NodeStatus::IDLE;
}

inline Serialization::PortDirection convertToFlatbuffers(BT::PortDirection direction)
{
  switch(direction)
  {
    case BT::PortDirection::INPUT:
      return Serialization::PortDirection::INPUT;
    case BT::PortDirection::OUTPUT:
      return Serialization::PortDirection::OUTPUT;
    case BT::PortDirection::INOUT:
      return Serialization::PortDirection::INOUT;
  }
  return Serialization::PortDirection::INOUT;
}

inline void CreateFlatbuffersBehaviorTree(flatbuffers::FlatBufferBuilder& builder,
                                          const BT::Tree& tree)
{
  std::vector<flatbuffers::Offset<Serialization::TreeNode>> fb_nodes;

  applyRecursiveVisitor(tree.rootNode(), [&](BT::TreeNode* node) {
    std::vector<uint16_t> children_uid;
    if(auto control = dynamic_cast<BT::ControlNode*>(node))
    {
      children_uid.reserve(control->children().size());
      for(const auto& child : control->children())
      {
        children_uid.push_back(child->UID());
      }
    }
    else if(auto decorator = dynamic_cast<BT::DecoratorNode*>(node))
    {
      const auto& child = decorator->child();
      children_uid.push_back(child->UID());
    }

    // Const cast to ensure public access to config() overload
    const auto& node_config = const_cast<BT::TreeNode const&>(*node).config();
    std::vector<flatbuffers::Offset<Serialization::PortConfig>> ports;
    for(const auto& it : node_config.input_ports)
    {
      ports.push_back(Serialization::CreatePortConfigDirect(builder, it.first.c_str(),
                                                            it.second.c_str()));
    }
    for(const auto& it : node_config.output_ports)
    {
      ports.push_back(Serialization::CreatePortConfigDirect(builder, it.first.c_str(),
                                                            it.second.c_str()));
    }

    auto tn = Serialization::CreateTreeNode(
        builder, node->UID(), builder.CreateVector(children_uid),
        convertToFlatbuffers(node->status()), builder.CreateString(node->name().c_str()),
        builder.CreateString(node->registrationName().c_str()),
        builder.CreateVector(ports));

    fb_nodes.push_back(tn);
  });

  std::vector<flatbuffers::Offset<Serialization::NodeModel>> node_models;

  for(const auto& node_it : tree.manifests)
  {
    const auto& manifest = node_it.second;
    std::vector<flatbuffers::Offset<Serialization::PortModel>> port_models;

    for(const auto& port_it : manifest.ports)
    {
      const auto& port_name = port_it.first;
      const auto& port = port_it.second;
      auto port_model = Serialization::CreatePortModel(
          builder, builder.CreateString(port_name.c_str()),
          convertToFlatbuffers(port.direction()),
          builder.CreateString(demangle(port.type()).c_str()),
          builder.CreateString(port.description().c_str()));
      port_models.push_back(port_model);
    }

    auto node_model = Serialization::CreateNodeModel(
        builder, builder.CreateString(manifest.registration_ID.c_str()),
        convertToFlatbuffers(manifest.type), builder.CreateVector(port_models));

    node_models.push_back(node_model);
  }

  auto behavior_tree = Serialization::CreateBehaviorTree(
      builder, tree.rootNode()->UID(), builder.CreateVector(fb_nodes),
      builder.CreateVector(node_models));

  builder.Finish(behavior_tree);
}

/** Serialize manually the information about state transition
 * No flatbuffer serialization here
 */
inline SerializedTransition SerializeTransition(uint16_t UID, Duration timestamp,
                                                NodeStatus prev_status, NodeStatus status)
{
  using namespace std::chrono;
  SerializedTransition buffer;
  int64_t usec = duration_cast<microseconds>(timestamp).count();
  int64_t t_sec = usec / 1000000;
  int64_t t_usec = usec % 1000000;

  flatbuffers::WriteScalar(&buffer[0], t_sec);
  flatbuffers::WriteScalar(&buffer[4], t_usec);
  flatbuffers::WriteScalar(&buffer[8], UID);

  flatbuffers::WriteScalar(&buffer[10],
                           static_cast<int8_t>(convertToFlatbuffers(prev_status)));
  flatbuffers::WriteScalar(&buffer[11],
                           static_cast<int8_t>(convertToFlatbuffers(status)));

  return buffer;
}

}  // namespace BT
