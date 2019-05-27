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
#include <type_traits>

#include "gtest/gtest.h"

#include "CursorIterator.h"
#include "TestPath.h"
#include "TreeTestUtils.h"
#include "TreeTests.h"
#include "RandomUtils.h"
#include "PathSort.h"

#include "SimpleRadixTree.h"

using namespace Akamai::Mapper::RadixTree;

template <std::size_t RADIX,std::size_t DEPTH>
using PathVal = TestPathValue<TestPath<RADIX,DEPTH>,uint32_t>;

template <std::size_t RADIX,std::size_t DEPTH>
using TestTree = SimpleRadixTree<uint32_t,RADIX,DEPTH,4>;


template <typename TreeSpotListT,typename CursorIteratorT>
std::string comparePathValueIter(TreeSpotListT&& tsl,CursorIteratorT&& cit) {
  using PathValueType = typename std::decay<TreeSpotListT>::type::PathValueType;
  const std::vector<std::size_t>& spotSequence(tsl.treeSpotSequence());
  const std::vector<PathValueType>& spotValues(tsl.treeSpots());
  std::size_t i = 0;
  while (!cit.finished()) {
    uint32_t expectedValue = spotValues.at(spotSequence.at(i)).value;
    if (!cit->atValue()) {
      return "Iterator missing value at sequence " + std::to_string(i) + " expected " + std::to_string(expectedValue);
    }
    uint32_t gotValue = *(cit->nodeValue().getPtrRO());
    if (gotValue != expectedValue) {
      return "Iterator mismatch value at sequence " + std::to_string(i) +
             " got " + std::to_string(gotValue) +
             " expected " + std::to_string(expectedValue);
    }
    ++cit;
    ++i;
  }
  if (i != spotSequence.size()) {
    return "Iterator finished at wrong place in sequence, got " + std::to_string(i) +
           " expected " + std::to_string(spotSequence.size());
  }

  return "OK";
}

template <std::size_t RADIX,std::size_t DEPTH>
typename std::enable_if<(RADIX % 2) != 0,std::string>::type
buildAndCheckIter(TreeSpotList<PathVal<RADIX,DEPTH>>& tsl) {
  using TreeType = TestTree<RADIX,DEPTH>;
  using PathValueType = PathVal<RADIX,DEPTH>;
  std::string r;
  TreeType tree{};
  tsl.addToTree(tree.cursor());  
  tsl.sort(PathSortPreOrder<PathValueType>{});
  r = comparePathValueIter(tsl,make_preorder_iterator(tree.walkCursorRO()));
  if (r != "OK") { return "[ComparePreOrder] " + r; }
  tsl.sort(PathSortPostOrder<PathValueType>{});
  r = comparePathValueIter(tsl,make_postorder_iterator(tree.walkCursorRO()));
  if (r != "OK") { return "[ComparePostOrder] " + r; }

  tsl.sort(PathSortPreOrder<PathValueType,true>{});
  r = comparePathValueIter(tsl,make_preorder_iterator<true>(tree.walkCursorRO()));
  if (r != "OK") { return "[ComparePreOrder-ReverseChildren] " + r; }
  tsl.sort(PathSortPostOrder<PathValueType,true>{});
  r = comparePathValueIter(tsl,make_postorder_iterator<true>(tree.walkCursorRO()));
  if (r != "OK") { return "[ComparePostOrder-ReverseChildren] " + r; }

  return "OK";
}

