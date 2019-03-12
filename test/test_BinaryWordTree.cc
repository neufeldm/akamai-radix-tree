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

#include "RandomUtils.h"
#include "BinaryTestPath.h"
//#include "PathEdgeTestUtils.h"
#include "TreeTestUtils.h"
#include "PathSort.h"
#include "TreeTests.h"

#include "../RadixTree/BinaryPath.h"
#include "../RadixTree/BinaryWordNode.h"
#include "../RadixTree/WordBlockAllocator.h"
#include "../RadixTree/RadixTree.h"
#include "../RadixTree/SimpleStack.h"

using namespace Akamai::Mapper::RadixTree;

using FourWord32Node = BinaryWordNode<uint32_t,WordBlockVectorAllocator>;
using FourWord64Node = BinaryWordNode<uint64_t,WordBlockVectorAllocator>;
using ThreeWord64Node = CompactBinaryWordNode<uint32_t,uint64_t,WordBlockVectorAllocator>;
using ThreeWord32Node = CompactBinaryWordNode<uint16_t,uint32_t,WordBlockVectorAllocator>;

template <std::size_t MaxDepth>
using FourWord32 = RadixTree<BinaryPath<MaxDepth>,FourWord32Node,SimpleFixedDepthStack>;

template <std::size_t MaxDepth>
using FourWord64 = RadixTree<BinaryPath<MaxDepth>,FourWord64Node,SimpleFixedDepthStack>;

template <std::size_t MaxDepth>
using ThreeWord64 = RadixTree<BinaryPath<MaxDepth>,ThreeWord64Node,SimpleFixedDepthStack>;

template <std::size_t MaxDepth>
using ThreeWord32 = RadixTree<BinaryPath<MaxDepth>,ThreeWord32Node,SimpleFixedDepthStack>;

using PathValue16 = TestPathValue<BinaryTestPath<16,uint16_t>,uint64_t>;
using PathValue12 = TestPathValue<BinaryTestPath<12,uint16_t>,uint64_t>;

TEST(BinaryWordTree32, FillTest) {
  RandomNumbers<std::size_t> rn(RandomSeeds::seed(0));
  auto newTree = [](){ return FourWord32<16>{}; };
  std::string result = fillEntireTree<PathValue16,FourWord32<16>>(rn,4,newTree);
  ASSERT_EQ(result,"OK");
}

TEST(BinaryWordTree32, FillSomeOfTest) {
  RandomSeeds seeds;
  RandomNumbers<std::size_t> rnShuffle(seeds.next());
  RandomNumbers<uint64_t> rnChoose(seeds.next());
  std::vector<float> fillRatios{0.9,0.75,0.5,0.25,0.1};
  auto newTree = [](){ return FourWord32<16>{}; };
  for (float fillRatio : fillRatios) {
    std::string result = fillSomeOfTree<PathValue16,FourWord32<16>>(rnShuffle,4,rnChoose,fillRatio,newTree);
    ASSERT_EQ(result,"OK");
  }
}

TEST(BinaryWordTree64, FillTest) {
  RandomNumbers<std::size_t> rn(RandomSeeds::seed(0));
  auto newTree = [](){ return FourWord64<16>{}; };
  std::string result = fillEntireTree<PathValue16,FourWord64<16>>(rn,4,newTree);
  ASSERT_EQ(result,"OK");
}

TEST(BinaryWordTree64, FillSomeOfTest) {
  RandomSeeds seeds;
  RandomNumbers<std::size_t> rnShuffle(seeds.next());
  RandomNumbers<uint64_t> rnChoose(seeds.next());
  std::vector<float> fillRatios{0.9,0.75,0.5,0.25,0.1};
  auto newTree = [](){ return FourWord64<16>{}; };
  for (float fillRatio : fillRatios) {
    std::string result = fillSomeOfTree<PathValue16,FourWord64<16>>(rnShuffle,4,rnChoose,fillRatio,newTree);
    ASSERT_EQ(result,"OK");
  }
}

TEST(CompactBinaryWordTree64, FillTest) {
  RandomNumbers<std::size_t> rn(RandomSeeds::seed(0));
  auto newTree = [](){ return ThreeWord64<16>{}; };
  std::string result = fillEntireTree<PathValue16,ThreeWord64<16>>(rn,4,newTree);
  ASSERT_EQ(result,"OK");
}

TEST(CompactBinaryWordTree64, FillSomeOfTest) {
  RandomSeeds seeds;
  RandomNumbers<std::size_t> rnShuffle(seeds.next());
  RandomNumbers<uint64_t> rnChoose(seeds.next());
  std::vector<float> fillRatios{0.9,0.75,0.5,0.25,0.1};
  auto newTree = [](){ return ThreeWord64<16>{}; };
  for (float fillRatio : fillRatios) {
    std::string result = fillSomeOfTree<PathValue16,ThreeWord64<16>>(rnShuffle,4,rnChoose,fillRatio,newTree);
    ASSERT_EQ(result,"OK");
  }
}

