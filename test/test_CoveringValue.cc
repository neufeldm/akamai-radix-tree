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

#include <string>
#include <array>
#include <algorithm>
#include <iterator>
#include <inttypes.h>
#include <random>
#include <cmath>

#include "gtest/gtest.h"

#include "TreeTestUtils.h"
#include "PathSort.h"
#include "TreeTests.h"

#include "BinaryRadixTree.h"
#include "BinaryWORMTreeUInt.h"
#include "BinaryWORMTreeBuilder.h"
#include "BinaryWORMTreeUIntBuilder.h"
#include "CursorOps.h"

using namespace Akamai::Mapper::RadixTree;

using BinaryPath16 = BinaryPath<16>;
using BinaryUInt32Tree = BinaryRadixTree32<uint32_t,16>;
using BinaryWORM32Tree = BinaryWORMTreeUInt<std::vector<uint8_t>,BinaryPath<16>,true,4,4>;
using BinaryWORM32TreeBuilder = BinaryWORMTreeBuilderVector<BinaryPath<16>,BinaryWORMNodeUIntWO<true,4,4>>;

/*
 *  Simple test for a few different covering value scenarios
 */

// Test on an empty tree
TEST(BinaryTree, TestCoverEmpty) {
  BinaryUInt32Tree emptyTree{};
  BinaryWORM32TreeBuilder buildWORM;
  ASSERT_TRUE(buildWORM.start());
  buildWORM.addNode(BinaryPath16{},false,nullptr,{false,false});
  ASSERT_TRUE(buildWORM.finish());
  auto emptyWORMGenericTree = makeWORMTreeUIntGeneric<BinaryPath16>(BinaryWORMTreeUIntParams{true,4,4},buildWORM.buffer());
  BinaryWORM32Tree emptyWORMTree(buildWORM.buffer());

  auto cRegular = emptyTree.cursorRO();
  auto cWalk = emptyTree.walkCursorRO();
  auto cLookup = emptyTree.lookupCursorRO();
  auto cWORM = emptyWORMTree.walkCursorRO();
  auto cWORMLookup = emptyWORMTree.lookupCursorRO();
  auto cWORMGeneric = emptyWORMGenericTree.walkCursorRO();

  ASSERT_EQ(cRegular.coveringNodeValueRO().getPtrRO(),nullptr);
  ASSERT_EQ(cWalk.coveringNodeValueRO().getPtrRO(),nullptr);
  ASSERT_EQ(cLookup.coveringNodeValueRO().getPtrRO(),nullptr);
  ASSERT_EQ(cWORM.coveringNodeValueRO().getPtrRO(),nullptr);
  ASSERT_EQ(cWORMLookup.coveringNodeValueRO().getPtrRO(),nullptr);
  ASSERT_EQ(cWORMGeneric.coveringNodeValueRO().getPtrRO(),nullptr);

  // Go below the value, verify that there's still nothing there
  BinaryPath16 emptyToCheck{0,1,0,0,1};
  cursorGoto(cRegular,emptyToCheck);
  cursorGoto(cWalk,emptyToCheck);
  cursorGoto(cLookup,emptyToCheck);
  cursorGoto(cWORM,emptyToCheck);
  cursorGoto(cWORMLookup,emptyToCheck);
  cursorGoto(cWORMGeneric,emptyToCheck);

  ASSERT_EQ(cRegular.coveringNodeValueRO().getPtrRO(),nullptr);
  ASSERT_EQ(cWalk.coveringNodeValueRO().getPtrRO(),nullptr);
  ASSERT_EQ(cLookup.coveringNodeValueRO().getPtrRO(),nullptr);
  ASSERT_EQ(cWORM.coveringNodeValueRO().getPtrRO(),nullptr);
  ASSERT_EQ(cWORMLookup.coveringNodeValueRO().getPtrRO(),nullptr);
  ASSERT_EQ(cWORMGeneric.coveringNodeValueRO().getPtrRO(),nullptr);
}

