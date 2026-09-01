#include "behaviortree_cpp/bt_factory.h"
#include "behaviortree_cpp/controls/fallback_node.h"

BT_REGISTER_NODES(factory)
{
  BT::TreeNodeManifest manifest{ BT::NodeType::CONTROL, "PluginAsyncFallback", {}, {} };
  BT::SetNodeManifestAsync(manifest);
  factory.registerBuilder(
      manifest,
      [](const std::string& name, const BT::NodeConfig&) -> std::unique_ptr<BT::TreeNode> {
        return std::make_unique<BT::FallbackNode>(name, true);
      });
}
