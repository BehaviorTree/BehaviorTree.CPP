/*  Copyright (C) 2018-2020 Davide Faconti, Eurecat -  All Rights Reserved
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
#pragma warning(disable : 4996)   // do not complain about sprintf
#endif

#include <map>
#include "behaviortree_cpp_v3/xml_parsing.h"
#include "tinyxml2/tinyxml2.h"
#include "filesystem/path.h"

#ifdef USING_ROS
#include <ros/package.h>
#endif

#ifdef USING_ROS2
#include <ament_index_cpp/get_package_share_directory.hpp>
#endif

#include "behaviortree_cpp_v3/blackboard.h"
#include "behaviortree_cpp_v3/utils/demangle_util.h"

namespace BT
{
using namespace tinyxml2;

auto StrEqual = [](const char* str1, const char* str2) -> bool {
  return strcmp(str1, str2) == 0;
};

struct XMLParser::Pimpl
{
  TreeNode::Ptr createNodeFromXML(const XMLElement* element,
                                  const Blackboard::Ptr& blackboard,
                                  const TreeNode::Ptr& node_parent);

  void recursivelyCreateTree(const std::string& tree_ID, Tree& output_tree,
                             Blackboard::Ptr blackboard,
                             const TreeNode::Ptr& root_parent);

  void getPortsRecursively(const XMLElement* element,
                           std::vector<std::string>& output_ports);

  void loadDocImpl(tinyxml2::XMLDocument* doc, bool add_includes);

  std::list<std::unique_ptr<tinyxml2::XMLDocument> > opened_documents;
  std::map<std::string, const XMLElement*> tree_roots;

  const BehaviorTreeFactory& factory;

  filesystem::path current_path;

  int suffix_count;

  explicit Pimpl(const BehaviorTreeFactory& fact) :
    factory(fact), current_path(filesystem::path::getcwd()), suffix_count(0)
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

XMLParser::XMLParser(const BehaviorTreeFactory& factory) : _p(new Pimpl(factory))
{}

XMLParser::~XMLParser()
{
  delete _p;
}

void XMLParser::loadFromFile(const std::string& filename, bool add_includes)
{
  _p->opened_documents.emplace_back(new tinyxml2::XMLDocument());

  tinyxml2::XMLDocument* doc = _p->opened_documents.back().get();
  doc->LoadFile(filename.c_str());

  filesystem::path file_path(filename);
  _p->current_path = file_path.parent_path().make_absolute();

  _p->loadDocImpl(doc, add_includes);
}

void XMLParser::loadFromText(const std::string& xml_text, bool add_includes)
{
  _p->opened_documents.emplace_back(new tinyxml2::XMLDocument());

  tinyxml2::XMLDocument* doc = _p->opened_documents.back().get();
  doc->Parse(xml_text.c_str(), xml_text.size());

  _p->loadDocImpl(doc, add_includes);
}

std::vector<std::string> XMLParser::registeredBehaviorTrees() const
{
  std::vector<std::string> out;
  for (const auto& it : _p->tree_roots)
  {
    out.push_back(it.first);
  }
  return out;
}

void XMLParser::Pimpl::loadDocImpl(tinyxml2::XMLDocument* doc, bool add_includes)
{
  if (doc->Error())
  {
    char buffer[200];
    sprintf(buffer, "Error parsing the XML: %s", doc->ErrorName());
    throw RuntimeError(buffer);
  }

  const XMLElement* xml_root = doc->RootElement();

  // recursively include other files
  for (auto include_node = xml_root->FirstChildElement("include");
       include_node != nullptr; include_node = include_node->NextSiblingElement("includ"
                                                                                "e"))
  {
    if (!add_includes)
    {
      break;
    }

    filesystem::path file_path(include_node->Attribute("path"));
    const char* ros_pkg_relative_path = include_node->Attribute("ros_pkg");

    if (ros_pkg_relative_path)
    {
      if (file_path.is_absolute())
      {
        std::cout << "WARNING: <include path=\"...\"> contains an absolute "
                     "path.\n"
                  << "Attribute [ros_pkg] will be ignored." << std::endl;
      }
      else
      {
        std::string ros_pkg_path;
#ifdef USING_ROS
        ros_pkg_path = ros::package::getPath(ros_pkg_relative_path);
        file_path = filesystem::path(ros_pkg_path) / file_path;
#elif defined USING_ROS2
        ros_pkg_path =
            ament_index_cpp::get_package_share_directory(ros_pkg_relative_path);
        file_path = filesystem::path(ros_pkg_path) / file_path;
#else
        throw RuntimeError("Using attribute [ros_pkg] in <include>, but this "
                           "library was "
                           "compiled "
                           "without ROS support. Recompile the BehaviorTree.CPP "
                           "using "
                           "catkin");
#endif
      }
    }

    if (!file_path.is_absolute())
    {
      file_path = current_path / file_path;
    }

    opened_documents.emplace_back(new tinyxml2::XMLDocument());
    tinyxml2::XMLDocument* next_doc = opened_documents.back().get();

    // change current path to the included file for handling additional relative paths
    const filesystem::path previous_path = current_path;
    current_path = file_path.parent_path().make_absolute();

    next_doc->LoadFile(file_path.str().c_str());
    loadDocImpl(next_doc, add_includes);

    // reset current path to the previous value
    current_path = previous_path;
  }

  // Collect the names of all nodes registered with the behavior tree factory
  std::unordered_map<std::string, BT::NodeType> registered_nodes;
  for (const auto& it : factory.manifests())
  {
    registered_nodes.insert({it.first, it.second.type});
  }

  XMLPrinter printer;
  doc->Print(&printer);
  auto xml_text = std::string(printer.CStr(), size_t(printer.CStrSize() - 1));

  // Verify the validity of the XML before adding any behavior trees to the parser's list of registered trees
  VerifyXML(xml_text, registered_nodes);

  // Register each BehaviorTree within the XML
  for (auto bt_node = xml_root->FirstChildElement("BehaviorTree"); bt_node != nullptr;
       bt_node = bt_node->NextSiblingElement("BehaviorTree"))
  {
    std::string tree_name;
    if (bt_node->Attribute("ID"))
    {
      tree_name = bt_node->Attribute("ID");
    }
    else
    {
      tree_name = "BehaviorTree_" + std::to_string(suffix_count++);
    }

    tree_roots.insert({tree_name, bt_node});
  }
}

void VerifyXML(const std::string& xml_text,
               const std::unordered_map<std::string, BT::NodeType>& registered_nodes)
{
  tinyxml2::XMLDocument doc;
  auto xml_error = doc.Parse(xml_text.c_str(), xml_text.size());
  if (xml_error)
  {
    char buffer[200];
    sprintf(buffer, "Error parsing the XML: %s", doc.ErrorName());
    throw RuntimeError(buffer);
  }

  //-------- Helper functions (lambdas) -----------------
  auto ThrowError = [&](int line_num, const std::string& text) {
    char buffer[256];
    sprintf(buffer, "Error at line %d: -> %s", line_num, text.c_str());
    throw RuntimeError(buffer);
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
  auto meta_sibling =
      models_root ? models_root->NextSiblingElement("TreeNodesModel") : nullptr;

  if (meta_sibling)
  {
    ThrowError(meta_sibling->GetLineNum(), " Only a single node <TreeNodesModel> is "
                                           "supported");
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
          StrEqual(name, "SubTree") || StrEqual(name, "Condition") ||
          StrEqual(name, "Control"))
      {
        const char* ID = node->Attribute("ID");
        if (!ID)
        {
          ThrowError(node->GetLineNum(), "Error at line %d: -> The attribute "
                                         "[ID] is "
                                         "mandatory");
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
        ThrowError(node->GetLineNum(), "The node <Decorator> must have exactly 1 "
                                       "child");
      }
      if (!node->Attribute("ID"))
      {
        ThrowError(node->GetLineNum(), "The node <Decorator> must have the "
                                       "attribute [ID]");
      }
    }
    else if (StrEqual(name, "Action"))
    {
      if (children_count != 0)
      {
        ThrowError(node->GetLineNum(), "The node <Action> must not have any "
                                       "child");
      }
      if (!node->Attribute("ID"))
      {
        ThrowError(node->GetLineNum(), "The node <Action> must have the "
                                       "attribute [ID]");
      }
    }
    else if (StrEqual(name, "Condition"))
    {
      if (children_count != 0)
      {
        ThrowError(node->GetLineNum(), "The node <Condition> must not have any "
                                       "child");
      }
      if (!node->Attribute("ID"))
      {
        ThrowError(node->GetLineNum(), "The node <Condition> must have the "
                                       "attribute [ID]");
      }
    }
    else if (StrEqual(name, "Control"))
    {
      if (children_count == 0)
      {
        ThrowError(node->GetLineNum(), "The node <Control> must have at least 1 "
                                       "child");
      }
      if (!node->Attribute("ID"))
      {
        ThrowError(node->GetLineNum(), "The node <Control> must have the "
                                       "attribute [ID]");
      }
    }
    else if (StrEqual(name, "Sequence") || StrEqual(name, "SequenceStar") ||
             StrEqual(name, "Fallback"))
    {
      if (children_count == 0)
      {
        ThrowError(node->GetLineNum(), "A Control node must have at least 1 "
                                       "child");
      }
    }
    else if (StrEqual(name, "SubTree"))
    {
      auto child = node->FirstChildElement();

      if (child)
      {
        if (StrEqual(child->Name(), "remap"))
        {
          ThrowError(node->GetLineNum(), "<remap> was deprecated");
        }
        else
        {
          ThrowError(node->GetLineNum(), "<SubTree> should not have any child");
        }
      }

      if (!node->Attribute("ID"))
      {
        ThrowError(node->GetLineNum(), "The node <SubTree> must have the "
                                       "attribute [ID]");
      }
    }
    else if (StrEqual(name, "BehaviorTree"))
    {
      if (children_count != 1)
      {
        ThrowError(node->GetLineNum(), "The node <BehaviorTree> must have exactly 1 "
                                       "child");
      }
    }
    else
    {
      // search in the factory and the list of subtrees
      const auto search = registered_nodes.find(name);
      bool found = (search != registered_nodes.end());
      if (!found)
      {
        ThrowError(node->GetLineNum(), std::string("Node not recognized: ") + name);
      }

      if (search->second == NodeType::DECORATOR)
      {
        if (children_count != 1)
        {
          ThrowError(node->GetLineNum(),
                     std::string("The node <") + name + "> must have exactly 1 child");
        }
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

  for (auto bt_root = xml_root->FirstChildElement("BehaviorTree"); bt_root != nullptr;
       bt_root = bt_root->NextSiblingElement("BehaviorTree"))
  {
    recursiveStep(bt_root);
  }
}

Tree XMLParser::instantiateTree(const Blackboard::Ptr& root_blackboard,
                                std::string main_tree_to_execute)
{
  Tree output_tree;
  std::string main_tree_ID = main_tree_to_execute;

  // use the main_tree_to_execute argument if it was provided by the user
  // or the one in the FIRST document opened
  if (main_tree_ID.empty())
  {
    XMLElement* first_xml_root = _p->opened_documents.front()->RootElement();

    if (auto main_tree_attribute = first_xml_root->Attribute("main_tree_to_execute"))
    {
      main_tree_ID = main_tree_attribute;
    }
    else if (_p->tree_roots.size() == 1)
    {
      // special case: there is only one registered BT.
      main_tree_ID = _p->tree_roots.begin()->first;
    }
    else
    {
      throw RuntimeError("[main_tree_to_execute] was not specified correctly");
    }
  }

  //--------------------------------------
  if (!root_blackboard)
  {
    throw RuntimeError("XMLParser::instantiateTree needs a non-empty "
                       "root_blackboard");
  }
  // first blackboard
  output_tree.blackboard_stack.push_back(root_blackboard);

  _p->recursivelyCreateTree(main_tree_ID, output_tree, root_blackboard, TreeNode::Ptr());
  output_tree.initialize();
  return output_tree;
}

void XMLParser::clearInternalState()
{
    _p->clear();
}

TreeNode::Ptr XMLParser::Pimpl::createNodeFromXML(const XMLElement* element,
                                                  const Blackboard::Ptr& blackboard,
                                                  const TreeNode::Ptr& node_parent)
{
  const std::string element_name = element->Name();
  std::string ID;
  std::string instance_name;

  // Actions and Decorators have their own ID
  if (element_name == "Action" || element_name == "Decorator" ||
      element_name == "Condition" || element_name == "Control")
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

  PortsRemapping port_remap;

  if (element_name == "SubTree" || element_name == "SubTreePlus")
  {
    instance_name = element->Attribute("ID");
  }
  else
  {
    // do this only if it NOT a Subtree
    for (const XMLAttribute* att = element->FirstAttribute(); att; att = att->Next())
    {
      const std::string attribute_name = att->Name();
      if (!IsReservedPortname(attribute_name))
      {
        port_remap[attribute_name] = att->Value();
      }
    }
  }
  NodeConfiguration config;
  config.blackboard = blackboard;

  //---------------------------------------------
  TreeNode::Ptr child_node;

  if (factory.builders().count(ID) != 0)
  {
    const auto& manifest = factory.manifests().at(ID);

    //Check that name in remapping can be found in the manifest
    for (const auto& remap_it : port_remap)
    {
      if (manifest.ports.count(remap_it.first) == 0)
      {
        throw RuntimeError("Possible typo? In the XML, you tried to remap port "
                           "\"",
                           remap_it.first, "\" in node [", ID, " / ", instance_name,
                           "], but the manifest of this node does not contain a "
                           "port with "
                           "this name.");
      }
    }

    // Initialize the ports in the BB to set the type
    for (const auto& port_it : manifest.ports)
    {
      const std::string& port_name = port_it.first;
      const auto& port_info = port_it.second;

      auto remap_it = port_remap.find(port_name);
      if (remap_it == port_remap.end())
      {
        continue;
      }
      StringView param_value = remap_it->second;

      if (auto param_res = TreeNode::getRemappedKey(port_name, param_value))
      {
        // port_key will contain the key to find the entry in the blackboard
        const auto port_key = static_cast<std::string>(param_res.value());

        // if the entry already exists, check that the type is the same
        if (auto prev_info = blackboard->portInfo(port_key))
        {
          bool const port_type_mismatch = (prev_info->isStronglyTyped() &&
                                           port_info.isStronglyTyped() &&
                                           *prev_info->type() != *port_info.type());

          // special case related to convertFromString
          bool const string_input = ( prev_info->type() &&
              *prev_info->type() == typeid(std::string));

          if (port_type_mismatch && !string_input)
          {
            blackboard->debugMessage();

            throw RuntimeError("The creation of the tree failed because the port [",
                               port_key, "] was initially created with type [",
                               demangle(prev_info->type()), "] and, later type [",
                               demangle(port_info.type()), "] was used somewhere else.");
          }
        }
        else
        {
          // not found, insert for the first time.
          blackboard->createEntry(port_key, port_info);
        }
      }
    }

    // use manifest to initialize NodeConfiguration
    for (const auto& remap_it : port_remap)
    {
      const auto& port_name = remap_it.first;
      auto port_it = manifest.ports.find(port_name);
      if (port_it != manifest.ports.end())
      {
        auto direction = port_it->second.direction();
        if (direction != PortDirection::OUTPUT)
        {
          config.input_ports.insert(remap_it);
        }
        if (direction != PortDirection::INPUT)
        {
          config.output_ports.insert(remap_it);
        }
      }
    }

    // use default value if available for empty ports. Only inputs
    for (const auto& port_it : manifest.ports)
    {
      const std::string& port_name = port_it.first;
      const PortInfo& port_info = port_it.second;

      auto direction = port_info.direction();
      if (direction != PortDirection::OUTPUT &&
          config.input_ports.count(port_name) == 0 &&
          port_info.defaultValue().empty() == false)
      {
        config.input_ports.insert({port_name, port_info.defaultValue()});
      }
    }

    child_node = factory.instantiateTreeNode(instance_name, ID, config);
  }
  else if (tree_roots.count(ID) != 0)
  {
    child_node = std::make_unique<SubtreeNode>(instance_name);
  }
  else
  {
    throw RuntimeError(ID, " is not a registered node, nor a Subtree");
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

  recursiveStep = [&](const TreeNode::Ptr& parent, const XMLElement* element) {
    // create the node
    auto node = createNodeFromXML(element, blackboard, parent);
    output_tree.nodes.push_back(node);

    if (node->type() == NodeType::SUBTREE)
    {
      if (dynamic_cast<const SubtreeNode*>(node.get()))
      {
        bool is_isolated = true;

        for (const XMLAttribute* attr = element->FirstAttribute(); attr != nullptr;
             attr = attr->Next())
        {
          if (StrEqual(attr->Name(), "__shared_blackboard"))
          {
            is_isolated = !convertFromString<bool>(attr->Value());
            break;
          }
        }

        if (!is_isolated)
        {
          recursivelyCreateTree(node->name(), output_tree, blackboard, node);
        }
        else
        {
          // Creating an isolated
          auto new_bb = Blackboard::create(blackboard);

          for (const XMLAttribute* attr = element->FirstAttribute(); attr != nullptr;
               attr = attr->Next())
          {
            if (!IsReservedPortname(attr->Name()))
            {
              new_bb->addSubtreeRemapping(attr->Name(), attr->Value());
            }
          }
          output_tree.blackboard_stack.emplace_back(new_bb);
          recursivelyCreateTree(node->name(), output_tree, new_bb, node);
        }
      }
      else if (dynamic_cast<const SubtreePlusNode*>(node.get()))
      {
        auto new_bb = Blackboard::create(blackboard);
        output_tree.blackboard_stack.emplace_back(new_bb);
        std::set<StringView> mapped_keys;

        for (const XMLAttribute* attr = element->FirstAttribute(); attr != nullptr;
             attr = attr->Next())
        {
          const char* attr_name = attr->Name();
          const char* attr_value = attr->Value();

          if (IsReservedPortname(attr->Name()))
          {
            continue;
          }
          if (StrEqual(attr_name, "__autoremap"))
          {
            bool do_autoremap = convertFromString<bool>(attr_value);
            new_bb->enableAutoRemapping(do_autoremap);
            continue;
          }

          if (TreeNode::isBlackboardPointer(attr_value))
          {
            // do remapping
            StringView port_name = TreeNode::stripBlackboardPointer(attr_value);
            new_bb->addSubtreeRemapping(attr_name, port_name);
            mapped_keys.insert(attr_name);
          }
          else
          {
            // constant string: just set that constant value into the BB
            new_bb->set(attr_name, static_cast<std::string>(attr_value));
            mapped_keys.insert(attr_name);
          }
        }

        recursivelyCreateTree(node->name(), output_tree, new_bb, node);
      }
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

  auto it = tree_roots.find(tree_ID);
  if (it == tree_roots.end())
  {
    throw std::runtime_error(std::string("Can't find a tree with name: ") + tree_ID);
  }

  auto root_element = it->second->FirstChildElement();

  // start recursion
  recursiveStep(root_parent, root_element);
}

void XMLParser::Pimpl::getPortsRecursively(const XMLElement* element,
                                           std::vector<std::string>& output_ports)
{
  for (const XMLAttribute* attr = element->FirstAttribute(); attr != nullptr;
       attr = attr->Next())
  {
    const char* attr_name = attr->Name();
    const char* attr_value = attr->Value();
    if (!IsReservedPortname(attr_name) &&
        TreeNode::isBlackboardPointer(attr_value))
    {
      auto port_name = TreeNode::stripBlackboardPointer(attr_value);
      output_ports.push_back(static_cast<std::string>(port_name));
    }
  }

  for (auto child_element = element->FirstChildElement(); child_element;
       child_element = child_element->NextSiblingElement())
  {
    getPortsRecursively(child_element, output_ports);
  }
}

std::string writeTreeNodesModelXML(const BehaviorTreeFactory& factory,
                                   bool include_builtin)
{
  XMLDocument doc;

  XMLElement* rootXML = doc.NewElement("root");
  doc.InsertFirstChild(rootXML);

  XMLElement* model_root = doc.NewElement("TreeNodesModel");
  rootXML->InsertEndChild(model_root);

  std::set<std::string> ordered_names;

  for (auto& model_it : factory.manifests())
  {
    const auto& registration_ID = model_it.first;
    if (!include_builtin && factory.builtinNodes().count(registration_ID) != 0)
    {
      continue;
    }
    ordered_names.insert(registration_ID);
  }

  for (auto& registration_ID : ordered_names)
  {
    const auto& model = factory.manifests().at(registration_ID);

    XMLElement* element = doc.NewElement(toStr(model.type).c_str());
    element->SetAttribute("ID", model.registration_ID.c_str());

    std::vector<std::string> ordered_ports;
    PortDirection directions[3] = {PortDirection::INPUT, PortDirection::OUTPUT,
                                   PortDirection::INOUT};
    for (int d = 0; d < 3; d++)
    {
      std::set<std::string> port_names;
      for (auto& port : model.ports)
      {
        const auto& port_name = port.first;
        const auto& port_info = port.second;
        if (port_info.direction() == directions[d])
        {
          port_names.insert(port_name);
        }
      }
      for (auto& port : port_names)
      {
        ordered_ports.push_back(port);
      }
    }

    for (const auto& port_name : ordered_ports)
    {
      const auto& port_info = model.ports.at(port_name);

      XMLElement* port_element = nullptr;
      switch (port_info.direction())
      {
        case PortDirection::INPUT:
          port_element = doc.NewElement("input_port");
          break;
        case PortDirection::OUTPUT:
          port_element = doc.NewElement("output_port");
          break;
        case PortDirection::INOUT:
          port_element = doc.NewElement("inout_port");
          break;
      }

      port_element->SetAttribute("name", port_name.c_str());
      if (port_info.type())
      {
        port_element->SetAttribute("type", BT::demangle(port_info.type()).c_str());
      }
      if (!port_info.defaultValue().empty())
      {
        port_element->SetAttribute("default", port_info.defaultValue().c_str());
      }

      if (!port_info.description().empty())
      {
        port_element->SetText(port_info.description().c_str());
      }
      element->InsertEndChild(port_element);
    }

    if (!model.description.empty())
    {
      element->SetAttribute("description", model.registration_ID.c_str());
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

}   // namespace BT
