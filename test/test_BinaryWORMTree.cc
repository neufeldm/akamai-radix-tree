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
#include <vector>

#include "gtest/gtest.h"

#include "CursorTraversal.h"
#include "TestPath.h"
#include "TreeTestUtils.h"
#include "TreeTests.h"
#include "RandomUtils.h"

#include "BinaryRadixTree.h"
#include "BinaryWORMTree.h"
#include "BinaryWORMTreeBuilder.h"
#include "BinaryWORMCursorRO.h"
#include "BinaryWORMTreeUInt.h"

using namespace Akamai::Mapper::RadixTree;

// little/big endian
// integer sizes 1..8

template <std::size_t UINTSIZE,bool LITTLEENDIAN>
std::string randomUIntOps() {
  static constexpr std::size_t UIntSize = UINTSIZE;
  using UIntOps = BinaryWORMNodeUIntOps<UINTSIZE,LITTLEENDIAN>;
  using UIntType = typename UIntOps::UIntType;
  const std::string configStr = "[randomUIntOps-" + std::to_string(UIntSize) + "-" + (LITTLEENDIAN ? "LITTLE" : "BIG" ) + "]";

  RandomNumbers<UIntType> randomNumbers(RandomSeeds::seed(1));
  uint8_t uintBuf[UIntSize];
  for (std::size_t i=0;i<10000;++i) {
    UIntType curUInt = (UIntOps::UINT_MASK & randomNumbers.next());
    UIntOps::writeUInt(uintBuf,curUInt);
    UIntType curUIntTest = UIntOps::readUInt(uintBuf);
    if (curUInt != curUIntTest) {
      return configStr + " " + std::to_string(curUInt) + " != " + std::to_string(curUIntTest) + " written/read";
    }
  }
  return "OK";
}

template <bool LITTLEENDIAN>
std::string allRandomUIntOpsForEndian() {
  std::string r;
  r = randomUIntOps<1,LITTLEENDIAN>();
  if (r != "OK") { return r; }
  r = randomUIntOps<2,LITTLEENDIAN>();
  if (r != "OK") { return r; }
  r = randomUIntOps<3,LITTLEENDIAN>();
  if (r != "OK") { return r; }
  r = randomUIntOps<4,LITTLEENDIAN>();
  if (r != "OK") { return r; }
  r = randomUIntOps<5,LITTLEENDIAN>();
  if (r != "OK") { return r; }
  r = randomUIntOps<6,LITTLEENDIAN>();
  if (r != "OK") { return r; }
  r = randomUIntOps<7,LITTLEENDIAN>();
  if (r != "OK") { return r; }
  r = randomUIntOps<8,LITTLEENDIAN>();
  if (r != "OK") { return r; }
  return "OK";
}

TEST(BinaryWORMNode, UIntOps) {
  std::string r;
  r = allRandomUIntOpsForEndian<false>();
  ASSERT_EQ(r,"OK");
  r = allRandomUIntOpsForEndian<true>();
  ASSERT_EQ(r,"OK");
}

/*
 * Traverses cursor in pre-order; callback executed when cursor is at a node.
 */
template <typename PathType,typename CursorType,typename CallbackType>
void preOrderWalkNodes(PathType&& p,CursorType&& c,CallbackType&& cb) { 
  static constexpr std::size_t radix = std::remove_reference<CursorType>::type::Radix;
  if (c.atNode()) { cb(std::forward<PathType>(p),std::forward<CursorType>(c)); }
  for (std::size_t child = 0; child < radix; ++child) {
    if (c.canGoChildNode(child)) {
      p.push_back(child);
      c.goChild(child);
      preOrderWalkNodes(std::forward<PathType>(p),std::forward<CursorType>(c),std::forward<CallbackType>(cb));
      c.goParent();
      p.pop_back();
    }
  }
}



template <std::size_t DEPTH>
using BinaryTestPath = TestPath<2,DEPTH>;

template <std::size_t DEPTH>
using PathVal = TestPathValue<BinaryTestPath<DEPTH>,uint32_t>;

template <std::size_t DEPTH>
using BinaryTreeUInt32 = Akamai::Mapper::RadixTree::BinaryRadixTree32<uint32_t,DEPTH>;

template <std::size_t INTSIZE>
using ReadWriteUInt = BinaryWORMReadWriteUInt<INTSIZE,false>;

template <std::size_t OFFSETSIZE,std::size_t INTSIZE>
using WORMNodeWO = BinaryWORMNodeWO<OFFSETSIZE,false,ReadWriteUInt<INTSIZE>>;