template <std::size_t RADIX,std::size_t DEPTH>
typename std::enable_if<(RADIX % 2) == 0,std::string>::type
buildAndCheckIter(TreeSpotList<PathVal<RADIX,DEPTH>>& tsl) {
  using TreeType = TestTree<RADIX,DEPTH>;
  using PathValueType = PathVal<RADIX,DEPTH>;
  std::string r;
  TreeType tree{};
  tsl.addToTree(tree.cursor());  
  tsl.sort(PathSortPreOrder<PathValueType>{});
  r = comparePathValueIter(tsl,make_preorder_iterator(tree.walkCursorRO()));
  if (r != "OK") { return "[ComparePreOrder] " + r; }
  tsl.sort(PathSortPostOrder<PathValueType>{});
  r = comparePathValueIter(tsl,make_postorder_iterator(tree.walkCursorRO()));
  if (r != "OK") { return "[ComparePostOrder] " + r; }
  tsl.sort(PathSortInOrder<PathValueType>{});
  r = comparePathValueIter(tsl,make_inorder_iterator(tree.walkCursorRO()));
  if (r != "OK") { return "[CompareInOrder] " + r; }

  tsl.sort(PathSortPreOrder<PathValueType,true>{});
  r = comparePathValueIter(tsl,make_preorder_iterator<true>(tree.walkCursorRO()));
  if (r != "OK") { return "[ComparePreOrder-ReverseChildren] " + r; }
  tsl.sort(PathSortPostOrder<PathValueType,true>{});
  r = comparePathValueIter(tsl,make_postorder_iterator<true>(tree.walkCursorRO()));
  if (r != "OK") { return "[ComparePostOrder-ReverseChildren] " + r; }
  tsl.sort(PathSortInOrder<PathValueType,true>{});
  r = comparePathValueIter(tsl,make_inorder_iterator<true>(tree.walkCursorRO()));
  if (r != "OK") { return "[CompareInOrder-ReverseChildren] " + r; }

  return "OK";
}

template <std::size_t RADIX,std::size_t DEPTH>
std::string testFillTree() {
  using PathValueType = PathVal<RADIX,DEPTH>;
  std::string testIDStr = "[FillAll-" + std::to_string(RADIX) + "-" + std::to_string(DEPTH) + "]";
  TreeSpotList<PathValueType> filledTree = spotListFillTree<PathValueType>();
  //std::cout << testIDStr << "spot count: " << std::to_string(filledTree.treeSpots().size()) << std::endl;
  std::string r = buildAndCheckIter<RADIX,DEPTH>(filledTree);
  if (r != "OK") { return testIDStr + r; }
  return "OK";
}

template <std::size_t RADIX,std::size_t DEPTH>
std::string testFillSomeRandom(const std::vector<double>& fillRatios) {
  using PathValueType = PathVal<RADIX,DEPTH>;
  std::string baseTestIDStr = "FillSomeRandom-" + std::to_string(RADIX) + "-" + std::to_string(DEPTH);
  RandomNumbers<uint64_t> rn(RandomSeeds::seed(0));
  for (double fillRatio : fillRatios) {
    std::string testIDStr = "[" + baseTestIDStr + "-" + std::to_string(fillRatio) + "] ";
    TreeSpotList<PathValueType> curFill = spotListFillSomeOfTree<PathValueType>(rn,fillRatio);
    std::string r = buildAndCheckIter<RADIX,DEPTH>(curFill);
    if (r != "OK") { return testIDStr + r; }
  }
  return "OK";
}

TEST(CursorIteration, SmallTrees) {
  std::string r;
  r = testFillTree<2,3>();
  r = testFillTree<2,4>();
  r = testFillTree<2,5>();
  r = testFillTree<2,10>();

  r = testFillTree<3,3>();
  r = testFillTree<3,4>();
  r = testFillTree<3,7>();
  r = testFillSomeRandom<3,7>({0.9,0.7,0.5,0.3,0.1});

  r = testFillTree<4,3>();
  r = testFillTree<4,4>();
  r = testFillTree<4,5>();
  r = testFillTree<4,6>();
  r = testFillSomeRandom<4,6>({0.9,0.7,0.5,0.3,0.1});

  ASSERT_EQ(r,"OK");
}


TEST(CursorIteration, BiggerTrees) {
  std::string r;
  r = testFillTree<2,12>();
  r = testFillSomeRandom<2,12>({0.5,0.3,0.1});
  r = testFillTree<2,13>();
  r = testFillSomeRandom<2,13>({0.5,0.3,0.1});
  r = testFillTree<2,14>();
  r = testFillSomeRandom<2,14>({0.5,0.3,0.1});
  r = testFillTree<2,15>();
  r = testFillSomeRandom<2,15>({0.5,0.3,0.1});

  r = testFillSomeRandom<3,10>({0.3,0.1,0.05});

  r = testFillSomeRandom<4,8>({0.3,0.1,0.05});

  ASSERT_EQ(r,"OK");
}


int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
