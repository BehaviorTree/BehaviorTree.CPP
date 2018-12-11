/*  Copyright (C) 2018 Davide Faconti -  All Rights Reserved
*
*   Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"),
*   to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
*   and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
*   The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
*
*   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
*   WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <functional>
#include <list>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wattributes"
#include "behaviortree_cpp/xml_parsing.h"
#include "tinyXML2/tinyxml2.h"
#include "filesystem/path.h"

#ifdef USING_ROS
#include <ros/package.h>
#endif

namespace BT
{

struct XMLParser::Pimpl
{
    TreeNode::Ptr treeParsing(const tinyxml2::XMLElement* root_element,
                              const NodeBuilder& node_builder,
                              std::vector<TreeNode::Ptr>& nodes,
                              const TreeNode::Ptr& root_parent);

    void openIncludedFiles();

    void loadDocImpl(tinyxml2::XMLDocument *doc);

    void verifyXML(const tinyxml2::XMLDocument* doc) const;

    std::list< std::unique_ptr<tinyxml2::XMLDocument>> opened_documents;
    std::map<std::string,const tinyxml2::XMLElement*> tree_roots;

    const BehaviorTreeFactory& factory;

    filesystem::path current_path;

    int suffix_count;

    Pimpl(const BehaviorTreeFactory &fact):
        factory(fact),
        current_path( filesystem::path::getcwd() ),
        suffix_count(0)
    {}

};
#pragma GCC diagnostic pop

XMLParser::XMLParser(const BehaviorTreeFactory &factory) : _p( new Pimpl(factory) )
{
}

XMLParser::~XMLParser()
{
    delete _p;
}

void XMLParser::loadFromFile(const std::string& filename)
{
    _p->opened_documents.emplace_back( new tinyxml2::XMLDocument() );

    tinyxml2::XMLDocument* doc = _p->opened_documents.back().get();
    doc->LoadFile(filename.c_str());

    filesystem::path file_path( filename );
    _p->current_path = file_path.parent_path().make_absolute();

    _p->loadDocImpl( doc );
}

void XMLParser::loadFromText(const std::string& xml_text)
{
    _p->opened_documents.emplace_back( new tinyxml2::XMLDocument() );

    tinyxml2::XMLDocument* doc = _p->opened_documents.back().get();
    doc->Parse(xml_text.c_str(), xml_text.size());
    _p->loadDocImpl( doc );
}

void XMLParser::Pimpl::loadDocImpl(tinyxml2::XMLDocument* doc)
{
    if (doc->Error())
    {
        char buffer[200];
        sprintf(buffer, "Error parsing the XML: %s", doc->ErrorName() );
        throw std::runtime_error(buffer);
    }

    const tinyxml2::XMLElement* xml_root = doc->RootElement();

    // recursively include other files
    for (auto include_node = xml_root->FirstChildElement("include");
         include_node != nullptr;
         include_node = include_node->NextSiblingElement("include"))
    {

        filesystem::path file_path( include_node->Attribute("path") );

        if( include_node->Attribute("ros_pkg") )
        {
#ifdef USING_ROS
            if( file_path.is_absolute() )
            {
                std::cout << "WARNING: <include path=\"...\"> containes an absolute path.\n"
                          << "Attribute [ros_pkg] will be ignored."<< std::endl;
            }
            else {
                auto ros_pkg_path = ros::package::getPath(  include_node->Attribute("ros_pkg") );
                file_path = filesystem::path( ros_pkg_path ) / file_path;
            }
#else
            throw std::runtime_error("Using attribute [ros_pkg] in <include>, but this library was compiled "
                                     "without ROS support. Recompile the BehaviorTree.CPP using catkin");
#endif
        }

        if( !file_path.is_absolute() )
        {
            file_path = current_path / file_path;
        }

        opened_documents.emplace_back( new tinyxml2::XMLDocument() );
        tinyxml2::XMLDocument* doc = opened_documents.back().get();
        doc->LoadFile(file_path.str().c_str());
        loadDocImpl( doc );
    }

    for (auto bt_node = xml_root->FirstChildElement("BehaviorTree");
         bt_node != nullptr;
         bt_node = bt_node->NextSiblingElement("BehaviorTree"))
    {
        std::string tree_name;
        if (bt_node->Attribute("ID"))
        {
            tree_name = bt_node->Attribute("ID");
        }
        else{
            tree_name = "BehaviorTree_" + std::to_string( suffix_count++ );
        }
        tree_roots.insert( {tree_name, bt_node} );
    }
    verifyXML(doc);
}

void XMLParser::Pimpl::verifyXML(const tinyxml2::XMLDocument* doc) const
{
    //-------- Helper functions (lambdas) -----------------
    auto StrEqual = [](const char* str1, const char* str2) -> bool {
        return strcmp(str1, str2) == 0;
    };

    auto ThrowError = [&](int line_num, const std::string& text) {
        char buffer[256];
        sprintf(buffer, "Error at line %d: -> %s", line_num, text.c_str());
        throw std::runtime_error( buffer );
    };

    auto ChildrenCount = [](const tinyxml2::XMLElement* parent_node) {
        int count = 0;
        for (auto node = parent_node->FirstChildElement(); node != nullptr;
             node = node->NextSiblingElement())
        {
            count++;
        }
        return count;
    };
    //-----------------------------

    const tinyxml2::XMLElement* xml_root = doc->RootElement();

    if (!xml_root || !StrEqual(xml_root->Name(), "root"))
    {
        throw std::runtime_error("The XML must have a root node called <root>");
    }
    //-------------------------------------------------
    auto meta_root = xml_root->FirstChildElement("TreeNodesModel");
    auto meta_sibling = meta_root ? meta_root->NextSiblingElement("TreeNodesModel") : nullptr;

    if (meta_sibling)
    {
        ThrowError(meta_sibling->GetLineNum(), " Only a single node <TreeNodesModel> is "
                                                "supported");
    }
    if (meta_root)
    {
        // not having a MetaModel is not an error. But consider that the
        // Graphical editor needs it.
        for (auto node = xml_root->FirstChildElement(); node != nullptr;
             node = node->NextSiblingElement())
        {
            const char* name = node->Name();
            if (StrEqual(name, "Action") || StrEqual(name, "Decorator") ||
                StrEqual(name, "SubTree") || StrEqual(name, "Condition"))
            {
                const char* ID = node->Attribute("ID");
                if (!ID)
                {
                    ThrowError(node->GetLineNum(), "Error at line %d: -> The attribute [ID] is "
                                                    "mandatory");
                }
            }
        }
    }
    //-------------------------------------------------

    // function to be called recursively
    std::function<void(const tinyxml2::XMLElement*)> recursiveStep;

    recursiveStep = [&](const tinyxml2::XMLElement* node) {
        const int children_count = ChildrenCount(node);
        const char* name = node->Name();
        if (StrEqual(name, "Decorator"))
        {
            if (children_count != 1)
            {
                ThrowError(node->GetLineNum(), "The node <Decorator> must have exactly 1 child");
            }
            if (!node->Attribute("ID"))
            {
                ThrowError(node->GetLineNum(), "The node <Decorator> must have the attribute "
                                                "[ID]");
            }
        }
        else if (StrEqual(name, "Action"))
        {
            if (children_count != 0)
            {
                ThrowError(node->GetLineNum(), "The node <Action> must not have any child");
            }
            if (!node->Attribute("ID"))
            {
                ThrowError(node->GetLineNum(), "The node <Action> must have the attribute [ID]");
            }
        }
        else if (StrEqual(name, "Condition"))
        {
            if (children_count != 0)
            {
                ThrowError(node->GetLineNum(), "The node <Condition> must not have any child");
            }
            if (!node->Attribute("ID"))
            {
                ThrowError(node->GetLineNum(), "The node <Condition> must have the attribute "
                                                "[ID]");
            }
        }
        else if (StrEqual(name, "Sequence") || StrEqual(name, "SequenceStar") ||
                 StrEqual(name, "Fallback") || StrEqual(name, "FallbackStar"))
        {
            if (children_count == 0)
            {
                ThrowError(node->GetLineNum(), "A Control node must have at least 1 child");
            }
        }
        else if (StrEqual(name, "SubTree"))
        {
            if (children_count > 0)
            {
                ThrowError(node->GetLineNum(), "The <SubTree> node must have no children");
            }
            if (!node->Attribute("ID"))
            {
                ThrowError(node->GetLineNum(), "The node <SubTree> must have the attribute [ID]");
            }
        }
        else
        {
            // Last resort:  MAYBE used ID as element name?
            bool found = false;
            for (const auto& model : factory.manifests())
            {
                if (model.registration_ID == name)
                {
                    found = true;
                    break;
                }
            }
            for (const auto& subtrees_it : tree_roots)
            {
                if (subtrees_it.first == name)
                {
                    found = true;
                    break;
                }
            }
            if (!found)
            {
                ThrowError(node->GetLineNum(), std::string("Node not recognized: ") + name);
            }
        }
        //recursion
        for (auto child = node->FirstChildElement(); child != nullptr;
             child = child->NextSiblingElement())
        {
            recursiveStep(child);
        }
    };

    std::vector<std::string> tree_names;
    int tree_count = 0;

    for (auto bt_root = xml_root->FirstChildElement("BehaviorTree"); bt_root != nullptr;
         bt_root = bt_root->NextSiblingElement("BehaviorTree"))
    {
        tree_count++;
        if (bt_root->Attribute("ID"))
        {
            tree_names.push_back(bt_root->Attribute("ID"));
        }
        if (ChildrenCount(bt_root) != 1)
        {
            ThrowError(bt_root->GetLineNum(), "The node <BehaviorTree> must have exactly 1 child");
        }
        else
        {
            recursiveStep(bt_root->FirstChildElement());
        }
    }

    if (xml_root->Attribute("main_tree_to_execute"))
    {
        std::string main_tree = xml_root->Attribute("main_tree_to_execute");
        if (std::find(tree_names.begin(), tree_names.end(), main_tree) == tree_names.end())
        {
            throw std::runtime_error("The tree esecified in [main_tree_to_execute] can't be found");
        }
    }
    else
    {
        if (tree_count != 1)
        {
            throw std::runtime_error(
                "If you don't specify the attribute [main_tree_to_execute], "
                "Your file must contain a single BehaviorTree");
        }
    }
}

TreeNode::Ptr XMLParser::instantiateTree(std::vector<TreeNode::Ptr>& nodes,
                                         const Blackboard::Ptr& blackboard)
{
    nodes.clear();

    tinyxml2::XMLElement* xml_root = _p->opened_documents.front()->RootElement();

    std::string main_tree_ID;
    if (xml_root->Attribute("main_tree_to_execute"))
    {
        main_tree_ID = xml_root->Attribute("main_tree_to_execute");
    }
    else if( _p->tree_roots.size() == 1)
    {
        main_tree_ID = _p->tree_roots.begin()->first;
    }
    else{
        throw std::runtime_error("[main_tree_to_execute] was not specified correctly");
    }

    //--------------------------------------
    NodeBuilder node_builder = [&](const std::string& ID, const std::string& name,
                                   const NodeParameters& params,
                                   TreeNode::Ptr parent) -> TreeNode::Ptr
    {


        TreeNode::Ptr child_node;

        if( _p->factory.builders().count(ID) != 0)
        {
            child_node = _p->factory.instantiateTreeNode(ID, name, params, blackboard);
        }
        else if( _p->tree_roots.count(ID) != 0) {
            child_node = std::unique_ptr<TreeNode>( new DecoratorSubtreeNode(name) );
        }
        else{
            throw std::runtime_error( ID + " is not a registered node, nor a Subtree");
        }

        nodes.push_back(child_node);
        if (parent)
        {
            ControlNode* control_parent = dynamic_cast<ControlNode*>(parent.get());
            if (control_parent)
            {
                control_parent->addChild(child_node.get());
            }
            DecoratorNode* decorator_parent = dynamic_cast<DecoratorNode*>(parent.get());
            if (decorator_parent)
            {
                decorator_parent->setChild(child_node.get());
            }
        }
        DecoratorSubtreeNode* subtree_node = dynamic_cast<DecoratorSubtreeNode*>(child_node.get());

        if (subtree_node)
        {
            auto subtree_elem = _p->tree_roots[name]->FirstChildElement();
            _p->treeParsing(subtree_elem, node_builder, nodes, child_node);
        }
        return child_node;
    };
    //--------------------------------------

    auto root_element = _p->tree_roots[main_tree_ID]->FirstChildElement();
    return _p->treeParsing(root_element, node_builder, nodes, TreeNode::Ptr());
}

TreeNode::Ptr BT::XMLParser::Pimpl::treeParsing(const tinyxml2::XMLElement* root_element,
                                                const NodeBuilder& node_builder,
                                                std::vector<TreeNode::Ptr>& nodes,
                                                const TreeNode::Ptr& root_parent)
{
    using namespace tinyxml2;

    std::function<TreeNode::Ptr(const TreeNode::Ptr&, const tinyxml2::XMLElement*)> recursiveStep;

    recursiveStep = [&](const TreeNode::Ptr& parent,
                        const tinyxml2::XMLElement* element) -> TreeNode::Ptr {
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

        TreeNode::Ptr node = node_builder(node_ID, node_alias, node_params, parent);
        nodes.push_back(node);

        for (auto child_element = element->FirstChildElement(); child_element;
             child_element = child_element->NextSiblingElement())
        {
            recursiveStep(node, child_element);
        }

        return node;
    };

    // start recursion
    TreeNode::Ptr root = recursiveStep(root_parent, root_element);
    return root;
}

void XMLParser::Pimpl::openIncludedFiles()
{

}

std::string writeXML(const BehaviorTreeFactory& factory,
                     const TreeNode* root_node,
                     bool compact_representation)
{
    using namespace tinyxml2;

    XMLDocument doc;

    XMLElement* rootXML = doc.NewElement("root");
    doc.InsertFirstChild(rootXML);

    if (root_node)
    {
        XMLElement* bt_root = doc.NewElement("BehaviorTree");
        rootXML->InsertEndChild(bt_root);

        std::function<void(const TreeNode*, XMLElement* parent)> recursiveVisitor;

        recursiveVisitor = [&recursiveVisitor, &doc, compact_representation,
                            &factory](const TreeNode* node, XMLElement* parent) -> void {
            std::string node_type = toStr(node->type());
            std::string node_ID = node->registrationName();
            std::string node_name = node->name();

            if (node->type() == NodeType::CONTROL)
            {
                node_type = node_ID;
            }
            else if (compact_representation)
            {
                for (const auto& model : factory.manifests())
                {
                    if (model.registration_ID == node_ID)
                    {
                        node_type = node_ID;
                        break;
                    }
                }
            }

            XMLElement* element = doc.NewElement(node_type.c_str());
            if (node_type != node_ID && !node_ID.empty())
            {
                element->SetAttribute("ID", node_ID.c_str());
            }
            if (node_type != node_name && !node_name.empty() && node_name != node_ID)
            {
                element->SetAttribute("name", node_name.c_str());
            }

            for (const auto& param : node->initializationParameters())
            {
                element->SetAttribute(param.first.c_str(), param.second.c_str());
            }

            parent->InsertEndChild(element);

            if (auto control = dynamic_cast<const BT::ControlNode*>(node))
            {
                for (const auto& child : control->children())
                {
                    recursiveVisitor(static_cast<const TreeNode*>(child), element);
                }
            }
            else if (auto decorator = dynamic_cast<const BT::DecoratorNode*>(node))
            {
                recursiveVisitor(decorator->child(), element);
            }
        };

        recursiveVisitor(root_node, bt_root);
    }
    //--------------------------

    XMLElement* model_root = doc.NewElement("TreeNodesModel");
    rootXML->InsertEndChild(model_root);

    for (auto& model : factory.manifests())
    {
        if( factory.builtinNodes().count( model.registration_ID ) != 0)
        {
            continue;
        }

        if (model.type == NodeType::CONTROL)
        {
            continue;
        }
        XMLElement* element = doc.NewElement(toStr(model.type));
        element->SetAttribute("ID", model.registration_ID.c_str());

        for (auto& param : model.required_parameters)
        {
            element->SetAttribute( param.first.c_str(),
                                   param.second.c_str() );
        }

        model_root->InsertEndChild(element);
    }

    tinyxml2::XMLPrinter printer;
    doc.Print(&printer);
    return std::string(printer.CStr(), printer.CStrSize() - 1);
}

Tree buildTreeFromText(const BehaviorTreeFactory& factory, const std::string& text,
                       const Blackboard::Ptr& blackboard)
{
    XMLParser parser(factory);
    parser.loadFromText(text);

    std::vector<TreeNode::Ptr> nodes;
    auto root = parser.instantiateTree(nodes, blackboard);

    return Tree(root.get(), nodes);
}

Tree buildTreeFromFile(const BehaviorTreeFactory& factory, const std::string& filename,
                       const Blackboard::Ptr& blackboard)
{
    XMLParser parser(factory);
    parser.loadFromFile(filename);

    std::vector<TreeNode::Ptr> nodes;
    auto root = parser.instantiateTree(nodes, blackboard);
    return Tree(root.get(), nodes);
}

}