template <std::size_t OFFSETSIZE,std::size_t INTSIZE>
using WORMNodeRO = BinaryWORMNodeRO<OFFSETSIZE,false,ReadWriteUInt<INTSIZE>>;

template <std::size_t OFFSETSIZE,std::size_t INTSIZE,std::size_t DEPTH>
using WORMTreeBuilder = BinaryWORMTreeBuilderVector<BinaryTestPath<DEPTH>,WORMNodeWO<OFFSETSIZE,INTSIZE>>;

template <std::size_t OFFSETSIZE,std::size_t INTSIZE,std::size_t DEPTH>
using WORMTree = BinaryWORMTreeVector<BinaryTestPath<DEPTH>,WORMNodeRO<OFFSETSIZE,INTSIZE>>;

template <std::size_t OFFSETSIZE,std::size_t INTSIZE,std::size_t DEPTH>
using WORMCursorRO = BinaryWORMCursorRO<BinaryTestPath<DEPTH>,WORMNodeRO<OFFSETSIZE,INTSIZE>,SimpleFixedDepthStack>;

template <std::size_t OFFSETSIZE,std::size_t INTSIZE>
using WORMNodeValueType = typename WORMNodeRO<OFFSETSIZE,INTSIZE>::ValueType;

template <typename WORMNodeROT>
std::string nodeToString(WORMNodeROT&& n) {
  std::string nodeStr;
  nodeStr += "+";
  for (std::size_t c = 0; c < n.edgeStepCount(); ++c) {
    nodeStr += std::to_string(n.edgeStepAt(c));
  }
  nodeStr += "@";
  bool hasLeftChild = n.hasChild(0);
  bool hasRightChild = n.hasChild(1);
  if (hasLeftChild && hasRightChild) { nodeStr += "B"; }
  else if (hasLeftChild) { nodeStr += "L"; }
  else if (hasRightChild) { nodeStr += "R"; }
  else { nodeStr += "T"; }
  nodeStr += "[";
  if (hasLeftChild && hasRightChild) {
    nodeStr += std::to_string(static_cast<std::size_t>(n.rightChildOffset()));
  }
  nodeStr += "]";
  if (n.hasValue()) {
    typename std::decay<WORMNodeROT>::type::ValueType v{};
    n.readValue(&v);
    nodeStr += "V(" + std::to_string(v) + ")";
  } else {
    nodeStr += "N()";
  }
  return nodeStr;
}

template <std::size_t WORMOFFSET,std::size_t WORMINT>
std::vector<std::string> dumpWORMNodes(const uint8_t* nodeBuffer,std::size_t size) {
  std::vector<std::string> nodeStrings{};
  std::size_t readAt{0};
  std::size_t atNode{0};
  while (readAt < size) {
    WORMNodeRO<WORMOFFSET,WORMINT> curNode{};
    curNode.setPtr(nodeBuffer + readAt);
    WORMNodeValueType<WORMOFFSET,WORMINT> v{};
    if (curNode.hasValue()) { curNode.readValue(&v); }
    std::string curNodeStr{};
    // Get byte offset
    curNodeStr += (std::to_string(readAt) + " " + nodeToString(curNode) + " # " + std::to_string(atNode++));
    readAt += (curNode.headerSize() + curNode.valueSize());
    nodeStrings.push_back(std::move(curNodeStr));
  }
  return nodeStrings;
}

