/*  Copyright (C) 2018-2019 Davide Faconti, Eurecat -  All Rights Reserved
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

#if defined(__linux) || defined(__linux__)
	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wattributes"
#endif 

#ifdef _MSC_VER
#pragma warning(disable : 4996) // do not complain about sprintf
#endif

#include "behaviortree_cpp_v3/xml_parsing.h"
#include "private/tinyxml2.h"
#include "filesystem/path.h"

#ifdef USING_ROS
#include <ros/package.h>
#endif

#include "behaviortree_cpp_v3/blackboard.h"
#include "behaviortree_cpp_v3/utils/demangle_util.h"

namespace BT
{
using namespace BT_TinyXML2;

struct XMLParser::Pimpl
{
    TreeNode::Ptr createNodeFromXML(const XMLElement* element,
                                    const Blackboard::Ptr& blackboard,
                                    const TreeNode::Ptr& node_parent);

    void recursivelyCreateTree(const std::string& tree_ID,
                               Tree& output_tree,
                               Blackboard::Ptr blackboard,
                               const TreeNode::Ptr& root_parent);

    void loadDocImpl(BT_TinyXML2::XMLDocument* doc);

    std::list<std::unique_ptr<BT_TinyXML2::XMLDocument> > opened_documents;
    std::unordered_map<std::string,const XMLElement*>  tree_roots;

    const BehaviorTreeFactory& factory;

    filesystem::path current_path;

    int suffix_count;

    explicit Pimpl(const BehaviorTreeFactory &fact):
        factory(fact),
        current_path( filesystem::path::getcwd() ),
        suffix_count(0)
    {}

    void clear()
    {
        suffix_count = 0;
        current_path = filesystem::path::getcwd();
        opened_documents.clear();
        tree_roots.clear();
    }

};

#if defined(__linux) || defined(__linux__)
#pragma GCC diagnostic pop
#endif

XMLParser::XMLParser(const BehaviorTreeFactory &factory) :
    _p( new Pimpl(factory) )
{
}

XMLParser::~XMLParser()
{
    delete _p;
}

void XMLParser::loadFromFile(const std::string& filename)
{
    _p->opened_documents.emplace_back(new BT_TinyXML2::XMLDocument());

    BT_TinyXML2::XMLDocument* doc = _p->opened_documents.back().get();
    doc->LoadFile(filename.c_str());

    filesystem::path file_path( filename );
    _p->current_path = file_path.parent_path().make_absolute();

    _p->loadDocImpl( doc );
}

void XMLParser::loadFromText(const std::string& xml_text)
{
    _p->opened_documents.emplace_back(new BT_TinyXML2::XMLDocument());

    BT_TinyXML2::XMLDocument* doc = _p->opened_documents.back().get();
    doc->Parse(xml_text.c_str(), xml_text.size());

    _p->loadDocImpl( doc );
}

void XMLParser::Pimpl::loadDocImpl(BT_TinyXML2::XMLDocument* doc)
{
    if (doc->Error())
    {
        char buffer[200];
        sprintf(buffer, "Error parsing the XML: %s", doc->ErrorName() );
        throw RuntimeError(buffer);
    }

    const XMLElement* xml_root = doc->RootElement();

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
            throw RuntimeError("Using attribute [ros_pkg] in <include>, but this library was compiled "
                               "without ROS support. Recompile the BehaviorTree.CPP using catkin");
#endif
        }

        if( !file_path.is_absolute() )
        {
            file_path = current_path / file_path;
        }

        opened_documents.emplace_back(new BT_TinyXML2::XMLDocument());
        BT_TinyXML2::XMLDocument* next_doc = opened_documents.back().get();
        next_doc->LoadFile(file_path.str().c_str());
        loadDocImpl(next_doc);
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

    std::set<std::string> registered_nodes;
    XMLPrinter printer;
    doc->Print(&printer);
    auto xml_text = std::string(printer.CStr(), size_t(printer.CStrSize() - 1));

    for( const auto& it: factory.manifests())
    {
        registered_nodes.insert( it.first );
    }
    for( const auto& it: tree_roots)
    {
        registered_nodes.insert( it.first );
    }

    VerifyXML(xml_text, registered_nodes);
}

void VerifyXML(const std::string& xml_text,
               const std::set<std::string>& registered_nodes)
{

    BT_TinyXML2::XMLDocument doc;
    auto xml_error = doc.Parse( xml_text.c_str(), xml_text.size());
    if (xml_error)
    {
        char buffer[200];
        sprintf(buffer, "Error parsing the XML: %s", doc.ErrorName() );
        throw RuntimeError( buffer );
    }

    //-------- Helper functions (lambdas) -----------------
    auto StrEqual = [](const char* str1, const char* str2) -> bool {
        return strcmp(str1, str2) == 0;
    };

    auto ThrowError = [&](int line_num, const std::string& text) {
        char buffer[256];
        sprintf(buffer, "Error at line %d: -> %s", line_num, text.c_str());
        throw RuntimeError( buffer );
    };

    auto ChildrenCount = [](const XMLElement* parent_node) {
        int count = 0;
        for (auto node = parent_node->FirstChildElement(); node != nullptr;
             node = node->NextSiblingElement())
        {
            count++;
        }
        return count;
    };
    //-----------------------------

    const XMLElement* xml_root = doc.RootElement();

    if (!xml_root || !StrEqual(xml_root->Name(), "root"))
    {
        throw RuntimeError("The XML must have a root node called <root>");
    }
    //-------------------------------------------------
    auto models_root = xml_root->FirstChildElement("TreeNodesModel");
    auto meta_sibling = models_root ? models_root->NextSiblingElement("TreeNodesModel") : nullptr;

    if (meta_sibling)
    {
       ThrowError(meta_sibling->GetLineNum(),
                           " Only a single node <TreeNodesModel> is supported");
    }
    if (models_root)
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
                   ThrowError(node->GetLineNum(),
                                       "Error at line %d: -> The attribute [ID] is mandatory");
                }
            }
        }
    }
    //-------------------------------------------------

    // function to be called recursively
    std::function<void(const XMLElement*)> recursiveStep;

    recursiveStep = [&](const XMLElement* node) {
        const int children_count = ChildrenCount(node);
        const char* name = node->Name();
        if (StrEqual(name, "Decorator"))
        {
            if (children_count != 1)
            {
               ThrowError(node->GetLineNum(),
                                   "The node <Decorator> must have exactly 1 child");
            }
            if (!node->Attribute("ID"))
            {
               ThrowError(node->GetLineNum(),
                                   "The node <Decorator> must have the attribute [ID]");
            }
        }
        else if (StrEqual(name, "Action"))
        {
            if (children_count != 0)
            {
               ThrowError(node->GetLineNum(),
                                   "The node <Action> must not have any child");
            }
            if (!node->Attribute("ID"))
            {
               ThrowError(node->GetLineNum(),
                                   "The node <Action> must have the attribute [ID]");
            }
        }
        else if (StrEqual(name, "Condition"))
        {
            if (children_count != 0)
            {
               ThrowError(node->GetLineNum(),
                                   "The node <Condition> must not have any child");
            }
            if (!node->Attribute("ID"))
            {
               ThrowError(node->GetLineNum(),
                                   "The node <Condition> must have the attribute [ID]");
            }
        }
        else if (StrEqual(name, "Sequence") ||
                 StrEqual(name, "SequenceStar") ||
                 StrEqual(name, "Fallback") )
        {
            if (children_count == 0)
            {
               ThrowError(node->GetLineNum(),
                                   "A Control node must have at least 1 child");
            }
        }
        else if (StrEqual(name, "SubTree"))
        {
            for (auto child = node->FirstChildElement(); child != nullptr;
                 child = child->NextSiblingElement())
            {
                if( StrEqual(child->Name(), "remap") )
                {
                   ThrowError(node->GetLineNum(), "<remap> was deprecated");
                }
                else{
                    ThrowError(node->GetLineNum(), "<SubTree> should not have any child");
                }
            }

            if (!node->Attribute("ID"))
            {
               ThrowError(node->GetLineNum(),
                                   "The node <SubTree> must have the attribute [ID]");
            }
        }
        else
        {
            // search in the factory and the list of subtrees
            bool found = ( registered_nodes.find(name)  != registered_nodes.end() );
            if (!found)
            {
               ThrowError(node->GetLineNum(),
                          std::string("Node not recognized: ") + name);
            }
        }
        //recursion
        if (StrEqual(name, "SubTree") == false)
        {           
            for (auto child = node->FirstChildElement(); child != nullptr;
                 child = child->NextSiblingElement())
            {
                recursiveStep(child);
            }
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
            tree_names.emplace_back(bt_root->Attribute("ID"));
        }
        if (ChildrenCount(bt_root) != 1)
        {
           ThrowError(bt_root->GetLineNum(),
                               "The node <BehaviorTree> must have exactly 1 child");
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
            throw RuntimeError("The tree specified in [main_tree_to_execute] can't be found");
        }
    }
    else
    {
        if (tree_count != 1)
        {
           throw RuntimeError("If you don't specify the attribute [main_tree_to_execute], "
                              "Your file must contain a single BehaviorTree");
        }
    }
}

Tree XMLParser::instantiateTree(const Blackboard::Ptr& root_blackboard)
{
    Tree output_tree;

    XMLElement* xml_root = _p->opened_documents.front()->RootElement();

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
        throw RuntimeError("[main_tree_to_execute] was not specified correctly");
    }
    //--------------------------------------
    if( !root_blackboard )
    {
        throw RuntimeError("XMLParser::instantiateTree needs a non-empty root_blackboard");
    }
    // first blackboard
    output_tree.blackboard_stack.push_back( root_blackboard );

    _p->recursivelyCreateTree(main_tree_ID,
                              output_tree,
                              root_blackboard,
                              TreeNode::Ptr() );

    if( output_tree.nodes.size() > 0)
    {
        output_tree.root_node = output_tree.nodes.front().get();
    }
    return output_tree;
}

TreeNode::Ptr XMLParser::Pimpl::createNodeFromXML(const XMLElement *element,
                                                  const Blackboard::Ptr &blackboard,
                                                  const TreeNode::Ptr &node_parent)
{
    const std::string element_name = element->Name();
    std::string ID;
    std::string instance_name;

    // Actions and Decorators have their own ID
    if (element_name == "Action" || element_name == "Decorator" || element_name == "Condition")
    {
        ID = element->Attribute("ID");
    }
    else
    {
        ID = element_name;
    }

    const char* attr_alias = element->Attribute("name");
    if (attr_alias)
    {
        instance_name = attr_alias;
    }
    else
    {
        instance_name = ID;
    }

    if (element_name == "SubTree")
    {
        instance_name = element->Attribute("ID");
    }

    PortsRemapping remapping_parameters;

    if (element_name != "SubTree") // in Subtree attributes have different meaning...
    {
        for (const XMLAttribute* att = element->FirstAttribute(); att; att = att->Next())
        {
            const std::string attribute_name = att->Name();
            if (attribute_name != "ID" && attribute_name != "name")
            {
                remapping_parameters[attribute_name] = att->Value();
            }
        }
    }
    NodeConfiguration config;
    config.blackboard = blackboard;

    //---------------------------------------------
    TreeNode::Ptr child_node;

    if( factory.builders().count(ID) != 0)
    {
        const auto& manifest = factory.manifests().at(ID);

        //Check that name in remapping can be found in the manifest
        for(const auto& remapping_it: remapping_parameters)
        {
            if( manifest.ports.count( remapping_it.first ) == 0 )
            {
                throw RuntimeError("Possible typo? In the XML, you tried to remap port \"",
                                   remapping_it.first, "\" in node [", ID," / ", instance_name,
                                   "], but the manifest of this node does not contain a port with this name.");
            }
        }

        // Initialize the ports in the BB to set the type
        for(const auto& port_it: manifest.ports)
        {
            const std::string& port_name = port_it.first;
            const auto& port_info = port_it.second;

            auto remap_it = remapping_parameters.find(port_name);
            if( remap_it == remapping_parameters.end())
            {
                continue;
            }
            StringView remapping_value = remap_it->second;
            auto remapped_res = TreeNode::getRemappedKey(port_name, remapping_value);
            if( remapped_res )
            {
                const auto& port_key = nonstd::to_string(remapped_res.value());

                auto prev_info = blackboard->portInfo( port_key );
                if( !prev_info  )
                {
                    // not found, insert for the first time.
                    blackboard->setPortInfo( port_key, port_info );
                }
                else{
                    // found. check consistency
                    if( prev_info->type() && port_info.type()  && // null type means that everything is valid
                        prev_info->type()!= port_info.type())
                    {
                        blackboard->debugMessage();

                        throw RuntimeError( "The creation of the tree failed because the port [", port_key,
                                           "] was initially created with type [", demangle( prev_info->type() ),
                                           "] and, later type [", demangle( port_info.type() ),
                                           "] was used somewhere else." );
                    }
                }
            }
        }

        // use manifest to initialize NodeConfiguration
        for(const auto& remap_it: remapping_parameters)
        {
            const auto& port_name = remap_it.first;
            auto port_it = manifest.ports.find( port_name );
            if( port_it != manifest.ports.end() )
            {
                auto direction = port_it->second.direction();
                if( direction != PortDirection::OUTPUT )
                {
                    config.input_ports.insert( remap_it );
                }
                if( direction != PortDirection::INPUT )
                {
                    config.output_ports.insert( remap_it );
                }
            }
        }
        // use default value if available for empty ports. Only inputs
        for (const auto& port_it: manifest.ports)
        {
            const std::string& port_name =  port_it.first;
            const PortInfo& port_info = port_it.second;

            auto direction = port_info.direction();
            if( direction != PortDirection::INPUT &&
                config.input_ports.count(port_name) == 0 &&
                port_info.defaultValue().empty() == false)
            {
                config.input_ports.insert( { port_name, port_info.defaultValue() } );
            }
        }
        child_node = factory.instantiateTreeNode(instance_name, ID, config);
    }
    else if( tree_roots.count(ID) != 0) {
        child_node = std::make_unique<DecoratorSubtreeNode>( instance_name );
    }
    else{
        throw RuntimeError( ID, " is not a registered node, nor a Subtree");
    }

    if (node_parent)
    {
        if (auto control_parent = dynamic_cast<ControlNode*>(node_parent.get()))
        {
            control_parent->addChild(child_node.get());
        }
        if (auto decorator_parent = dynamic_cast<DecoratorNode*>(node_parent.get()))
        {
            decorator_parent->setChild(child_node.get());
        }
    }
    return child_node;
}

void BT::XMLParser::Pimpl::recursivelyCreateTree(const std::string& tree_ID,
                                                 Tree& output_tree,
                                                 Blackboard::Ptr blackboard,
                                                 const TreeNode::Ptr& root_parent)
{
    std::function<void(const TreeNode::Ptr&, const XMLElement*)> recursiveStep;

    recursiveStep = [&](const TreeNode::Ptr& parent,
                        const XMLElement* element)
    {
        auto node = createNodeFromXML(element, blackboard, parent);
        output_tree.nodes.push_back(node);

        if( node->type() == NodeType::SUBTREE )
        {
            auto new_bb = Blackboard::create(blackboard);

            for (const XMLAttribute* attr = element->FirstAttribute(); attr != nullptr; attr = attr->Next())
            {
                new_bb->addSubtreeRemapping( attr->Name(), attr->Value() );
            }

            output_tree.blackboard_stack.emplace_back(new_bb);
            recursivelyCreateTree( node->name(), output_tree, new_bb, node );
        }
        else
        {
            for (auto child_element = element->FirstChildElement(); child_element;
                 child_element = child_element->NextSiblingElement())
            {
                recursiveStep(node, child_element);
            }
        }
    };

    auto root_element = tree_roots[tree_ID]->FirstChildElement();

    // start recursion
    recursiveStep(root_parent, root_element);
}


std::string writeTreeNodesModelXML(const BehaviorTreeFactory& factory)
{
    using namespace BT_TinyXML2;

    BT_TinyXML2::XMLDocument doc;

    XMLElement* rootXML = doc.NewElement("root");
    doc.InsertFirstChild(rootXML);

    XMLElement* model_root = doc.NewElement("TreeNodesModel");
    rootXML->InsertEndChild(model_root);

    for (auto& model_it : factory.manifests())
    {
        const auto& registration_ID = model_it.first;
        const auto& model = model_it.second;

        if( factory.builtinNodes().count( registration_ID ) != 0)
        {
            continue;
        }

        if (model.type == NodeType::CONTROL)
        {
            continue;
        }
        XMLElement* element = doc.NewElement( toStr(model.type).c_str() );
        element->SetAttribute("ID", model.registration_ID.c_str());

        for (auto& port : model.ports)
        {
            const auto& port_name = port.first;
            const auto& port_info = port.second;

            XMLElement* port_element = nullptr;
            switch(  port_info.direction() )
            {
            case PortDirection::INPUT:  port_element = doc.NewElement("input_port");  break;
            case PortDirection::OUTPUT: port_element = doc.NewElement("output_port"); break;
            case PortDirection::INOUT:  port_element = doc.NewElement("inout_port");  break;
            }

            port_element->SetAttribute("name", port_name.c_str() );
            if( port_info.type() )
            {
                port_element->SetAttribute("type", BT::demangle( port_info.type() ).c_str() );
            }
            if( !port_info.defaultValue().empty() )
            {
                port_element->SetAttribute("default", port_info.defaultValue().c_str() );
            }

            if( !port_info.description().empty() )
            {
                port_element->SetText( port_info.description().c_str() );
            }

            element->InsertEndChild(port_element);
        }

        model_root->InsertEndChild(element);
    }

    XMLPrinter printer;
    doc.Print(&printer);
    return std::string(printer.CStr(), size_t(printer.CStrSize() - 1));
}

Tree buildTreeFromText(const BehaviorTreeFactory& factory, const std::string& text,
                       const Blackboard::Ptr& blackboard)
{
    XMLParser parser(factory);
    parser.loadFromText(text);
    return parser.instantiateTree(blackboard);
}

Tree buildTreeFromFile(const BehaviorTreeFactory& factory, const std::string& filename,
                       const Blackboard::Ptr& blackboard)
{
    XMLParser parser(factory);
    parser.loadFromFile(filename);
    return parser.instantiateTree(blackboard);
}

}