// Test with a value at the root.
TEST(BinaryTree, TestCoverRoot) {
  BinaryUInt32Tree rootTree{};
  uint32_t rootVal = 1;
  rootTree.cursor().addNode().set(rootVal);
  BinaryWORM32TreeBuilder buildWORM;
  ASSERT_TRUE(buildWORM.start());
  buildWORM.addNode(BinaryPath16{},true,&rootVal,{false,false});
  ASSERT_TRUE(buildWORM.finish());
  BinaryWORM32Tree rootWORMTree(buildWORM.buffer());
  auto rootWORMGenericTree = makeWORMTreeUIntGeneric<BinaryPath16>(BinaryWORMTreeUIntParams{true,4,4},buildWORM.buffer());

  auto cRegular = rootTree.cursorRO();
  auto cWalk = rootTree.walkCursorRO();
  auto cLookup = rootTree.lookupCursorRO();
  auto cWORM = rootWORMTree.walkCursorRO();
  auto cWORMLookup = rootWORMTree.lookupCursorRO();
  auto cWORMGeneric = rootWORMGenericTree.walkCursorRO();

  // Verify that we've got our covering value at the root.
  ASSERT_EQ(*(cRegular.coveringNodeValueRO().getPtrRO()),rootVal);
  ASSERT_EQ(*(cWalk.coveringNodeValueRO().getPtrRO()),rootVal);
  ASSERT_EQ(*(cLookup.coveringNodeValueRO().getPtrRO()),rootVal);
  ASSERT_EQ(*(cWORM.coveringNodeValueRO().getPtrRO()),rootVal);
  ASSERT_EQ(*(cWORMLookup.coveringNodeValueRO().getPtrRO()),rootVal);
  ASSERT_EQ(*(cWORMGeneric.coveringNodeValueRO().getPtrRO()),rootVal);

  // Now go down a path, check the covering value again.
  BinaryPath16 rootToCheck{0,1,0,0,1};
  cursorGoto(cRegular,rootToCheck);
  cursorGoto(cWalk,rootToCheck);
  cursorGoto(cLookup,rootToCheck);
  cursorGoto(cWORM,rootToCheck);
  cursorGoto(cWORMLookup,rootToCheck);
  cursorGoto(cWORMGeneric,rootToCheck);

  ASSERT_EQ(*(cRegular.coveringNodeValueRO().getPtrRO()),rootVal);
  ASSERT_EQ(cRegular.coveringNodeValueDepth(),0);
  ASSERT_EQ(*(cWalk.coveringNodeValueRO().getPtrRO()),rootVal);
  ASSERT_EQ(cWalk.coveringNodeValueDepth(),0);
  ASSERT_EQ(*(cLookup.coveringNodeValueRO().getPtrRO()),rootVal);
  ASSERT_EQ(cLookup.coveringNodeValueDepth(),0);
  ASSERT_EQ(*(cWORM.coveringNodeValueRO().getPtrRO()),rootVal);
  ASSERT_EQ(cWORM.coveringNodeValueDepth(),0);
  ASSERT_EQ(*(cWORMLookup.coveringNodeValueRO().getPtrRO()),rootVal);
  ASSERT_EQ(cWORMLookup.coveringNodeValueDepth(),0);
  ASSERT_EQ(*(cWORMGeneric.coveringNodeValueRO().getPtrRO()),rootVal);
  ASSERT_EQ(cWORMGeneric.coveringNodeValueDepth(),0);
}

