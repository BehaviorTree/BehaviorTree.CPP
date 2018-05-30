#ifndef XML_PARSING_BT_H
#define XML_PARSING_BT_H

#include "behavior_tree_core/bt_factory.h"
#include "tinyXML2/tinyxml2.h"

namespace BT
{
class XMLParser
{
  public:
    XMLParser() = default;

    void loadFromFile(const std::string& filename) noexcept(false);

    void loadFromText(const std::string& xml_text) noexcept(false);

    bool verifyXML(std::vector<std::string>& error_messages) const noexcept(false);

    template <typename NodePtr>
    using NodeBuilder =
        std::function<NodePtr(const std::string& ID, const std::string& name, const NodeParameters&, NodePtr)>;

    // general and reusable method to visit each node of a tree
    template <typename NodePtr>
    NodePtr treeParsing(const tinyxml2::XMLElement* root_element, const NodeBuilder<NodePtr>& node_builder,
                        std::vector<NodePtr>& nodes, NodePtr root_parent);

    TreeNodePtr instantiateTree(const BehaviorTreeFactory& factory, std::vector<TreeNodePtr>& nodes);

  private:
    tinyxml2::XMLDocument doc_;
};

//---------------------------------------------

template <typename NodePtr>
inline NodePtr XMLParser::treeParsing(const tinyxml2::XMLElement* root_element,
                                      const NodeBuilder<NodePtr>& node_builder, std::vector<NodePtr>& nodes,
                                      NodePtr root_parent)
{
    using namespace tinyxml2;

    std::function<NodePtr(NodePtr, const tinyxml2::XMLElement*)> RecursiveVisitor;

    RecursiveVisitor = [&](NodePtr parent, const tinyxml2::XMLElement* element) -> NodePtr {
        const std::string element_name = element->Name();
        std::string node_ID;
        std::string node_alias;
        NodeParameters node_params;

        // Actions and Decorators have their own ID
        if (element_name == "Action" || element_name == "Decorator" || element_name == "Condition")
        {
            node_ID = element->Attribute("ID");
        }
        else
        {
            node_ID = element_name;
        }

        const char* attr_alias = element->Attribute("name");
        if (attr_alias)
        {
            node_alias = attr_alias;
        }
        else
        {
            node_alias = node_ID;
        }

        if (element_name == "SubTree")
        {
            node_alias = element->Attribute("ID");
        }

        for (const XMLAttribute* att = element->FirstAttribute(); att; att = att->Next())
        {
            const std::string attribute_name = att->Name();
            if (attribute_name != "ID" && attribute_name != "name")
            {
                node_params[attribute_name] = att->Value();
            }
        }

        NodePtr node = node_builder(node_ID, node_alias, node_params, parent);
        nodes.push_back(node);

        for (auto child_element = element->FirstChildElement(); child_element;
             child_element = child_element->NextSiblingElement())
        {
            RecursiveVisitor(node, child_element);
        }

        return node;
    };

    // start recursion
    NodePtr root = RecursiveVisitor(root_parent, root_element);
    return root;
}
}

#endif   // XML_PARSING_BT_H
