#include "behaviortree_cpp/basic_types.h"
#include "behaviortree_cpp/bt_factory.h"
#include "behaviortree_cpp/control_node.h"
#include "behaviortree_cpp/xml_parsing.h"

#include <array>
#include <string>

#include <gtest/gtest.h>

using namespace BT;

namespace
{

class NameValidationTestControlNode : public ControlNode
{
public:
  NameValidationTestControlNode(const std::string& name, const NodeConfig& config)
    : ControlNode(name, config)
  {}

  static PortsList providedPorts()
  {
    return {};
  }

  NodeStatus tick() override
  {
    return NodeStatus::SUCCESS;
  }
};

struct ExplicitNodeCase
{
  const char* tag;
  const char* child_xml;
};

constexpr std::array<ExplicitNodeCase, 4> kExplicitNodeCases = {
  ExplicitNodeCase{ "Action", "" }, ExplicitNodeCase{ "Condition", "" },
  ExplicitNodeCase{ "Decorator", "<AlwaysSuccess/>" },
  ExplicitNodeCase{ "Control", "<AlwaysSuccess/>" }
};

constexpr std::array<const char*, 3> kValidASCIIModelNames = { "Valid_Name", "Valid-Name",
                                                               "1LeadingDigit" };

constexpr std::array<const char*, 1> kValidUnicodeModelNames = { "检查门状态" };

constexpr std::array<const char*, 2> kInvalidExplicitModelNames = { "Node.With.Dot",
                                                                    "Root" };

std::string MakeExplicitNodeXML(const ExplicitNodeCase& node_case,
                                const std::string& model_name,
                                const char* instance_name = nullptr)
{
  std::string xml = "\n"
                    "    <root BTCPP_format=\"4\">\n"
                    "      <BehaviorTree ID=\"MainTree\">\n"
                    "        <";
  xml += node_case.tag;
  xml += " ID=\"";
  xml += model_name;
  xml += "\"";
  if(instance_name != nullptr)
  {
    xml += " name=\"";
    xml += instance_name;
    xml += "\"";
  }
  if(node_case.child_xml[0] == '\0')
  {
    xml += "/>\n";
  }
  else
  {
    xml += ">";
    xml += node_case.child_xml;
    xml += "</";
    xml += node_case.tag;
    xml += ">\n";
  }
  xml += "      </BehaviorTree>\n"
         "    </root>";
  return xml;
}

void RegisterExplicitNodeModel(BehaviorTreeFactory& factory, std::string_view tag,
                               const std::string& model_name)
{
  if(tag == "Action")
  {
    auto tick = [](TreeNode&) { return NodeStatus::SUCCESS; };
    factory.registerSimpleAction(model_name, tick);
  }
  else if(tag == "Condition")
  {
    auto tick = [](TreeNode&) { return NodeStatus::SUCCESS; };
    factory.registerSimpleCondition(model_name, tick);
  }
  else if(tag == "Decorator")
  {
    auto tick = [](NodeStatus child_status, TreeNode&) { return child_status; };
    factory.registerSimpleDecorator(model_name, tick);
  }
  else if(tag == "Control")
  {
    factory.registerNodeType<NameValidationTestControlNode>(model_name);
  }
}

}  // namespace

// ============== Tests for findForbiddenChar() ==============

TEST(NameValidation, ForbiddenCharDetection_ValidNames)
{
  // Valid ASCII names
  EXPECT_EQ(findForbiddenChar("ValidName"), '\0');
  EXPECT_EQ(findForbiddenChar("my_action"), '\0');
  EXPECT_EQ(findForbiddenChar("My-Action"), '\0');
  EXPECT_EQ(findForbiddenChar("action123"), '\0');
  EXPECT_EQ(findForbiddenChar("CamelCaseNode"), '\0');
  EXPECT_EQ(findForbiddenChar("snake_case_node"), '\0');
  EXPECT_EQ(findForbiddenChar("kebab-case-node"), '\0');
}

