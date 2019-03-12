![Akamai Logo](Akamai-Logo-Circle-PMS.png)

Copyright (C) 2019 Akamai Technologies, Inc

https://www.akamai.com

# Akamai Radix Tree

## Introduction
The Akamai Radix Tree library provides an abstraction as well as multiple
implementations of radix trees in C++. Radix trees are typically used as key/value
stores with longest prefix matching, emphasizing random access key storage and
retrieval - *e.g.* IP routing and forwarding. This library supports that use case,
but also supports navigating a radix tree as if it were a simple, fully-populated
*order-N* tree through an abstraction we call a *cursor*. A cursor tracks every possible
position in a tree with specific degree and depth. For example, a binary tree of depth 2:

                 ROOT
           0               1
       0       1       0       1

has 7 possible positions, shown in pre-order here: 

    ROOT
	0
	00
	01
	1
	10
	11

Even if a particular tree contained just a single value at position 0, a cursor on that
tree would still be able to navigate through all 7 positions. When at position 0 the
cursor would indicate the presence of a value, and at positions 00 and 01 it would
show the value a 0 as the longest prefix match for those positions. For the other 4
positions in the tree the cursor would indicate that no value and no longest prefix
match is present. If a value were also present at ROOT, then the cursor would indicate
the presence of that value at ROOT as well as that value as the longest prefix match for
1,10, and 11.

The ability to navigate a radix tree as a simple tree is very useful for collecting, combining,
and analyzing information about the internet. At Akamai we collect information from a variety of 
different sources spread throughout IPv6 space, *e.g.* BGP feeds and DNS hit logs, that we analyze
and use when managing our network. Radix trees covering IPv6 space are one of our core
data structures.

## Radix Tree Refresher
If you're already familiar with radix/patricia trees then feel free to skip this
section. A radix tree encodes a key value as part of its structure. As an example imagine
that we're building a simple spelling checker. A simple and direct way of doing this would
be to take all of our valid words and put them into either a tree or hash data structure.
We'll assume lower-case letters only and a very small dictionary to keep the example simple.
The words in the dictionary are also contrived to highlight important differences between a
regular tree and a radix tree. Here are the words in our dictionary:

    cart
    cartoon
    cat
    catalog
    dog

Storing these words in a sorted binary tree might look like this:

                 cat
       cartoon         catalog
    cart                     dog

Looking up a word is logarithmic with the number of words stored as long as we keep the tree
balanced. Storing these words in a naive radix tree would use a tree of degree 26 - one child
for each lowercase letter of the alphabet:

           ROOT
      c              d
      a              o
    r   t            g
    t   a
    o   l
    o   o
    n   g

Looking up a value in the radix tree requires us to branch on each letter, so a successful
lookup will require linear time based on the length of the word. The radix tree requires at
least one bit at each node to indicate whether or not it is a valid stopping place for a word.
Without an indicator there would be no way to determine in the example above that "cat" is a
valid dictionary item and "cata" isn't. Having the values contained in the tree embedded in its
structure allows us to quickly perform longest prefix matching. For example, if a user typed "ca"
we could quickly present all valid dictionary completions by following the letters in the tree.
For "ca" these would be "car", "cat", "cartoon", and "catalog". Providing this prefix matching
functionality would be more complicated in either a sorted binary tree or hash table.

An important implementation detail of radix trees is the use of "edges" to avoid requiring full
nodes at non-terminal and non-branching points in the tree. An edge substitutes a list of the
branches to be taken for actual nodes and child pointers. This saves a significant amount of
space when trees are sparse. Our example dictionary requires 16 full nodes in the simple
representation. The same dictionary using edges to compress branches would look like this:

              ROOT
        ca            do             
      r    t            g
    too    alo
       n      g

Using edges we only require 10 nodes instead of 16. When representing large and sparse trees
the space saved by using edges quickly becomes significant.

