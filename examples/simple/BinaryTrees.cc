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

#include "SimpleRadixTree.h"
#include "BinaryRadixTree.h"
#include "RadixTree.h"
#include "CursorTraversal.h"
#include "CursorOps.h"
#include "BinaryPath.h"

// Build and traverse some simple binary trees using cursor operations.

using namespace Akamai::Mapper::RadixTree;

// The RadixTree library is very template-heavy. Typedefs/using declarations
// are often critical for readability.

// This is an instance of our regular node/child pointer radix tree implementation.
// We're using it as one of our binary tree examples, but it may also be used
// for higher order trees.
using BinaryStringTree16 = BinaryRadixTree32<std::string,16>;

// This is an instance of our "four word" binary tree. Each node consists of four
// 32 or 64 bit (in this case 32 bit) unsigned integers. All of the nodes in a tree
// are allocated sequentially in a std::vector. The tree always stores a single
// integer.
using BinaryWord32Tree16 = BinaryWordTree32<16>;

// This is an instance of our "three word" binary tree. It is like the four word
// tree except the value stored takes up space inside the "metadata" word instead
// of having its own separate word. The cost of this is that the metadata word
// has fewer bits available for holding edge bits. Another wrinkle is that
// the stored value itself is not byte-addressable, so the separable node value has to
// keep a copy of the value in the "node" itself.
using BinaryCompactWord16Tree16 = CompactBinaryWordTree<uint16_t,uint32_t,16>;

// This is another instance of our "three word" binary tree as above, but only
// storing a bool. The bool trees are specialized to store just 1 bit.
using BinaryCompactBoolTree16 = CompactBinaryBoolTree32<16>;

using BinaryCompactVoidTree16 = CompactBinaryVoidTree32<16>;

// Using a 16-deep binary path for all of our examples here.
using BinaryPath16 = BinaryPath<16>;