TEST(NameValidation, ForbiddenCharDetection_Unicode)
{
  // Unicode names should be allowed (UTF-8 multibyte sequences)
  EXPECT_EQ(findForbiddenChar("检查门状态"), '\0');    // Chinese
  EXPECT_EQ(findForbiddenChar("ドアを開ける"), '\0');  // Japanese
  EXPECT_EQ(findForbiddenChar("Tür_öffnen"), '\0');    // German with umlaut
  EXPECT_EQ(findForbiddenChar("проверка"), '\0');      // Russian
  EXPECT_EQ(findForbiddenChar("действие"), '\0');      // Russian
}

TEST(NameValidation, ForbiddenCharDetection_ForbiddenChars)
{
  // Space and whitespace
  EXPECT_EQ(findForbiddenChar("My Action"), ' ');
  EXPECT_EQ(findForbiddenChar("with\ttab"), '\t');
  EXPECT_EQ(findForbiddenChar("with\nnewline"), '\n');
  EXPECT_EQ(findForbiddenChar("with\rcarriage"), '\r');

  // XML special characters
  EXPECT_EQ(findForbiddenChar("My<Node>"), '<');
  EXPECT_EQ(findForbiddenChar("Node>End"), '>');
  EXPECT_EQ(findForbiddenChar("A&B"), '&');
  EXPECT_EQ(findForbiddenChar("say\"hello\""), '"');
  EXPECT_EQ(findForbiddenChar("it's"), '\'');

  // Filesystem problematic characters
  EXPECT_EQ(findForbiddenChar("path/to/node"), '/');
  EXPECT_EQ(findForbiddenChar("path\\to\\node"), '\\');
  EXPECT_EQ(findForbiddenChar("C:drive"), ':');
  EXPECT_EQ(findForbiddenChar("wild*card"), '*');
  EXPECT_EQ(findForbiddenChar("what?"), '?');
  EXPECT_EQ(findForbiddenChar("pipe|char"), '|');

  // Period (can cause issues)
  EXPECT_EQ(findForbiddenChar("request.name"), '.');
  EXPECT_EQ(findForbiddenChar("file.ext"), '.');
}

TEST(NameValidation, ForbiddenCharDetection_ControlChars)
{
  // Control characters should be forbidden
  std::string with_null = "test";
  with_null += '\0';
  with_null += "name";
  EXPECT_EQ(findForbiddenChar(with_null), '\0');  // null char detected

  // Bell character (ASCII 7) - use string concatenation to avoid hex digit issues
  std::string with_bell = "test";
  with_bell += '\x07';
  with_bell += "bell";
  EXPECT_EQ(findForbiddenChar(with_bell), '\x07');

  // DEL character (ASCII 127)
  std::string with_del = "test";
  with_del += '\x7F';
  with_del += "del";
  EXPECT_EQ(findForbiddenChar(with_del), '\x7F');
}

// ============== Tests for IsAllowedPortName() ==============

TEST(NameValidation, IsAllowedPortName_Valid)
{
  EXPECT_TRUE(IsAllowedPortName("input"));
  EXPECT_TRUE(IsAllowedPortName("output_value"));
  EXPECT_TRUE(IsAllowedPortName("myPort123"));
  EXPECT_TRUE(IsAllowedPortName("Port_With_Underscore"));
}

TEST(NameValidation, IsAllowedPortName_Invalid)
{
  // Empty
  EXPECT_FALSE(IsAllowedPortName(""));

  // Starts with digit
  EXPECT_FALSE(IsAllowedPortName("1port"));
  EXPECT_FALSE(IsAllowedPortName("123"));

  // Starts with underscore (reserved)
  EXPECT_FALSE(IsAllowedPortName("_private"));

  // Reserved names
  EXPECT_FALSE(IsAllowedPortName("name"));
  EXPECT_FALSE(IsAllowedPortName("ID"));
  EXPECT_FALSE(IsAllowedPortName("_failureIf"));
  EXPECT_FALSE(IsAllowedPortName("_successIf"));
  EXPECT_FALSE(IsAllowedPortName("_skipIf"));
  EXPECT_FALSE(IsAllowedPortName("_while"));
  EXPECT_FALSE(IsAllowedPortName("_onSuccess"));
  EXPECT_FALSE(IsAllowedPortName("_onFailure"));
  EXPECT_FALSE(IsAllowedPortName("_onHalted"));
  EXPECT_FALSE(IsAllowedPortName("_post"));
  EXPECT_FALSE(IsAllowedPortName("_autoremap"));

  // Forbidden characters
  EXPECT_FALSE(IsAllowedPortName("port name"));  // space
  EXPECT_FALSE(IsAllowedPortName("port.name"));  // period
  EXPECT_FALSE(IsAllowedPortName("port<T>"));    // angle brackets
}

