/*
Copyright (c) 2019 Akamai Technologies, Inc

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <iostream>
#include <fstream>
#include <cstddef>
#include <stdint.h>
#include <vector>
#include <string>

#include "SimpleRadixTree.h"
#include "RadixTree.h"
#include "CursorOps.h"
#include "SimplePath.h"

// This example combines some typedefs for terenary radix trees and uses them
// to build and traverse a very simple tree.

using namespace Akamai::Mapper::RadixTree;

// The RadixTree library is very template-heavy. Typedefs/using declarations
// are often critical for readability.
using TerenaryStringTree16 = SimpleRadixTree<std::string,3,16,8>;
using TerenaryPath16 = SimplePath<3,16>;
using TerenaryString16CursorRO = TerenaryStringTree16::CursorROType;
using TerenaryString16Cursor = TerenaryStringTree16::CursorType;

// Quick and dirty function to transform a path of arbitrary
// radix into string form for printing.
template <typename PathType>
std::string pathToString(PathType&& p,const std::string& sep = "-") {
  std::string pathstr;
  std::size_t pathlen = p.size();
  if (pathlen == 0) { return sep + "/0"; }
  for (std::size_t i = 0;i < pathlen; ++i) {
    if (i > 0) { pathstr += sep; }
    pathstr += std::to_string(p.at(i));
  }
  pathstr += ("/" + std::to_string(pathlen));
  return pathstr;
}

// Simple implementations of pre/post order walking of trees with cursors.
// These are also provided as library routines, but putting them
// here to serve as examples. Note that these traversals are implemented
// recursively even though the cursors themselves are fundamentally iterative.
// Depending on what your tree traversal requirements are it often makes sense
// to take the recursive approach if it makes the code easier to follow.
template <typename CallbackType>
void simplePreOrder(TerenaryString16CursorRO& c,CallbackType& cb)
{
  if (c.atValue()) { cb(c); }
  for (std::size_t i = 0; i < TerenaryString16CursorRO::Radix; ++i) {
    if (c.canGoChildNode(i)) {
      c.goChild(i);
      simplePreOrder(c,cb);
      c.goParent();
    }
  }
}

template <typename CallbackType>
void simplePostOrder(TerenaryString16CursorRO& c,CallbackType& cb)
{
  for (std::size_t i = 0; i < TerenaryString16CursorRO::Radix; ++i) {
    if (c.canGoChildNode(i)) {
      c.goChild(i);
      simplePreOrder(c,cb);
      c.goParent();
    }
  }
  if (c.atValue()) { cb(c); }
}


int main(int /*argc*/,const char** /*argv*/) {
  // We're storing strings in our tree, quick and dirty way of converting small numbers
  // to their English equivalent.
  std::vector<std::string> numberWords{"zero","one","two","three","four","five","six","seven","eight","nine","ten"};

  std::cout << "=== BUILDING TERENARY TREE ===" << std::endl;
  std::vector<TerenaryPath16> terenaryPaths{{0,1,2},
                                            {0,0,0,0,0,0},
                                            {1},
                                            {2},
                                            {2,2,2,1,1,1,0,0,0,1,2,1,2,1,2,0},
                                            {2,2,2,2,2,2,2},
                                            {0,0,0,1,1,2,1,1},
                                            {1,2,1,2,1,1,0,1,1,1,2,0,0,1,1,1},
                                            {}};

  // Build the tree using simple cursor operations.
  TerenaryStringTree16 terenaryTree;
  for (std::size_t i = 0;i < terenaryPaths.size(); ++i) {
    std::cout <<
      std::to_string(i) << " " << pathToString(terenaryPaths[i]) << ": " << numberWords.at(i) << std::endl;
    cursorAddValueAt(terenaryTree.cursor(),terenaryPaths[i],numberWords.at(i));
  }

  std::cout << std::endl;
  std::cout << "=== CHECKING TERENARY TREE ===" << std::endl;
  // Go through all of the entries we added and use simple cursor operations
  // to verify that we've got values where we expect them to be. Normally
  // we'd check for the existence of a value before dereferencing it - if
  // something goes wrong in this case we'll segfault.
  for (std::size_t i = 0;i < terenaryPaths.size(); ++i) {
    std::cout <<
      std::to_string(i) << " " << pathToString(terenaryPaths[i]) << ": " <<
      *(cursorGotoValue(terenaryTree.cursorRO(),terenaryPaths[i]).getPtrRO()) << std::endl;
  }

  std::cout << std::endl;
  std::cout << "=== TRAVERSING TERENARY TREE ===" << std::endl;
  // Use the simple traversal functions we implemented earlier
  // to walk the tree in pre/post order. In-order doesn't make sense
  // for a terenary tree - we need to have an even number of children
  // if we're going to evaluate a particular node in the middle of them.
  std::cout << "----PRE ORDER----" << std::endl;
  auto terenaryCursor = terenaryTree.cursorRO();
  auto printCursorValue =
    [](const TerenaryString16CursorRO& c) {
      std::cout << pathToString(c.getPath()) << ": " << *(c.nodeValueRO().getPtrRO()) << std::endl;
    };
    
  simplePreOrder(terenaryCursor,printCursorValue);
  std::cout << "----POST ORDER----" << std::endl;
  simplePostOrder(terenaryCursor,printCursorValue);

  return 0;
}