#ifndef XML_PARSING_BT_H
#define XML_PARSING_BT_H

#include "behaviortree_cpp/bt_parser.h"

#include <filesystem>
#include <unordered_map>

namespace BT
{
/**
 * @brief The XMLParser is a class used to read the model
 * of a BehaviorTree from file or text and instantiate the
 * corresponding tree using the BehaviorTreeFactory.
 */
class XMLParser : public Parser
{
public:
  XMLParser(const BehaviorTreeFactory& factory);

  ~XMLParser() override;

  XMLParser(const XMLParser& other) = delete;
  XMLParser& operator=(const XMLParser& other) = delete;

  XMLParser(XMLParser&& other) noexcept;
  XMLParser& operator=(XMLParser&& other) noexcept;

  void loadFromFile(const std::filesystem::path& filename,
                    bool add_includes = true) override;

  void loadFromText(const std::string& xml_text, bool add_includes = true) override;

  [[nodiscard]] std::vector<std::string> registeredBehaviorTrees() const override;

  [[nodiscard]] Tree instantiateTree(const Blackboard::Ptr& root_blackboard,
                                     std::string main_tree_to_execute = {}) override;

  void clearInternalState() override;

private:
  struct PImpl;
  std::unique_ptr<PImpl> _p;
};

void VerifyXML(const std::string& xml_text,
               const std::unordered_map<std::string, NodeType>& registered_nodes);

/**
 * @brief writeTreeNodesModelXML generates an XMl that contains the manifests in the
 * <TreeNodesModel>
 *
 * @param factory          the factory with the registered types
 * @param include_builtin  if true, include the builtin Nodes
 *
 * @return  string containing the XML.
 */
[[nodiscard]] std::string writeTreeNodesModelXML(const BehaviorTreeFactory& factory,
                                                 bool include_builtin = false);

/**
 * @brief writeTreeXSD generates an XSD for the nodes defined in the factory
 *
 * @param factory          the factory with the registered types
 *
 * @return  string containing the XML.
 */
[[nodiscard]] std::string writeTreeXSD(const BehaviorTreeFactory& factory);

/**
 * @brief WriteTreeToXML create a string that contains the XML that corresponds to a given tree.
 * When using this function with a logger, you should probably set both add_metadata and
 * add_builtin_models to true.
 *
 * @param tree               the input tree
 * @param add_metadata       if true, the attributes "_uid" and "_fullPath" will be added to the nodes
 * @param add_builtin_models if true, include the builtin Nodes into the <TreeNodesModel>
 *
 * @return string containing the XML.
 */
[[nodiscard]] std::string WriteTreeToXML(const Tree& tree, bool add_metadata,
                                         bool add_builtin_models);

}  // namespace BT

#endif  // XML_PARSING_BT_H