TEST(NameValidation, CreatePort_NamesForbiddenChar)
{
  try
  {
    (void)InputPort<std::string>("goal.pose");
    FAIL() << "Expected RuntimeError to be thrown";
  }
  catch(const RuntimeError& e)
  {
    const std::string msg = e.what();
    EXPECT_NE(msg.find("goal.pose"), std::string::npos) << msg;
    EXPECT_NE(msg.find("forbidden character '.'"), std::string::npos) << msg;
  }

  try
  {
    (void)InputPort<std::string>("goal\tpose");
    FAIL() << "Expected RuntimeError to be thrown";
  }
  catch(const RuntimeError& e)
  {
    const std::string msg = e.what();
    EXPECT_NE(msg.find("control character (ASCII 9)"), std::string::npos) << msg;
  }
}

// ============== Tests for XML parsing validation ==============

class NameValidationXMLTest : public testing::Test
{
protected:
  BehaviorTreeFactory factory;
};

TEST_F(NameValidationXMLTest, ValidBehaviorTreeID)
{
  const char* xml = R"(
    <root BTCPP_format="4">
      <BehaviorTree ID="MainTree">
        <AlwaysSuccess/>
      </BehaviorTree>
    </root>)";
  EXPECT_NO_THROW((void)factory.createTreeFromText(xml));
}

TEST_F(NameValidationXMLTest, ValidBehaviorTreeID_WithUnderscore)
{
  const char* xml = R"(
    <root BTCPP_format="4">
      <BehaviorTree ID="My_Main_Tree">
        <AlwaysSuccess/>
      </BehaviorTree>
    </root>)";
  EXPECT_NO_THROW((void)factory.createTreeFromText(xml));
}

TEST_F(NameValidationXMLTest, InvalidBehaviorTreeID_Root)
{
  const char* xml = R"(
    <root BTCPP_format="4">
      <BehaviorTree ID="Root">
        <AlwaysSuccess/>
      </BehaviorTree>
    </root>)";
  EXPECT_THROW((void)factory.createTreeFromText(xml), RuntimeError);
}

TEST_F(NameValidationXMLTest, InvalidBehaviorTreeID_root_lowercase)
{
  const char* xml = R"(
    <root BTCPP_format="4">
      <BehaviorTree ID="root">
        <AlwaysSuccess/>
      </BehaviorTree>
    </root>)";
  EXPECT_THROW((void)factory.createTreeFromText(xml), RuntimeError);
}

TEST_F(NameValidationXMLTest, InvalidBehaviorTreeID_WithSpace)
{
  const char* xml = R"(
    <root BTCPP_format="4">
      <BehaviorTree ID="Main Tree">
        <AlwaysSuccess/>
      </BehaviorTree>
    </root>)";
  EXPECT_THROW((void)factory.createTreeFromText(xml), RuntimeError);
}

TEST_F(NameValidationXMLTest, InvalidBehaviorTreeID_WithPeriod)
{
  const char* xml = R"(
    <root BTCPP_format="4">
      <BehaviorTree ID="Main.Tree">
        <AlwaysSuccess/>
      </BehaviorTree>
    </root>)";
  EXPECT_THROW((void)factory.createTreeFromText(xml), RuntimeError);
}

TEST_F(NameValidationXMLTest, ValidInstanceName)
{
  const char* xml = R"(
    <root BTCPP_format="4">
      <BehaviorTree ID="MainTree">
        <AlwaysSuccess name="my_success_node"/>
      </BehaviorTree>
    </root>)";
  EXPECT_NO_THROW((void)factory.createTreeFromText(xml));
}