template <std::size_t WORMOFFSET,std::size_t WORMINT,std::size_t DEPTH>
std::string buildAndCheckWORM(TreeSpotList<PathVal<DEPTH>>& tsl,bool showAddedNodes = false,bool printNodeDump = false) {
  using TreeType = BinaryTreeUInt32<DEPTH>;
  TreeType tree{};
  tsl.addToTree(tree.cursor());
  WORMTreeBuilder<WORMOFFSET,WORMINT,DEPTH> wormBuilder;
  auto srcCursor = tree.cursorRO();
  auto buildWORMCB = 
    [&wormBuilder,showAddedNodes](const BinaryTestPath<DEPTH>& p,const decltype(srcCursor)& c) {
      bool hasLeftChild = c.canGoChildNode(0);
      bool hasRightChild = c.canGoChildNode(1);
      if (c.atValue() || (hasLeftChild && hasRightChild)) {
        if (showAddedNodes) { std::cout << "ADDNODE " << pathToString(p) << (c.atValue() ? " -> " + std::to_string(*(c.nodeValueRO().getPtrRO())) : "") << std::endl; }
        wormBuilder.addNode(p,c.atValue(),c.nodeValueRO().getPtrRO(),{hasLeftChild,hasRightChild});
      }
    };
  if (!wormBuilder.start(true)) { return "Unable to start building dry run tree"; };
  preOrderWalkNodes(BinaryTestPath<DEPTH>{},srcCursor,buildWORMCB);
  if (!wormBuilder.finish()) { return "Unable to finish building dry run tree"; }
  auto dryRunTreeStats = wormBuilder.treeStats();

  if (!wormBuilder.start()) { return "Unable to start building tree"; };
  preOrderWalkNodes(BinaryTestPath<DEPTH>{},srcCursor,buildWORMCB);
  if (!wormBuilder.finish()) { return "Unable to finish building tree"; }
  auto treeStats = wormBuilder.treeStats();

  // compare the dry run/final tree stats
  if (dryRunTreeStats != treeStats) { return "Dry run stats not equal to actual stats"; }
  //std::cout << "Largest offset: " << std::to_string(dryRunTreeStats.longestOffsetGap.at(WORMOFFSET)) << std::endl;
  if (printNodeDump) {
    const std::vector<uint8_t>& wormVector(wormBuilder.buffer());
    std::vector<std::string> wormTreeNodeStrings = dumpWORMNodes<WORMOFFSET,WORMINT>(wormVector.data(),wormVector.size());
    for (const std::string& nodeStr : wormTreeNodeStrings) { std::cout << nodeStr << std::endl; }
  }
  // make the WORM tree object
  WORMTree<WORMOFFSET,WORMINT,DEPTH> wormTree(wormBuilder.extractBuffer());

  // compare with our original list
  std::string r{};
  r = tsl.checkTree(wormTree.cursorRO());
  if (r != "OK") { return "[Check cursorRO] " + r; }
  r = tsl.checkTree(wormTree.cursorRO(),true);
  if (r != "OK") { return "[Check cursorRO from root] " + r; }
  r = tsl.checkTreeNewCursor([&wormTree](){return wormTree.lookupCursorRO();});
  if (r != "OK") { return "[Check lookupCursorRO] " +  r; }

  return "OK";
}

template <std::size_t DEPTH>
std::string buildAndCheckWORMGeneric(TreeSpotList<PathVal<DEPTH>>& tsl) {
  using TreeType = BinaryTreeUInt32<DEPTH>;
  using PathType = typename TreeType::CursorType::PathType;
  TreeType tree{};
  tsl.addToTree(tree.cursor());
  BinaryWORMTreeUIntParams minWORMParams = findMinimumWORMTreeUIntParameters(tree.cursorRO());
  BinaryWORMTreeUIntGeneric<PathType> wormTree = buildWORMTreeUIntGeneric(minWORMParams,tree.cursorRO());

  // compare with our original list
  std::string r{};
  r = tsl.checkTree(wormTree.cursorRO());
  if (r != "OK") { return "[Check cursorRO] " + r; }
  r = tsl.checkTree(wormTree.cursorRO(),true);
  if (r != "OK") { return "[Check cursorRO from root] " + r; }
  r = tsl.checkTreeNewCursor([&wormTree](){return wormTree.lookupCursorRO();});
  if (r != "OK") { return "[Check lookupCursorRO] " +  r; }

  return "OK";
}


template <std::size_t WORMOFFSET,std::size_t WORMINT,std::size_t DEPTH>
std::string testFillTree() {
  std::string testIDStr = "[FillAll-" + std::to_string(WORMOFFSET) + "-" + std::to_string(WORMINT) + "-" + std::to_string(DEPTH) + "]";
  TreeSpotList<PathVal<DEPTH>> filledTree = spotListFillTree<PathVal<DEPTH>>();
  std::string r = buildAndCheckWORM<WORMOFFSET,WORMINT,DEPTH>(filledTree);
  if (r != "OK") { return testIDStr + r; }
  return "OK";
}

template <std::size_t WORMOFFSET,std::size_t WORMINT,std::size_t DEPTH>
std::string testFillSomeRandom(const std::vector<double>& fillRatios) {
  std::string baseTestIDStr = "FillSomeRandom-" + std::to_string(WORMOFFSET) + "-" + std::to_string(WORMINT) + "-" + std::to_string(DEPTH);
  RandomNumbers<uint64_t> rn(RandomSeeds::seed(0));
  for (double fillRatio : fillRatios) {
    std::string testIDStr = "[" + baseTestIDStr + "-" + std::to_string(fillRatio) + "] ";
    TreeSpotList<PathVal<DEPTH>> curFill = spotListFillSomeOfTree<PathVal<DEPTH>>(rn,fillRatio);
    std::string r = buildAndCheckWORM<WORMOFFSET,WORMINT,DEPTH>(curFill);
    if (r != "OK") { return testIDStr + r; }
  }
  return "OK";
}

