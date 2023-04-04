# Debugging Skins

The purpose of this document is to describe a way to implement
**mock testing** in the context of Behavior Trees.

By **mock testing** we mean changing the way a single Node return its
value. This can be done at two levels:

1. substituting the entire Node at **deployment time** with a mock. We call this
"substitution".
2. injecting pre/post conditions at **deployment or run-time**.

We will use JSOn as configuration language.

## Filters and rules

Usually, we want to mock-ify only a subset of Nodes. 
To define the nodes, we use a syntax like this:

```json
{
  "BehaviorTrees": [
    {
      "tree_filter": "SubTreeA",
      "Nodes": [
        {
          "node_filter": "MyAction::the_action",
          "substitution": "alwaysSuccess(0)"
        }
     ]
    }
  ]
}
```

- **BehaviorTrees** is an array; each element contains a field "filter and an array **Nodes**. 
- The array **Nodes** contains our rules; they have the field "filter" too.

In the example above, we select all the nodes in the SubTrees of type **SubTreeA**
which have type **MyAction** and instance name **the_action**.

Our rule is called "substitution" (more details in the next section).

The filter always follow the syntax:

       type_name::instance_name

The following two filters are equivalent: they mean that the filter should be applied to all
the nodes/trees with a given type:

       type_name::*
       type_name

Similarly, if you only want to specify the instance name,
a valid syntax is:

       *::instance_name

## Substitute an entire Node

This approach makes particular sense when you don't want 
to create a certain node in the first place.

This can only be done at deployment time, i.e. when 
the method `BehaviorTreeFactory::createTree(treeID)` is invoked.

To do that, we can use instead the method:

`BehaviorTreeFactory::createTree(treeID, json_rules_string)`;

Where the second argument is the string containing our JSON
configuration.

Let's say that we have a Node that talks to a piece of hardware
that we don't have, called **CameraSnapshot**.

We want to substitute that node with a mock that always returns SUCCESS:

```json
{
  "BehaviorTrees": [
    {
      "tree_filter": "*",
      "Nodes": [
        {
          "node_filter": "CameraSnapshot::*",
          "substitution": "alwaysSuccess(0)"
        }
      ]
    }
  ]
}
```

The possible substitution rules are:

- **skip**
- **alwaysSuccess**(running_time)
- **alwaysFailure**(running_time)
- **alwaysRunning**()
- **failureInjection**(running_time, success_count, config)

Then `running_time` is > 0, the action becomes **asynchronous**.
Its value is expressed in milliseconds.

The rule **failureInjection** is used to succeed X time and return
failure once. `config` is an enum that could be:

- ONCE: fail once and then always retun success
- REPEAT: fails periodically.
- KEEP_FAILING: once you fail, continue failing.

## Script injection

This method can be applied both ad deployment time or
run-time (to be implemented).

It adds a pre/post condition script that is executed respectively
before or after all the others.

Example:

```json
"Nodes": [
  {
     "node_filter": "CameraSnapshot::*",
     "precondition": "successIf(true)"
  }
]
```

This is similar to the previous one: the `tick()` of 
**CameraSnapshot** is never executed and success is returned instead
but, in this case, the instance of the Node does exist; 
this rule might also be removed at run-time.

**Pre conditions**:

- **skipIf**(script)
- **successIf**(script)
- **failureIf**(script)
- **while**(script)

As you can see, this is exactly the same syntax used in regular preconditions.

Post conditions instead are different:

- **forceFailure**(script)
- **forceSuccess**(script)

> TODO: do we need more? Describe how the script can use the actual
return value.