// Test with a covering value below the root, plus an intervening node.
TEST(BinaryTree, TestCoverBelowRoot) {
  BinaryUInt32Tree belowRootTree{};
  uint32_t belowRootVal = 2;
  uint32_t belowRootVal2 = 5;
  BinaryPath16 belowRootPath{0,1,1,1};
  BinaryPath16 betweenPath{0,1,1,1,0,1};
  BinaryPath16 belowRootPath2{0,1,1,1,0,1,0,1,0,1,1,0,1};
  cursorAddValueAt(belowRootTree.cursor(),belowRootPath,belowRootVal);
  auto betweenCursor = belowRootTree.cursor();
  cursorGoto(betweenCursor,betweenPath);
  betweenCursor.addNode();
  cursorAddValueAt(belowRootTree.cursor(),belowRootPath2,belowRootVal2);
  // The WORM tree will have an intermediate empty scaffolding node added so we
  // don't need to add it explicitly.
  BinaryWORM32TreeBuilder buildWORM;
  ASSERT_TRUE(buildWORM.start());
  buildWORM.addNode(belowRootPath,true,&belowRootVal,{true,false});
  buildWORM.addNode(belowRootPath2,true,&belowRootVal2,{false,false});
  ASSERT_TRUE(buildWORM.finish());
  BinaryWORM32Tree belowRootWORMTree(buildWORM.buffer());
  auto belowRootWORMGenericTree = makeWORMTreeUIntGeneric<BinaryPath16>(BinaryWORMTreeUIntParams{true,4,4},buildWORM.buffer());

  auto cRegular = belowRootTree.cursorRO();
  auto cWalk = belowRootTree.walkCursorRO();
  auto cLookup = belowRootTree.lookupCursorRO();
  auto cWORM = belowRootWORMTree.walkCursorRO();
  auto cWORMLookup = belowRootWORMTree.lookupCursorRO();
  auto cWORMGeneric = belowRootWORMGenericTree.lookupCursorRO();

  // Make sure we've got no covering value at the root.
  ASSERT_EQ(cRegular.coveringNodeValueRO().getPtrRO(),nullptr);
  ASSERT_EQ(cWalk.coveringNodeValueRO().getPtrRO(),nullptr);
  ASSERT_EQ(cLookup.coveringNodeValueRO().getPtrRO(),nullptr);
  ASSERT_EQ(cWORM.coveringNodeValueRO().getPtrRO(),nullptr);
  ASSERT_EQ(cWORMLookup.coveringNodeValueRO().getPtrRO(),nullptr);
  ASSERT_EQ(cWORMGeneric.coveringNodeValueRO().getPtrRO(),nullptr);

  // Now walk down to the value and check our covering value again.
  cursorGoto(cRegular,belowRootPath);
  cursorGoto(cWalk,belowRootPath);
  cursorGoto(cLookup,belowRootPath);
  cursorGoto(cWORM,belowRootPath);
  cursorGoto(cWORMLookup,belowRootPath);
  cursorGoto(cWORMGeneric,belowRootPath);

  ASSERT_EQ(*(cRegular.coveringNodeValueRO().getPtrRO()),belowRootVal);
  ASSERT_EQ(*(cWalk.coveringNodeValueRO().getPtrRO()),belowRootVal);
  ASSERT_EQ(*(cLookup.coveringNodeValueRO().getPtrRO()),belowRootVal);
  ASSERT_EQ(*(cWORM.coveringNodeValueRO().getPtrRO()),belowRootVal);
  ASSERT_EQ(*(cWORMLookup.coveringNodeValueRO().getPtrRO()),belowRootVal);
  ASSERT_EQ(*(cWORMGeneric.coveringNodeValueRO().getPtrRO()),belowRootVal);

  ASSERT_EQ(*(cRegular.nodeValueRO().getPtrRO()),belowRootVal);
  ASSERT_EQ(*(cWalk.nodeValueRO().getPtrRO()),belowRootVal);
  ASSERT_EQ(*(cLookup.nodeValueRO().getPtrRO()),belowRootVal);
  ASSERT_EQ(*(cWORM.nodeValueRO().getPtrRO()),belowRootVal);
  ASSERT_EQ(*(cWORMLookup.nodeValueRO().getPtrRO()),belowRootVal);
  ASSERT_EQ(*(cWORMGeneric.nodeValueRO().getPtrRO()),belowRootVal);

  // Walk down one child below, check covering and values here as well.
  cRegular.goChild(0);
  cWalk.goChild(0);
  cLookup.goChild(0);
  cWORM.goChild(0);
  cWORMLookup.goChild(0);
  cWORMGeneric.goChild(0);

  ASSERT_EQ(*(cRegular.coveringNodeValueRO().getPtrRO()),belowRootVal);
  ASSERT_EQ(*(cWalk.coveringNodeValueRO().getPtrRO()),belowRootVal);
  ASSERT_EQ(*(cLookup.coveringNodeValueRO().getPtrRO()),belowRootVal);
  ASSERT_EQ(*(cWORM.coveringNodeValueRO().getPtrRO()),belowRootVal);
  ASSERT_EQ(*(cWORMLookup.coveringNodeValueRO().getPtrRO()),belowRootVal);
  ASSERT_EQ(*(cWORMGeneric.coveringNodeValueRO().getPtrRO()),belowRootVal);

  std::size_t valAtDepth = belowRootPath.size();
  ASSERT_EQ(cRegular.coveringNodeValueDepth(),valAtDepth);
  ASSERT_EQ(cWalk.coveringNodeValueDepth(),valAtDepth);
  ASSERT_EQ(cLookup.coveringNodeValueDepth(),valAtDepth);
  ASSERT_EQ(cWORM.coveringNodeValueDepth(),valAtDepth);
  ASSERT_EQ(cWORMLookup.coveringNodeValueDepth(),valAtDepth);
  ASSERT_EQ(cWORMGeneric.coveringNodeValueDepth(),valAtDepth);

  ASSERT_EQ(cRegular.nodeValueRO().getPtrRO(),nullptr);
  ASSERT_EQ(cWalk.nodeValueRO().getPtrRO(),nullptr);
  ASSERT_EQ(cLookup.nodeValueRO().getPtrRO(),nullptr);
  ASSERT_EQ(cWORM.nodeValueRO().getPtrRO(),nullptr);
  ASSERT_EQ(cWORMLookup.nodeValueRO().getPtrRO(),nullptr);
  ASSERT_EQ(cWORMGeneric.nodeValueRO().getPtrRO(),nullptr);

  // Walk down to the intermediate point, check covering/values here.
  cRegular = belowRootTree.cursorRO();
  cWalk = belowRootTree.walkCursorRO();
  cLookup = belowRootTree.lookupCursorRO();
  cWORM = belowRootWORMTree.walkCursorRO();
  cWORMLookup = belowRootWORMTree.lookupCursorRO();
  cWORMGeneric = belowRootWORMGenericTree.lookupCursorRO();
  cursorGoto(cRegular,betweenPath);
  cursorGoto(cWalk,betweenPath);
  cursorGoto(cLookup,betweenPath);
  cursorGoto(cWORM,betweenPath);
  cursorGoto(cWORMLookup,betweenPath);
  cursorGoto(cWORMGeneric,betweenPath);

  ASSERT_EQ(cRegular.coveringNodeValueDepth(),valAtDepth);
  ASSERT_EQ(cWalk.coveringNodeValueDepth(),valAtDepth);
  ASSERT_EQ(cLookup.coveringNodeValueDepth(),valAtDepth);
  ASSERT_EQ(cWORM.coveringNodeValueDepth(),valAtDepth);
  ASSERT_EQ(cWORMLookup.coveringNodeValueDepth(),valAtDepth);
  ASSERT_EQ(cWORMGeneric.coveringNodeValueDepth(),valAtDepth);

  ASSERT_EQ(cRegular.nodeValueRO().getPtrRO(),nullptr);
  ASSERT_EQ(cWalk.nodeValueRO().getPtrRO(),nullptr);
  ASSERT_EQ(cLookup.nodeValueRO().getPtrRO(),nullptr);
  ASSERT_EQ(cWORM.nodeValueRO().getPtrRO(),nullptr);
  ASSERT_EQ(cWORMLookup.nodeValueRO().getPtrRO(),nullptr);
  ASSERT_EQ(cWORMGeneric.nodeValueRO().getPtrRO(),nullptr);

  // Walk down to the second point, check covering/values here.
  std::size_t val2AtDepth = belowRootPath2.size();
  cRegular = belowRootTree.cursorRO();
  cWalk = belowRootTree.walkCursorRO();
  cLookup = belowRootTree.lookupCursorRO();
  cWORM = belowRootWORMTree.walkCursorRO();
  cWORMLookup = belowRootWORMTree.lookupCursorRO();
  cWORMGeneric = belowRootWORMGenericTree.lookupCursorRO();
  cursorGoto(cRegular,belowRootPath2);
  cursorGoto(cWalk,belowRootPath2);
  cursorGoto(cLookup,belowRootPath2);
  cursorGoto(cWORM,belowRootPath2);
  cursorGoto(cWORMLookup,belowRootPath2);
  cursorGoto(cWORMGeneric,belowRootPath2); 

  ASSERT_EQ(cRegular.coveringNodeValueDepth(),val2AtDepth);
  ASSERT_EQ(cWalk.coveringNodeValueDepth(),val2AtDepth);
  ASSERT_EQ(cLookup.coveringNodeValueDepth(),val2AtDepth);
  ASSERT_EQ(cWORM.coveringNodeValueDepth(),val2AtDepth);
  ASSERT_EQ(cWORMLookup.coveringNodeValueDepth(),val2AtDepth);
  ASSERT_EQ(cWORMGeneric.coveringNodeValueDepth(),val2AtDepth);

  ASSERT_EQ(*(cRegular.nodeValueRO().getPtrRO()),belowRootVal2);
  ASSERT_EQ(*(cWalk.nodeValueRO().getPtrRO()),belowRootVal2);
  ASSERT_EQ(*(cLookup.nodeValueRO().getPtrRO()),belowRootVal2);
  ASSERT_EQ(*(cWORM.nodeValueRO().getPtrRO()),belowRootVal2);
  ASSERT_EQ(*(cWORMLookup.nodeValueRO().getPtrRO()),belowRootVal2);
  ASSERT_EQ(*(cWORMGeneric.nodeValueRO().getPtrRO()),belowRootVal2);

  ASSERT_EQ(*(cRegular.coveringNodeValueRO().getPtrRO()),belowRootVal2);
  ASSERT_EQ(*(cWalk.coveringNodeValueRO().getPtrRO()),belowRootVal2);
  ASSERT_EQ(*(cLookup.coveringNodeValueRO().getPtrRO()),belowRootVal2);
  ASSERT_EQ(*(cWORM.coveringNodeValueRO().getPtrRO()),belowRootVal2);
  ASSERT_EQ(*(cWORMLookup.coveringNodeValueRO().getPtrRO()),belowRootVal2);
  ASSERT_EQ(*(cWORMGeneric.coveringNodeValueRO().getPtrRO()),belowRootVal2);

  // Now go below value 2, check again.
  cRegular.goChild(1);
  cWalk.goChild(1);
  cLookup.goChild(1);
  cWORM.goChild(1);
  cWORMLookup.goChild(1);
  cWORMGeneric.goChild(1);

  ASSERT_EQ(cRegular.coveringNodeValueDepth(),val2AtDepth);
  ASSERT_EQ(cWalk.coveringNodeValueDepth(),val2AtDepth);
  ASSERT_EQ(cLookup.coveringNodeValueDepth(),val2AtDepth);
  ASSERT_EQ(cWORM.coveringNodeValueDepth(),val2AtDepth);
  ASSERT_EQ(cWORMLookup.coveringNodeValueDepth(),val2AtDepth);
  ASSERT_EQ(cWORMGeneric.coveringNodeValueDepth(),val2AtDepth);

  ASSERT_EQ(cRegular.nodeValueRO().getPtrRO(),nullptr);
  ASSERT_EQ(cWalk.nodeValueRO().getPtrRO(),nullptr);
  ASSERT_EQ(cLookup.nodeValueRO().getPtrRO(),nullptr);
  ASSERT_EQ(cWORM.nodeValueRO().getPtrRO(),nullptr);
  ASSERT_EQ(cWORMLookup.nodeValueRO().getPtrRO(),nullptr);
  ASSERT_EQ(cWORMGeneric.nodeValueRO().getPtrRO(),nullptr);

  ASSERT_EQ(*(cRegular.coveringNodeValueRO().getPtrRO()),belowRootVal2);
  ASSERT_EQ(*(cWalk.coveringNodeValueRO().getPtrRO()),belowRootVal2);
  ASSERT_EQ(*(cLookup.coveringNodeValueRO().getPtrRO()),belowRootVal2);
  ASSERT_EQ(*(cWORM.coveringNodeValueRO().getPtrRO()),belowRootVal2);
  ASSERT_EQ(*(cWORMLookup.coveringNodeValueRO().getPtrRO()),belowRootVal2);
  ASSERT_EQ(*(cWORMGeneric.coveringNodeValueRO().getPtrRO()),belowRootVal2);
}



