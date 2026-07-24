#include <iostream>

#include <behaviortree_cpp/bt_factory.h>
#include <behaviortree_cpp/xml_parsing.h>

// Generates a generic XSD schema for BehaviorTree.CPP XML files and
// prints it to stdout.
//
// The schema covers all built-in node types with their specific attributes
// and child-count constraints. Unknown custom node elements are accepted via
// xs:any processContents="lax", and top-level xs:element declarations allow
// lax processing to still validate built-in node elements by name.
//
// Usage:
//   ./bt4_generate_xsd > btcpp4.xsd
//   xmllint --noout --schema btcpp4.xsd myfile.xml
int main()
{
  BT::BehaviorTreeFactory factory;
  std::cout << BT::writeTreeXSD(factory, /*generic=*/true);
  return 0;
}
