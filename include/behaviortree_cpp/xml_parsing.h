#ifndef XML_PARSING_BT_H
#define XML_PARSING_BT_H

#include "behaviortree_cpp/bt_factory.h"

namespace BT
{
/**
 * @brief The XMLParser is a class used to read the model
 * of a BehaviorTree from file or text and instantiate the
 * corresponding tree using the BehaviorTreeFactory.
 */
class XMLParser
{
  public:
    XMLParser(const BehaviorTreeFactory& factory);

    ~XMLParser();

    XMLParser(const XMLParser& other) = delete;
    XMLParser& operator=(const XMLParser& other) = delete;

    void loadFromFile(const std::string& filename);

    void loadFromText(const std::string& xml_text);

    TreeNode::Ptr instantiateTree(std::vector<TreeNode::Ptr>& nodes,
                                  const Blackboard::Ptr &blackboard);

  private:

    struct Pimpl;
    Pimpl* _p;

};

/**
 * @brief Struct used to store a tree.
 * If this object goes out of scope, the tree is destroyed.
 *
 * To tick the tree, simply call:
 *
 *    NodeStatus status = my_tree.root_node->executeTick();
 */
struct Tree
{
    TreeNode* root_node;
    std::vector<TreeNode::Ptr> nodes;

    Tree() : root_node(nullptr)
    { }

    Tree(TreeNode* root_node, std::vector<TreeNode::Ptr> nodes)
        : root_node(root_node), nodes(nodes)
    { }

    ~Tree()
    {
        if (root_node) {
            haltAllActions(root_node);
        }
    }
};

/** Helper function to do the most common steps, all at once:
* 1) Create an instance of XMLParse and call loadFromText.
* 2) Instantiate the entire tree.
*
*/
Tree buildTreeFromText(const BehaviorTreeFactory& factory,
                       const std::string& text,
                       const Blackboard::Ptr& blackboard = Blackboard::Ptr());

/** Helper function to do the most common steps all at once:
* 1) Create an instance of XMLParse and call loadFromFile.
* 2) Instantiate the entire tree.
*
*/
Tree buildTreeFromFile(const BehaviorTreeFactory& factory,
                       const std::string& filename,
                       const Blackboard::Ptr& blackboard = Blackboard::Ptr());

std::string writeXML(const BehaviorTreeFactory& factory,
                     const TreeNode* root_node,
                     bool compact_representation = false);
}

#endif   // XML_PARSING_BT_H
