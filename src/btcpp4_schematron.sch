<?xml version="1.0" encoding="UTF-8"?>
<sch:schema xmlns:sch="http://purl.oclc.org/dsdl/schematron" queryBinding="xslt">
  <sch:title>BehaviorTree.CPP XML validation rules</sch:title>

  <!--
    Rule A checks that every compact-notation custom node appearing in a BehaviorTree
    body has a corresponding entry in TreeNodesModel. Groot2 and other graphical
    editors require this because compact notation alone gives no port information.
    Explicit notation (<Action ID="X"/>) is already sufficient for Groot2 and does
    not require a TreeNodesModel entry (see behaviortree.dev/docs/learn-the-basics/xml_format).
    Built-in node types are excluded - they are always known to the library.

    ##BUILTIN_PIPE## is a placeholder expanded at runtime by writeTreeSchematron()
    with a pipe-delimited list of all built-in node names derived from the factory,
    e.g. |AlwaysFailure|AlwaysSuccess|Fallback|Sequence|...|.

    The XPath 1.0 idiom  contains('|A|B|C|', concat('|', local-name(), '|'))
    tests set membership without requiring XPath 2.0 sequence literals, keeping
    this schema compatible with xsltproc and lxml.isoschematron.
  -->
  <sch:pattern id="treeNodesModel">
    <sch:title>TreeNodesModel completeness</sch:title>

    <!-- Rule A: compact-notation custom nodes (element name IS the registration ID) -->
    <sch:rule context="/root/BehaviorTree//*[
        not(contains('##BUILTIN_PIPE##', concat('|', local-name(), '|'))) and
        local-name() != 'Action' and local-name() != 'Condition' and
        local-name() != 'Control' and local-name() != 'Decorator' and
        local-name() != 'SubTree'
      ]">
      <sch:assert test="/root/TreeNodesModel/*[@ID = local-name(current())]">
        Custom node '<sch:value-of select="local-name()"/>' used in tree
        '<sch:value-of select="ancestor::BehaviorTree/@ID"/>'
        has no &lt;TreeNodesModel&gt; entry.
      </sch:assert>
    </sch:rule>
  </sch:pattern>

  <!--
    Rule B checks structural constraints for explicit-notation nodes.
    The BT.CPP XML format mandates:
      - Action and Condition nodes are leaves (no children).
      - Decorator nodes have exactly one child.
      - Control nodes have at least one child.
    These constraints apply to the generic wrapper elements used in explicit notation
    (<Action ID="..."/>, <Condition ID="..."/>, <Decorator ID="..."/>, <Control ID="..."/>).
    Built-in nodes written in compact notation are already constrained by the XSD.
  -->
  <sch:pattern id="explicitNodeStructure">
    <sch:title>Explicit-notation node structure</sch:title>

    <!-- Action and Condition are leaves -->
    <sch:rule context="/root/BehaviorTree//*[
        @ID and (local-name() = 'Action' or local-name() = 'Condition')
      ]">
      <sch:assert test="count(*) = 0">
        <sch:value-of select="local-name()"/> ID='<sch:value-of select="@ID"/>'
        must have no children (has <sch:value-of select="count(*)"/>).
      </sch:assert>
    </sch:rule>

    <!-- Decorator has exactly one child -->
    <sch:rule context="/root/BehaviorTree//*[local-name() = 'Decorator' and @ID]">
      <sch:assert test="count(*) = 1">
        Decorator ID='<sch:value-of select="@ID"/>'
        must have exactly one child (has <sch:value-of select="count(*)"/>).
      </sch:assert>
    </sch:rule>

    <!-- Control has at least one child -->
    <sch:rule context="/root/BehaviorTree//*[local-name() = 'Control' and @ID]">
      <sch:assert test="count(*) >= 1">
        Control ID='<sch:value-of select="@ID"/>' must have at least one child.
      </sch:assert>
    </sch:rule>
  </sch:pattern>

  <!--
    Rule C checks that every <SubTree ID="X"/> references a <BehaviorTree ID="X">
    defined in the same file. The check is relaxed when an <include> element is
    present because the referenced tree may reside in an external file.
  -->
  <sch:pattern id="subtreeResolution">
    <sch:title>SubTree ID cross-references</sch:title>
    <sch:rule context="/root/BehaviorTree//SubTree[@ID]">
      <sch:assert
          test="/root/BehaviorTree[@ID = current()/@ID] or /root/include">
        SubTree ID='<sch:value-of select="@ID"/>' is not defined in this file.
        (If the definition lives in an included file, add an &lt;include&gt;.)
      </sch:assert>
    </sch:rule>
  </sch:pattern>

  <sch:pattern id="rootStructure">
    <sch:title>Root element consistency</sch:title>
    <sch:rule context="/root[@main_tree_to_execute]">
      <sch:assert
          test="BehaviorTree[@ID = current()/@main_tree_to_execute]">
        main_tree_to_execute='<sch:value-of select="@main_tree_to_execute"/>'
        does not match any BehaviorTree ID in this file.
      </sch:assert>
    </sch:rule>
  </sch:pattern>

</sch:schema>