// For the 32 bit word/16 bit value tree we've only got
// 65536 possible values - our test infrastructure
// assumes we can set a unique numeric value at each
// spot in the tree, so we have to reduce our tree
// depth. We're using 12 as an arbitrary value.
TEST(CompactBinaryWordTree32, FillTest) {
  RandomNumbers<std::size_t> rn(RandomSeeds::seed(0));
  auto newTree = [](){ return ThreeWord32<12>{}; };
  std::string result = fillEntireTree<PathValue12,ThreeWord32<12>>(rn,4,newTree);
  ASSERT_EQ(result,"OK");
}

TEST(CompactBinaryWordTree32, FillSomeOfTest) {
  RandomSeeds seeds;
  RandomNumbers<std::size_t> rnShuffle(seeds.next());
  RandomNumbers<uint64_t> rnChoose(seeds.next());
  std::vector<float> fillRatios{0.9,0.75,0.5,0.25,0.1};
  auto newTree = [](){ return ThreeWord32<12>{}; };
  for (float fillRatio : fillRatios) {
    std::string result = fillSomeOfTree<PathValue12,ThreeWord32<12>>(rnShuffle,4,rnChoose,fillRatio,newTree);
    ASSERT_EQ(result,"OK");
  }
}

// Our three word bool and void trees are special. Since
// our testing infrastructure requires the ability to store
// unique values at every point in the tree it isn't much
// use in this case. So we'll test some specific features
// of the trees instead of doing bulk randomized tests.
// Should probably add this functionality to the test
// infrastructure, but at least wanted to get some basic
// tests in for now.

using ThreeWord32BoolNode = CompactBinaryWordNode<bool,uint32_t,WordBlockVectorAllocator>;
using ThreeWord32VoidNode = CompactBinaryWordNode<void,uint32_t,WordBlockVectorAllocator>;

template <std::size_t MaxDepth>
using ThreeWordBool = RadixTree<BinaryPath<MaxDepth>,ThreeWord32BoolNode,SimpleFixedDepthStack>;
template <std::size_t MaxDepth>
using ThreeWordVoid = RadixTree<BinaryPath<MaxDepth>,ThreeWord32VoidNode,SimpleFixedDepthStack>;

using BinaryPath16 = BinaryPath<16>;
using ThreeWordBool16 = ThreeWordBool<16>;
using ThreeWordVoid16 = ThreeWordVoid<16>;

std::vector<BinaryPath16> boolPaths{{},
                                    {1,0,0,1,0,0,1},
                                    {0,1},
                                    {1,1,1,1,1,1},
                                    {0,0,0,1,0},
                                    {1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0}};

std::vector<bool> boolValues{false,
                             true,
                             true,
                             false,
                             true,
                             false};

TEST(CompactBinaryWordTreeBool32, SimpleTest) {
  ThreeWordBool16 tree{};
  auto cursorRW = tree.cursor();
  // First we add some values...
  for (std::size_t i=0;i<boolPaths.size();++i) {
    cursorGoto(cursorRW,boolPaths.at(i));
    cursorRW.addNode().set(boolValues.at(i));
  }
  // ...then we check them
  auto cursorRO = tree.cursorRO();
  for (std::size_t i=0;i<boolPaths.size();++i) {
    cursorGoto(cursorRO,boolPaths.at(i));
    ASSERT_TRUE(cursorRO.atNode());
    ASSERT_TRUE(cursorRO.atValue());
    ASSERT_EQ(*(cursorRO.nodeValueRO().getPtrRO()),boolValues.at(i));
  }
}

TEST(CompactBinaryWordTreeVoid32, SimpleTest) {
  ThreeWordVoid16 tree{};
  auto cursorRW = tree.cursor();
  // First we add some values...
  for (std::size_t i=0;i<boolPaths.size();++i) {
    cursorGoto(cursorRW,boolPaths.at(i));
    cursorRW.addNode().set(boolValues.at(i));
  }
  // ...then we check them
  auto cursorRO = tree.cursorRO();
  for (std::size_t i=0;i<boolPaths.size();++i) {
    cursorGoto(cursorRO,boolPaths.at(i));
    // Checking the void tree is a little different -
    // the places that have "false" set should have
    // nodes but not values, the places that have "true"
    // set should have nodes and values.
    ASSERT_TRUE(cursorRO.atNode());
    ASSERT_EQ(boolValues.at(i),cursorRO.atValue());
  }
}


int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
