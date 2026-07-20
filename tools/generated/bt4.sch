<?xml version="1.0" encoding="UTF-8"?>
<sch:schema xmlns:sch="http://purl.oclc.org/dsdl/schematron" queryBinding="xslt">
  <sch:title>BehaviorTree.CPP XML validation rules</sch:title>

  <!--
    Rules A and B check that every custom node appearing in a BehaviorTree
    body has a corresponding entry in TreeNodesModel. Groot2 and other
    graphical editors require this metadata to display nodes and their ports.
    Built-in node types are excluded — they are always known to the library
    and do not need an explicit declaration.

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
        not(contains('|AlwaysFailure|AlwaysSuccess|AsyncFallback|AsyncSequence|Delay|Fallback|ForceFailure|ForceSuccess|IfThenElse|Inverter|KeepRunningUntilFailure|LoopBool|LoopDouble|LoopInt|LoopString|Parallel|ParallelAll|Precondition|ReactiveFallback|ReactiveSequence|Repeat|RetryUntilSuccessful|RunOnce|Script|ScriptCondition|Sequence|SequenceWithMemory|SetBlackboard|SkipUnlessUpdated|Sleep|SubTree|Switch2|Switch3|Switch4|Switch5|Switch6|Timeout|TryCatch|UnsetBlackboard|WaitValueUpdate|WasEntryUpdated|WhileDoElse|', concat('|', local-name(), '|'))) and
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

    <!-- Rule B: explicit-notation nodes (<Action ID="..."/>, <Condition ID="..."/>, etc.) -->
    <sch:rule context="/root/BehaviorTree//*[
        @ID and (
          local-name() = 'Action' or local-name() = 'Condition' or
          local-name() = 'Control' or local-name() = 'Decorator'
        )
      ]">
      <sch:assert test="/root/TreeNodesModel/*[@ID = current()/@ID]">
        Node with ID='<sch:value-of select="@ID"/>' used in tree
        '<sch:value-of select="ancestor::BehaviorTree/@ID"/>'
        has no &lt;TreeNodesModel&gt; entry.
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