TEST_F(NameValidationXMLTest, ValidInstanceName_WithSpace)
{
  // Instance names are XML attribute VALUES, so spaces are allowed
  const char* xml = R"(
    <root BTCPP_format="4">
      <BehaviorTree ID="MainTree">
        <AlwaysSuccess name="my success node"/>
      </BehaviorTree>
    </root>)";
  EXPECT_NO_THROW((void)factory.createTreeFromText(xml));
}

TEST_F(NameValidationXMLTest, ValidInstanceName_WithPeriod)
{
  // Instance names are XML attribute VALUES, so periods are allowed
  const char* xml = R"(
    <root BTCPP_format="4">
      <BehaviorTree ID="MainTree">
        <AlwaysSuccess name="node.name"/>
      </BehaviorTree>
    </root>)";
  EXPECT_NO_THROW((void)factory.createTreeFromText(xml));
}

TEST_F(NameValidationXMLTest, ExplicitNodeIDs_InvalidModelNamesThrowHelpfulErrors)
{
  for(const auto& node_case : kExplicitNodeCases)
  {
    for(const char* invalid_model_name : kInvalidExplicitModelNames)
    {
      BehaviorTreeFactory explicit_factory;
      RegisterExplicitNodeModel(explicit_factory, node_case.tag, invalid_model_name);

      SCOPED_TRACE(std::string(node_case.tag) + " / " + invalid_model_name);
      const auto xml = MakeExplicitNodeXML(node_case, invalid_model_name);

      try
      {
        (void)explicit_factory.createTreeFromText(xml);
        FAIL() << "Expected RuntimeError for explicit model name: " << invalid_model_name;
      }
      catch(const RuntimeError& e)
      {
        const std::string msg = e.what();
        EXPECT_NE(msg.find(invalid_model_name), std::string::npos) << msg;
        if(std::string(invalid_model_name) == "Node.With.Dot")
        {
          EXPECT_NE(msg.find("forbidden character '.'"), std::string::npos) << msg;
        }
        else
        {
          EXPECT_NE(msg.find("reserved name"), std::string::npos) << msg;
        }
      }
    }
  }
}

TEST_F(NameValidationXMLTest, ExplicitNodeIDs_ValidASCIIAndUnicodeAreAccepted)
{
  for(const auto& node_case : kExplicitNodeCases)
  {
    for(const char* valid_model_name : kValidASCIIModelNames)
    {
      BehaviorTreeFactory explicit_factory;
      RegisterExplicitNodeModel(explicit_factory, node_case.tag, valid_model_name);

      SCOPED_TRACE(std::string(node_case.tag) + " / " + valid_model_name);
      const auto xml = MakeExplicitNodeXML(node_case, valid_model_name);
      EXPECT_NO_THROW((void)explicit_factory.createTreeFromText(xml));
    }

    for(const char* valid_model_name : kValidUnicodeModelNames)
    {
      BehaviorTreeFactory explicit_factory;
      RegisterExplicitNodeModel(explicit_factory, node_case.tag, valid_model_name);

      SCOPED_TRACE(std::string(node_case.tag) + " / " + valid_model_name);
      const auto xml = MakeExplicitNodeXML(node_case, valid_model_name);
      EXPECT_NO_THROW((void)explicit_factory.createTreeFromText(xml));
    }
  }
}

TEST_F(NameValidationXMLTest, ExplicitNodeIDs_PreserveRelaxedInstanceNames)
{
  for(const auto& node_case : kExplicitNodeCases)
  {
    BehaviorTreeFactory explicit_factory;
    RegisterExplicitNodeModel(explicit_factory, node_case.tag, "Valid_Name");

    SCOPED_TRACE(node_case.tag);
    const auto xml =
        MakeExplicitNodeXML(node_case, "Valid_Name", "node.name with spaces");
    EXPECT_NO_THROW((void)explicit_factory.createTreeFromText(xml));
  }
}

TEST_F(NameValidationXMLTest, ValidSubTreeID)
{
  const char* xml = R"(
    <root BTCPP_format="4" main_tree_to_execute="MainTree">
      <BehaviorTree ID="MainTree">
        <SubTree ID="SubTree1"/>
      </BehaviorTree>
      <BehaviorTree ID="SubTree1">
        <AlwaysSuccess/>
      </BehaviorTree>
    </root>)";
  EXPECT_NO_THROW((void)factory.createTreeFromText(xml));
}

