Copyright (C) 2019 Akamai Technologies, Inc

https://www.akamai.com

# Simple Radix Tree Examples
The examples in this directory illustrate basic usage of cursors to
construct and walk radix trees of different bases.

## Binary Trees

*BinaryTrees.cc*

This example uses multiple binary tree implementations provided by the Radix Tree library
to illustrate basic tree operations.

## Terenary Tree

*TerenaryTree.cc*

This example uses the standard node/child pointer implementations to build and
traverse small trees of order 3.

## Alphabet Tree

*AlphabetTree.cc*

This example uses the standard node/child pointer implementations to build
and traverse an order 26 "word dictionary" tree. It also illustrates prefix
matching by enumerating all words in the tree that start with a given prefix.