int main(int /*argc*/,const char** /*argv*/) {
  // Map from small integers to their English equivalent, also put together a small set
  // of simple paths.
  std::vector<std::string> numberWords{"zero","one","two","three","four","five","six","seven","eight","nine","ten"};
  std::vector<BinaryPath16> binaryPaths{{},
                                        {1,0,0,1,0,0,1},
                                        {0,1},
                                        {1,1,1,1,1,1},
                                        {0,0,0,1,0},
                                        {1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0}};

  BinaryStringTree16 binaryString{};
  BinaryWord32Tree16 binaryWord32{};
  BinaryCompactBoolTree16 binaryCompactBool{};
  BinaryCompactVoidTree16 binaryCompactVoid{};
  BinaryCompactWord16Tree16 binaryCompactWord16{};

  // We're going to build all of the binary trees using basic cursor operations.
  // Along the way we'll print out the values we've added for comparison to when
  // we check the values in the tree later.
  std::cout << "=== BUILDING BINARY TREES ===" << std::endl;
  for (std::size_t i=0;i<binaryPaths.size();++i) {
    const BinaryPath16& curPath = binaryPaths.at(i);
    std::string valueStr = numberWords.at(i);
    uint32_t value32 = static_cast<uint32_t>(i);
    uint16_t value16 = static_cast<uint16_t>(i);
    bool valueBool = true;

    std::cout << std::to_string(i) << " " << curPath.toBinaryString() << ": " <<
              valueStr << "," <<
              std::to_string(value32) << "," <<
              (valueBool ? "true": "false") << "," <<
              (valueBool ? "true": "false") << "," <<
              std::to_string(value16) <<
              std::endl;

    // Note that we're creating a new cursor at the root every time
    // we navigate down to a value. We could also reuse the cursors
    // if we wanted to.
    cursorAddValueAt(binaryString.cursor(),curPath,valueStr);
    cursorAddValueAt(binaryWord32.cursor(),curPath,value32);
    cursorAddValueAt(binaryCompactBool.cursor(),curPath,valueBool);
    cursorAddValueAt(binaryCompactVoid.cursor(),curPath,valueBool);
    cursorAddValueAt(binaryCompactWord16.cursor(),curPath,value16);
  }

  // To check the tree values we're going to navigate down to each
  // cursor position and print the value that's there.
  std::cout << std::endl;
  std::cout << "=== CHECKING BINARY TREES ===" << std::endl;
  for (std::size_t i=0;i<binaryPaths.size();++i) {
    const BinaryPath16& curPath = binaryPaths.at(i);
    // The nodeValue objects store the "context" of a node value
    // at a particular position. In a regular tree these values would
    // be accessed by simply using a reference to the entire node that
    // contains the value. Since the cursors track arbitrary, possibly
    // non-existent, locations in tree implementations that may not even have
    // regular nodes we limit our abstraction to just the node value.
    auto nodeValueStr = cursorGotoValue(binaryString.cursorRO(),curPath);
    auto nodeValue32 = cursorGotoValue(binaryWord32.cursorRO(),curPath);
    auto nodeValueBool = cursorGotoValue(binaryCompactBool.cursorRO(),curPath);
    auto nodeValueVoid = cursorGotoValue(binaryCompactVoid.cursorRO(),curPath);
    auto nodeValue16 = cursorGotoValue(binaryCompactWord16.cursorRO(),curPath);

    // We dereference the pointers to the node values without checking to see
    // if they're not null first. This is because we know that there ought to
    // be values at each of the path locations we're visiting. A nullptr would
    // result in a segfault, indicating that there was something wrong with
    // the trees we constructed.
    std::cout << std::to_string(i) << " " << curPath.toBinaryString() << ": " <<
              *(nodeValueStr.getPtrRO()) << "," <<
              std::to_string(*(nodeValue32.getPtrRO())) << "," <<
              ((*(nodeValueBool.getPtrRO())) ? "true" : "false") << "," <<
              ((*(nodeValueVoid.getPtrRO())) ? "true" : "false") << "," <<
              std::to_string(*(nodeValue16.getPtrRO())) <<
              std::endl;
  }
  
  // Now traverse the trees using our traversal routines provided in
  // CursorTraversal.h. The code for those traversals is template-heavy
  // and generic - the TerenaryTree.cc example has much simpler (but less
  // flexible) versions of recursive tree traversal using cursors.
  std::cout << std::endl;
  std::cout << "=== TRAVERSING BINARY TREES ===" << std::endl;
  std::cout << "* SINGLE TREE *" << std::endl;
  std::cout << "----PRE ORDER----" << std::endl;
  // Walks the tree, calling a user-defined callback in pre-order at each place
  // there's a value stored in the tree. If you're doing something simple putting
  // the callback into an inline lambda is a succinct way of doing it.
  // That said, if you don't get the callback function signature correct the error
  // messages are pretty terrible.
  preOrderWalk([](const BinaryStringTree16::CursorROType& c) {
                 std::cout << c.getPath().toBinaryString() << ": " << *(c.nodeValueRO().getPtrRO()) << std::endl;
               },
               binaryString.cursorRO());
            
  std::cout << std::endl;
  std::cout << "* MULTI TREE *" << std::endl;
  // We can also walk multiple trees simultaneously, we'll get a callback
  // wherever any of them has a value with all of the tree cursors.
  // Still using a lambda as in the single tree case, but since this is a little more
  // verbose and we're going to use it multiple times we keep it in a separate
  // variable.
  auto printStringAnd16 = [](const BinaryStringTree16::CursorROType& cstr,
                             const BinaryCompactWord16Tree16::CursorROType& c16) {
                               std::cout << cstr.getPath().toBinaryString() << ": " <<
                                            *(cstr.nodeValueRO().getPtrRO()) << "," <<
                                            std::to_string(*(c16.nodeValueRO().getPtrRO())) <<
                                            std::endl;
                             };
  std::cout << "----PRE ORDER----" << std::endl;
  preOrderWalk(printStringAnd16,binaryString.cursorRO(),binaryCompactWord16.cursorRO());
  // An in-order traversal only makes sense on trees with an even degree - otherwise
  // there's no even divide between the "left" and "right" halves of the children.
  std::cout << "----IN ORDER----" << std::endl;
  inOrderWalk(printStringAnd16,binaryString.cursorRO(),binaryCompactWord16.cursorRO());
  std::cout << "----POST ORDER----" << std::endl;
  postOrderWalk(printStringAnd16,binaryString.cursorRO(),binaryCompactWord16.cursorRO());

  return 0;
}