TEST_F(NameValidationXMLTest, InvalidSubTreeID_WithSpace)
{
  const char* xml = R"(
    <root BTCPP_format="4" main_tree_to_execute="MainTree">
      <BehaviorTree ID="MainTree">
        <SubTree ID="Sub Tree"/>
      </BehaviorTree>
      <BehaviorTree ID="Sub Tree">
        <AlwaysSuccess/>
      </BehaviorTree>
    </root>)";
  EXPECT_THROW((void)factory.createTreeFromText(xml), RuntimeError);
}

// ============== Tests for Unicode support ==============

TEST_F(NameValidationXMLTest, UnicodeTreeID_Chinese)
{
  const char* xml = R"(
    <root BTCPP_format="4">
      <BehaviorTree ID="检查门">
        <AlwaysSuccess/>
      </BehaviorTree>
    </root>)";
  EXPECT_NO_THROW((void)factory.createTreeFromText(xml));
}

TEST_F(NameValidationXMLTest, UnicodeInstanceName_Japanese)
{
  const char* xml = R"(
    <root BTCPP_format="4">
      <BehaviorTree ID="MainTree">
        <AlwaysSuccess name="成功ノード"/>
      </BehaviorTree>
    </root>)";
  EXPECT_NO_THROW((void)factory.createTreeFromText(xml));
}

TEST_F(NameValidationXMLTest, UnicodeTreeID_German)
{
  const char* xml = R"(
    <root BTCPP_format="4">
      <BehaviorTree ID="Türöffner">
        <AlwaysSuccess/>
      </BehaviorTree>
    </root>)";
  EXPECT_NO_THROW((void)factory.createTreeFromText(xml));
}

// ============== Tests for SubTree port validation ==============

TEST_F(NameValidationXMLTest, ValidSubTreePortName)
{
  const char* xml = R"(
    <root BTCPP_format="4" main_tree_to_execute="MainTree">
      <BehaviorTree ID="MainTree">
        <SubTree ID="MySubTree" input_value="{value}"/>
      </BehaviorTree>
      <BehaviorTree ID="MySubTree">
        <AlwaysSuccess/>
      </BehaviorTree>
      <TreeNodesModel>
        <SubTree ID="MySubTree">
          <input_port name="input_value"/>
        </SubTree>
      </TreeNodesModel>
    </root>)";
  EXPECT_NO_THROW((void)factory.createTreeFromText(xml));
}

TEST_F(NameValidationXMLTest, InvalidSubTreePortName_WithSpace)
{
  const char* xml = R"(
    <root BTCPP_format="4" main_tree_to_execute="MainTree">
      <BehaviorTree ID="MainTree">
        <AlwaysSuccess/>
      </BehaviorTree>
      <TreeNodesModel>
        <SubTree ID="MySubTree">
          <input_port name="input value"/>
        </SubTree>
      </TreeNodesModel>
    </root>)";
  EXPECT_THROW((void)factory.createTreeFromText(xml), RuntimeError);
}

TEST_F(NameValidationXMLTest, InvalidSubTreePortName_Reserved)
{
  const char* xml = R"(
    <root BTCPP_format="4" main_tree_to_execute="MainTree">
      <BehaviorTree ID="MainTree">
        <AlwaysSuccess/>
      </BehaviorTree>
      <TreeNodesModel>
        <SubTree ID="MySubTree">
          <input_port name="ID"/>
        </SubTree>
      </TreeNodesModel>
    </root>)";
  EXPECT_THROW((void)factory.createTreeFromText(xml), RuntimeError);
}

TEST_F(NameValidationXMLTest, InvalidSubTreePortName_StartsWithDigit)
{
  const char* xml = R"(
    <root BTCPP_format="4" main_tree_to_execute="MainTree">
      <BehaviorTree ID="MainTree">
        <AlwaysSuccess/>
      </BehaviorTree>
      <TreeNodesModel>
        <SubTree ID="MySubTree">
          <input_port name="1port"/>
        </SubTree>
      </TreeNodesModel>
    </root>)";
  EXPECT_THROW((void)factory.createTreeFromText(xml), RuntimeError);
}
