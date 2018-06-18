#ifndef XML_PARSING_BT_H
#define XML_PARSING_BT_H

#include "behavior_tree_core/bt_factory.h"
#include "tinyXML2/tinyxml2.h"

namespace BT
{
class XMLParser
{
  public:
    XMLParser(const BehaviorTreeFactory& factory): factory_(factory) {}

    void loadFromFile(const std::string& filename) noexcept(false);

    void loadFromText(const std::string& xml_text) noexcept(false);

    bool verifyXML(std::vector<std::string>& error_messages) const noexcept(false);

    using NodeBuilder =
        std::function<TreeNodePtr(const std::string&, const std::string&, const NodeParameters&, TreeNodePtr)>;

    TreeNodePtr instantiateTree(std::vector<TreeNodePtr>& nodes);

  private:

    //method to visit each node of a tree
    TreeNodePtr treeParsing(const tinyxml2::XMLElement* root_element,
                            const NodeBuilder& node_builder,
                            std::vector<TreeNodePtr>& nodes,
                            const TreeNodePtr &root_parent);

    tinyxml2::XMLDocument doc_;

    const BehaviorTreeFactory& factory_;
};

class XMLWriter
{
public:
    XMLWriter( const BehaviorTreeFactory& factory):
        factory_(factory)
    {}

    std::string writeXML(const TreeNode* root_node, bool compact_representation = false) const;

private:
   const BehaviorTreeFactory& factory_;

};


}

#endif   // XML_PARSING_BT_H
