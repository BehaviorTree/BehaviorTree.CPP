#ifndef PARSING_BT_H
#define PARSING_BT_H

#include "behaviortree_cpp_v3/bt_factory.h"
#include "behaviortree_cpp_v3/blackboard.h"

namespace BT
{
/**
 * @brief The BehaviorTreeParser is a class used to read the model
 * of a BehaviorTree from file or text and instantiate the
 * corresponding tree using the BehaviorTreeFactory.
 */
class Parser
{
  public:
    Parser() = default;

    virtual ~Parser() = default;

    Parser(const Parser& other) = delete;
    Parser& operator=(const Parser& other) = delete;

    virtual void loadFromFile(const std::string& filename, bool add_includes = true) = 0;

    virtual void loadFromText(const std::string& xml_text, bool add_includes = true) = 0;

    virtual std::vector<std::string> registeredBehaviorTrees() const = 0;

    virtual Tree instantiateTree(const Blackboard::Ptr &root_blackboard, std::string tree_name = {}) = 0;
};

}

#endif   // PARSING_BT_H
