#ifndef XML_PARSING_BT_H
#define XML_PARSING_BT_H

#include "behaviortree_cpp_v3/bt_parser.h"

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

  void loadFromFile(const std::string& filename, bool add_includes = true) override;

  void loadFromText(const std::string& xml_text, bool add_includes = true) override;

  std::vector<std::string> registeredBehaviorTrees() const override;

  Tree instantiateTree(const Blackboard::Ptr& root_blackboard,
                       std::string main_tree_to_execute = {}) override;

  void clearInternalState() override;

private:
  struct Pimpl;
  Pimpl* _p;
};

void VerifyXML(const std::string& xml_text,
               const std::unordered_map<std::string, NodeType>& registered_nodes);

std::string writeTreeNodesModelXML(const BehaviorTreeFactory& factory,
                                   bool include_builtin = false);

}   // namespace BT

#endif   // XML_PARSING_BT_H
