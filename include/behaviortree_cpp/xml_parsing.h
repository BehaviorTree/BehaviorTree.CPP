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
 * @brief writeTreeXSD generates an XSD for the nodes defined in the factory.
 *
 * @param factory     the factory with the registered types
 *
 * @return  string containing the XSD XML (strict, closed-vocabulary schema).
 */
[[nodiscard]] std::string writeTreeXSD(const BehaviorTreeFactory& factory);

/**
 * @brief writeTreeXSD generates an XSD for the nodes defined in the factory.
 *
 * @param factory     the factory with the registered types
 * @param generic     if true, oneNodeGroup uses xs:any processContents="lax"
 *                    instead of a closed xs:choice, allowing unknown custom
 *                    node elements to pass validation.  Top-level xs:element
 *                    declarations are also emitted so that lax processing can
 *                    still resolve and validate the known built-in node types.
 *
 * @return  string containing the XSD XML.
 */
[[nodiscard]] std::string writeTreeXSD(const BehaviorTreeFactory& factory, bool generic);

/**
 * @brief writeTreeSchematron generates an ISO Schematron schema for BehaviorTree.CPP XML files.
 *
 * XSD alone cannot express cross-reference constraints.  This Schematron
 * complements writeTreeXSD() with three rule patterns:
 *   - treeNodesModel: every custom (non-built-in) node element appearing in a
 *     BehaviorTree body must have a matching TreeNodesModel entry (required by
 *     Groot2 for port display and editing).
 *   - subtreeResolution: every <SubTree ID="X"/> must resolve to a
 *     <BehaviorTree ID="X"> in the same file (relaxed when <include> is present).
 *   - rootStructure: main_tree_to_execute must name an existing BehaviorTree.
 *
 * The built-in node list is derived from factory.builtinNodes() so that the
 * schema stays in sync as new built-ins are added.
 *
 * @param factory  factory from which the built-in node names are taken;
 *                 a default-constructed BehaviorTreeFactory suffices for most uses.
 *
 * @return  string containing the Schematron XML (queryBinding="xslt",
 *          compatible with xsltproc and lxml.isoschematron).
 */
[[nodiscard]] std::string writeTreeSchematron(const BehaviorTreeFactory& factory);

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

namespace detail
{
/// Returns true when @p pos inside @p s falls within an XML comment (<!-- ... -->).
[[nodiscard]] bool insideXmlComment(const std::string& s, std::size_t pos);
}  // namespace detail

}  // namespace BT

#endif  // XML_PARSING_BT_H
