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

    void loadFromFile(const std::string& filename);

    void loadFromText(const std::string& xml_text);

    bool verifyXML(std::vector<std::string>& error_messages) const noexcept(false);

    using NodeBuilder =
        std::function<TreeNode::Ptr(const std::string&, const std::string&, const NodeParameters&, TreeNode::Ptr)>;

    TreeNode::Ptr instantiateTree(std::vector<TreeNode::Ptr>& nodes);

  private:

    //method to visit each node of a tree
    TreeNode::Ptr treeParsing(const tinyxml2::XMLElement* root_element,
                            const NodeBuilder& node_builder,
                            std::vector<TreeNode::Ptr>& nodes,
                            const TreeNode::Ptr &root_parent);

    tinyxml2::XMLDocument doc_;

    const BehaviorTreeFactory& factory_;
};

/** Helper function to do the most common steps all at once:
* 1) Create an instance of XMLParse and call loadFromText.
* 2) Instantiate the entire tree.
* 3) Assign the given Blackboard
*
* return: a pair containing the root node (first) and a vector with all the instantiated nodes (second).
*/
std::pair<TreeNode::Ptr, std::vector<TreeNode::Ptr>>
buildTreeFromText(const BehaviorTreeFactory& factory,
                  const std::string& text,
                  const Blackboard::Ptr& blackboard = Blackboard::Ptr() );

/** Helper function to do the most common steps all at once:
* 1) Create an instance of XMLParse and call loadFromFile.
* 2) Instantiate the entire tree.
* 3) Assign the given Blackboard
*
* return: a pair containing the root node (first) and a vector with all the instantiated nodes (second).
*/
std::pair<TreeNode::Ptr, std::vector<TreeNode::Ptr>>
buildTreeFromFile(const BehaviorTreeFactory& factory,
                  const std::string& filename,
                  const Blackboard::Ptr& blackboard = Blackboard::Ptr() );

std::string writeXML(const BehaviorTreeFactory& factory,
                     const TreeNode* root_node,
                     bool compact_representation = false);



}

#endif   // XML_PARSING_BT_H