// Test with a value at the root, below,
// at position off the extension in between.
TEST(BinaryTree, TestExitExtension) {
  BinaryUInt32Tree belowRootTree{};
  uint32_t rootVal = 1;
  uint32_t belowVal = 2;
  BinaryPath16 belowPath{0,1,1,0};
  BinaryPath16 betweenPath{0,1,0};
  belowRootTree.cursor().addNode().set(rootVal);
  cursorAddValueAt(belowRootTree.cursor(),belowPath,belowVal);

  BinaryWORM32TreeBuilder buildWORM;
  ASSERT_TRUE(buildWORM.start());
  buildWORM.addNode(BinaryPath16{},true,&rootVal,{true,false});
  buildWORM.addNode(belowPath,true,&belowVal,{false,false});
  ASSERT_TRUE(buildWORM.finish());
  BinaryWORM32Tree belowRootWORMTree(buildWORM.buffer());
  auto belowRootWORMGenericTree = makeWORMTreeUIntGeneric<BinaryPath16>(BinaryWORMTreeUIntParams{true,4,4},buildWORM.buffer());

  auto cRegular = belowRootTree.cursorRO();
  auto cWalk = belowRootTree.walkCursorRO();
  auto cLookup = belowRootTree.lookupCursorRO();
  auto cWORM = belowRootWORMTree.walkCursorRO();
  auto cWORMLookup = belowRootWORMTree.lookupCursorRO();
  auto cWORMGeneric = belowRootWORMGenericTree.walkCursorRO();

  // Verify that we've got our covering value at the root.
  ASSERT_EQ(*(cRegular.coveringNodeValueRO().getPtrRO()),rootVal);
  ASSERT_EQ(*(cWalk.coveringNodeValueRO().getPtrRO()),rootVal);
  ASSERT_EQ(*(cLookup.coveringNodeValueRO().getPtrRO()),rootVal);
  ASSERT_EQ(*(cWORM.coveringNodeValueRO().getPtrRO()),rootVal);
  ASSERT_EQ(*(cWORMLookup.coveringNodeValueRO().getPtrRO()),rootVal);
  ASSERT_EQ(*(cWORMGeneric.coveringNodeValueRO().getPtrRO()),rootVal);

  // Now go down a path, check the covering value again.
  cursorGoto(cRegular,betweenPath);
  cursorGoto(cWalk,betweenPath);
  cursorGoto(cLookup,betweenPath);
  cursorGoto(cWORM,betweenPath);
  cursorGoto(cWORMLookup,betweenPath);
  cursorGoto(cWORMGeneric,betweenPath);

  ASSERT_EQ(*(cRegular.coveringNodeValueRO().getPtrRO()),rootVal);
  ASSERT_EQ(cRegular.coveringNodeValueDepth(),0);
  ASSERT_EQ(*(cWalk.coveringNodeValueRO().getPtrRO()),rootVal);
  ASSERT_EQ(cWalk.coveringNodeValueDepth(),0);
  ASSERT_EQ(*(cLookup.coveringNodeValueRO().getPtrRO()),rootVal);
  ASSERT_EQ(cLookup.coveringNodeValueDepth(),0);
  ASSERT_EQ(*(cWORM.coveringNodeValueRO().getPtrRO()),rootVal);
  ASSERT_EQ(cWORM.coveringNodeValueDepth(),0);
  ASSERT_EQ(*(cWORMLookup.coveringNodeValueRO().getPtrRO()),rootVal);
  ASSERT_EQ(cWORMLookup.coveringNodeValueDepth(),0);
  ASSERT_EQ(*(cWORMGeneric.coveringNodeValueRO().getPtrRO()),rootVal);
  ASSERT_EQ(cWORMGeneric.coveringNodeValueDepth(),0);
}



int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