template <std::size_t DEPTH>
std::string testFillTreeGeneric() {
  std::string testIDStr = "[FillAllGeneric-" + std::to_string(DEPTH) + "]";
  TreeSpotList<PathVal<DEPTH>> filledTree = spotListFillTree<PathVal<DEPTH>>();
  std::string r = buildAndCheckWORMGeneric<DEPTH>(filledTree);
  if (r != "OK") { return testIDStr + r; }
  return "OK";
}

template <std::size_t DEPTH>
std::string testFillSomeRandomGeneric(const std::vector<double>& fillRatios) {
  std::string baseTestIDStr = "FillSomeRandomGeneric-" + std::to_string(DEPTH);
  RandomNumbers<uint64_t> rn(RandomSeeds::seed(0));
  for (double fillRatio : fillRatios) {
    std::string testIDStr = "[" + baseTestIDStr + "-" + std::to_string(fillRatio) + "] ";
    TreeSpotList<PathVal<DEPTH>> curFill = spotListFillSomeOfTree<PathVal<DEPTH>>(rn,fillRatio);
    std::string r = buildAndCheckWORMGeneric<DEPTH>(curFill);
    if (r != "OK") { return testIDStr + r; }
  }
  return "OK";
}



TEST(BinaryWORMTree, SmallTrees) {
  std::string r;
  r = testFillTree<1,4,2>();
  ASSERT_EQ(r,"OK");
  r = testFillTreeGeneric<2>();
  ASSERT_EQ(r,"OK");


  r = testFillTree<1,4,3>();
  ASSERT_EQ(r,"OK");
  r = testFillTreeGeneric<3>();
  ASSERT_EQ(r,"OK");

  r = testFillTree<1,4,4>();
  ASSERT_EQ(r,"OK");
  r = testFillTreeGeneric<4>();
  ASSERT_EQ(r,"OK");


  r = testFillSomeRandom<1,4,4>({0.9,0.8,0.7,0.6,0.5,0.4,0.3,0.2,0.1});
  ASSERT_EQ(r,"OK");
  r = testFillSomeRandomGeneric<4>({0.9,0.8,0.7,0.6,0.5,0.4,0.3,0.2,0.1});
  ASSERT_EQ(r,"OK");

  r = testFillTree<3,4,4>();
  ASSERT_EQ(r,"OK");
  r = testFillTreeGeneric<4>();
  ASSERT_EQ(r,"OK");

  r = testFillSomeRandom<3,4,4>({0.9,0.8,0.7,0.6,0.5,0.4,0.3,0.2,0.1});
  ASSERT_EQ(r,"OK");
  r = testFillSomeRandomGeneric<4>({0.9,0.8,0.7,0.6,0.5,0.4,0.3,0.2,0.1});
  ASSERT_EQ(r,"OK");

  r = testFillTree<4,4,4>();
  ASSERT_EQ(r,"OK");
  r = testFillTreeGeneric<4>();
  ASSERT_EQ(r,"OK");

  r = testFillSomeRandom<4,4,4>({0.9,0.8,0.7,0.6,0.5,0.4,0.3,0.2,0.1});
  ASSERT_EQ(r,"OK");
  r = testFillSomeRandomGeneric<4>({0.9,0.8,0.7,0.6,0.5,0.4,0.3,0.2,0.1});
  ASSERT_EQ(r,"OK");
}

TEST(BinaryWORMTree, MediumTrees) {
  std::string r;

  r = testFillTree<4,4,13>();
  ASSERT_EQ(r,"OK");

  r = testFillSomeRandom<4,4,13>({0.75,0.5,0.25,0.1,0.05});
  ASSERT_EQ(r,"OK");


  r = testFillTree<4,4,16>();
  ASSERT_EQ(r,"OK");

  r = testFillSomeRandom<4,4,16>({0.75,0.5,0.25,0.1,0.05});
  ASSERT_EQ(r,"OK");
}

TEST(BinaryWORMTree, LargeTrees) {
  std::string r;

  r = testFillSomeRandom<4,4,24>({0.05,0.025,0.01});
  ASSERT_EQ(r,"OK");

  r = testFillSomeRandom<4,4,19>({0.05,0.025,0.01});
  ASSERT_EQ(r,"OK");
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