## Cursor Abstraction
The core abstraction used in the Radix Tree library is the *cursor*. A cursor tracks an
arbitrary position in a radix tree, as described in the Introduction. Key operations
on a cursor include the expected tree operations, *i.e.* descending to a child position
and ascending to a parent position and checking to see if a value exists at a position
in the tree. However, there are also some operations that are required for efficiently
working with the "virtual" tree positions supported by cursors. Three key features of
supporting virtual positions are the ability to see if the cursor is presently at an actual
node, whether or not any actual nodes exist down a specific child path, and the handling
of values that are set at nodes.

## Radix Tree Implementations
The Akamai Radix Tree library provides multiple underlying implementations of
radix trees. Each of these makes its own performance/capability tradeoffs. The
provided implementations are not the only ones possible, just those that we
implemented first. Our tree implementations support the use of custom memory
allocators. The default allocator uses the system new and delete calls, but
it is straightforward to switch to using a separate custom memory allocator if
required.

### Arbitrary Radix Node/Edge/Pointer
The basic all-purpose radix tree implementation uses a traditional node/edge/child pointer
architecture. There are actually two variations of this tree. The simplest uses an array of
pointers to children, one for each possible. The slightly more complicated uses a hash map
(std::unordered_map) to store child pointers which might prove useful for sparsely populated
trees. Edges are represented by arrays of integers, one for each step in the edge. The size
of each integer in the edge array is the smallest standard C++ integer type large enough
to hold the maximum possible child number.

### Binary Node/Edge/Pointer
The binary node/edge/pointer radix tree is similar to the arbitrary radix. The critical
difference is that both metadata and edges are packed into either 32 or 64 bit integers
instead of using separate C++ class/struct members for each, making for a more compact
node format. This "packed edge" implementation could be extended to handle higher degree
trees, but this work hasn't been done yet.

### Binary Word Array Trees
The binary "word array" trees implement binary trees using either 3 or 4 CPU words
of either 32 or 64 bits. In the current implementation these trees use a std::vector
for underlying memory allocation - all of the "nodes" are sequences of integers,
contiguous in memory. This tree can only store integers directly, but these integers
could easily be used as an index into a separate "dictionary" containing arbitrary values.
These trees are generally intended for applications that add all of the nodes to a particular
tree at once and then perform read only operations on that tree. In particular, if the tree
is built in pre-order the nodes ought to lay out in memory well for lookups and traversal
in addition to avoiding heap overhead.

### Binary Byte-Packed Write Once/Read Many (WORM) Trees
The byte-packed WORM tree implementation takes the binary word array tree scheme
and makes it even more compact, though with the tradeoff that the tree must be
built all at once, and is read-only afterwards. Each tree node has a single byte
used for metadata, followed by offset pointers to child nodes and/or value bytes
only if they are required. The tree encoding requires a byte-addressable buffer,
and is suitable for direct memory mapping/serialization.

## Cursor Implementations
Along the same lines as the tree implementations, there are multiple available cursor
implementations. Each of these implementations has somewhat different performance,
capability, and consistency properties. The consistency properties are only relevant
when performing read and write operations on the same tree with different cursors.

### Standard RO/RW Cursors
The standard read-only and read-write cursors are the baseline cursor implementations.
They can both navigate through all potential tree positions, and are the safest with respect
to consistency across simultaneous read and write operations. Cursors are not invalidated
by write operations as long as those write operations happen at tree positions below or
outside of their position. However, these cursors are also usually the slowest in operation
due to the minimal amount of tree structural information they cache (providing them with
their stronger consistency properties).

### "Walking" RO Cursor
The "walking" read-only cursor allows full access to all tree positions, as with the standard
read-only cursor. It can provide better performance than the standard RO cursor, but at the cost
of less predictability with respect to simultaneous read and write operations on the same tree.
In some cases a write to a position below the current position of a tree cursor may invalidate
that cursor in non-obvious ways, making them less suitable if your application requires
cursors to interleave read and write operations on the same tree.

