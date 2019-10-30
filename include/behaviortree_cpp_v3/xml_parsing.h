#ifndef XML_PARSING_BT_H
#define XML_PARSING_BT_H

#include "behaviortree_cpp_v3/bt_parser.h"

namespace BT
{

/**
 * @brief The XMLParser is a class used to read the model
 * of a BehaviorTree from file or text and instantiate the
 * corresponding tree using the BehaviorTreeFactory.
 */
class XMLParser: public Parser
{
  public:
    XMLParser(const BehaviorTreeFactory& factory);

    ~XMLParser();

    XMLParser(const XMLParser& other) = delete;
    XMLParser& operator=(const XMLParser& other) = delete;

    void loadFromFile(const std::string& filename) override;

    void loadFromText(const std::string& xml_text) override;

    Tree instantiateTree(const Blackboard::Ptr &root_blackboard) override;

  private:

    struct Pimpl;
    Pimpl* _p;

};

void VerifyXML(const std::string& xml_text,
               const std::set<std::string> &registered_nodes);

std::string writeTreeNodesModelXML(const BehaviorTreeFactory& factory);

}

#endif   // XML_PARSING_BT_H
