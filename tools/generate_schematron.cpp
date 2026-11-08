#include <iostream>

#include <behaviortree_cpp/bt_factory.h>
#include <behaviortree_cpp/xml_parsing.h>

// Generates an ISO Schematron schema for BehaviorTree.CPP XML files and
// prints it to stdout.
//
// The Schematron complements the generic XSD (bt4_generate_xsd) with
// cross-reference rules that XSD cannot express:
//
//   - treeNodesModel: every custom (non-built-in) node used in a BehaviorTree
//     body must have a matching <TreeNodesModel> entry (required by Groot2).
//   - subtreeResolution: every <SubTree ID="X"/> must resolve to a
//     <BehaviorTree ID="X"> in the same file.
//   - rootStructure: main_tree_to_execute must name an existing BehaviorTree.
//
// Usage (two-step validation):
//
//   # Generate schemas
//   ./bt4_generate_xsd > btcpp4.xsd
//   ./bt4_generate_schematron > btcpp4.sch
//
//   # Step 1 — structural validation (XSD)
//   xmllint --noout --schema btcpp4.xsd myfile.xml
//
//   # Step 2 — cross-reference validation (Schematron)
//   #   Option A: xsltproc + ISO Schematron XSLT1 skeleton
//   #     (download iso_schematron_skeleton_for_xslt1.xsl from
//   #      https://github.com/Schematron/schematron)
//   xsltproc iso_schematron_skeleton_for_xslt1.xsl btcpp4.sch > btcpp4_validator.xsl
//   xsltproc btcpp4_validator.xsl myfile.xml
//
//   #   Option B: Python + lxml (no XSLT skeleton needed)
//   pip install lxml
//   python3 - << 'EOF'
//   from lxml import etree, isoschematron
//   ns = "http://purl.oclc.org/dsdl/svrl"
//   sch = isoschematron.Schematron(etree.parse("btcpp4.sch"), store_report=True)
//   if not sch.validate(etree.parse("myfile.xml")):
//       for fa in sch.validation_report.findall(f".//{{{ns}}}failed-assert"):
//           print(f"{fa.get('location')}: {fa.findtext(f'{{{ns}}}text', '').strip()}")
//   EOF
//
//   #   Option C: Java + SchXslt
//   java -jar schxslt-cli.jar -s btcpp4.sch -i myfile.xml
int main()
{
  BT::BehaviorTreeFactory factory;
  std::cout << BT::writeTreeSchematron(factory);
  return 0;
}