### "Lookup" RO/WO Cursors
The "lookup" cursors are optimized for simple lookup/insertion operations. These cursors
are only capable of moving to child nodes - any call to an operation that would move the
cursor back "up" the tree towards the root results in an exception. Furthermore, the
"write-only" version of the cursor actually creates the path it traverses down the tree
as it goes. Unlike the other cursors which don't alter the tree by simple navigation,
this cursor assumes that the end goal is to make an addition to the tree at a specific place.
These cursors are much simpler and generally significantly faster than the other cursor
implementations. The IPv4/6 tree example applications mentioned below are a good
example of the type of use case intended for these cursors.

## Compound Cursor Wrapper
The compound cursor wrapper takes multiple cursors and treats them as a single entity,
as if navigating a union of the trees underlying the cursors. The various encapsulated
types (values, etc.) of the cursor are collected as tuples of the values for the
underlying cursors. In addition to a simple "union" cursor the compound cursors also
have a "following" version. A "follower" cursor is moved along with the "followed" cursors,
but the follower cursor structure isn't used when deciding where values in the 
compound tree are. This type of cursor is useful when creating a new tree out of the
values specified at one or more other trees. A variant on the following cursor is
a cursor that "follows over" other cursors. In the "follow over" cursor the follower
values are considered as part of the compound tree value only when the cursor encounters
the follower values in the course of traversing the followed cursors. This type of cursor
is useful when accumulating information from other trees into a single tree. The new
information in the followed cursors is assimilated into the existing information in the
following cursor. The cursor only needs to check for information in the area covered
by the followed cursors, but also needs to know where that information intersects
with the contents of the follower cursor.

## Cursor Tree Traversal
The radix tree library contains both recursive and iterative tree traversal routines.
While the cursors are an inherently iterative data structure, tree navigation is often
more clearly expressed in a recursive way so both options are provided. By and large
these traversal routines perform as one might expect: pre, post, and in-order traversals
are available, the recursive versions call into user-defined callbacks at points where
node values must be evaluated. A slight variation from normal is that the traversal
routines permit navigating child nodes in reverse order ("right to left") instead of the
standard "left to right" direction. For example, the regular pre-order traversal of a
binary tree would first check the node, then the left child (child 0) and then the right
child (child 1). The reversed version would first check the node, then the right child
(child 1) followed by the left child (child 0). The default is to use a "left to right"
ordering.

## Configuring, Building, and Installing
The Akamai Radix Tree uses CMake for configuration, building, and installing. The core
library is header-only, so installing simply involves copying the relevant headers to
an accessible location. Optionally unit tests, sample applications, and Doxygen-generated
documentation may also be built, though these are disabled by default. The following
CMake definitions control whether these optional subcomponents are built:

* AKAMAI_MAPPER_RADIXTREE_TEST *Build/run unit tests (OFF)*
* AKAMAI_MAPPER_RADIXTREE_DOCS *Build Doxygen documentation (OFF)*
* AKAMAI_MAPPER_RADIXTREE_EXAMPLES *Build RadixTree examples (OFF)*

The performance of the Radix Tree library depends heavily on compiler optimizations. If
you wish to build/run the unit tests it is a good idea to use a release build confiugration,
otherwise the tests may take a long time to complete.

## Simple Tree Examples
The simple tree examples show how to perform some basic tree instantiation, creation,
and traversals. The examples include trees of different degrees (2,3, and 26) as well
as different underlying implementations.

## IPv4/6 Tree Examples
The IPv4/6 examples show some examples of using binary radix trees for IPv4 and IPV6
addresses. These examples are more complicated than the simple tree examples, so it
is probably best to look at these after looking at the simple examples. The examples
here include implementations of longest prefix lookup for IPv4 and IPv6 addresses.

## Credits
The primary developer and maintainer of this library is Michael Neufeld
(mneufeld@akamai.com). During her internship at Akamai (Summer 2018),
Mallory Grider made significant contributions to this library, performing
extensive testing and debugging, providing design feedback, adding documentation,
and enhancing the test infrastructure.

The internal IPv4/IPv6 radix tree at Akamai which inspired this library
was built by Amitabha Roy and Larry Campbell.