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

// Our WordArray node is also special. The automated tests
// rely on storing a single integer value at various points
// in the tree, the array isn't strictly compatible with that.
// Fortunately the only part of the WordArray node that's actually
// different than the Word node is setting/getting values, so we
// can rely on the baseline structure testing of the Word node.
// We'll do some simple spot topology testing like we've done
// with the void and bool variants to make sure that we're
// able to get/set values correctly.

template <std::size_t ArrayValueWords>
using ArrayWord32Node = BinaryWordArrayNode<uint32_t,ArrayValueWords,WordBlockVectorAllocator>;

template <std::size_t ArrayValueWords,std::size_t MaxDepth>
using ArrayWord32 = RadixTree<BinaryPath<MaxDepth>,ArrayWord32Node<ArrayValueWords>,SimpleFixedDepthStack>;

std::vector<BinaryPath16> wordArrayPaths{{},
                                         {1,0,0,1,0,0,1},
                                         {0,1},
                                         {1,1,1,1,1,1},
                                         {0,0,0,1,0},
                                         {1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0}};

// Element 0 of each word array value starts here, we add
// one for each following array element in sequence.
std::vector<uint32_t> wordArrayBaseValues{1000,
                                          2000,
                                          3000,
                                          4000,
                                          5000,
                                          6000};

template <std::size_t ArrayValueWords>
std::array<uint32_t,ArrayValueWords>
getExpectedArrayValue(std::size_t valueIndex) {
  std::array<uint32_t,ArrayValueWords> v;
  for (std::size_t i=0;i<ArrayValueWords;++i) { v[i] = wordArrayBaseValues.at(valueIndex) + i; }
  return v;
}

template <std::size_t ArrayValueWords>
std::string
arrayValueWord32SimpleTest() {
  ArrayWord32<ArrayValueWords,16> arrayWordTree{};
  using ValueType = std::array<uint32_t,ArrayValueWords>;

  // First set some values
  auto cursorRW = arrayWordTree.cursor();
  for (std::size_t i=0;i<wordArrayPaths.size();++i) {
    cursorGoto(cursorRW,wordArrayPaths.at(i));
    ValueType curValue = getExpectedArrayValue<ArrayValueWords>(i);
    cursorRW.addNode().set(curValue);
  }

  // Second go back and check them
  auto cursorRO = arrayWordTree.cursorRO();
  for (std::size_t i=0;i<wordArrayPaths.size();++i) {
    cursorGoto(cursorRO,wordArrayPaths.at(i));
    if (!cursorRO.atValue()) { return "No value at path: " + wordArrayPaths.at(i).toBinaryString(); }
    ValueType expectedValue = getExpectedArrayValue<ArrayValueWords>(i);
    ValueType foundValue = *(cursorRO.nodeValueRO().getPtrRO());
    if (foundValue != expectedValue) {
      std::string foundValueStr,expectedValueStr;
      for (std::size_t j=0;j<ArrayValueWords;++j) {
        foundValueStr += "[" + std::to_string(foundValue.at(j)) + "]";
        expectedValueStr += "[" + std::to_string(expectedValue.at(j)) + "]";
      }
      return ("Value at path " + wordArrayPaths.at(i).toBinaryString() + " " +
               foundValueStr + " != expected value " + expectedValueStr);
    }
  }
  return "OK";
}

TEST(WordArrayNode32,SimpleTest) {
  std::string result;
  result = arrayValueWord32SimpleTest<1>();
  ASSERT_EQ(result,"OK");
  result = arrayValueWord32SimpleTest<2>();
  ASSERT_EQ(result,"OK");
  result = arrayValueWord32SimpleTest<3>();
  ASSERT_EQ(result,"OK");
  result = arrayValueWord32SimpleTest<4>();
  ASSERT_EQ(result,"OK");
  result = arrayValueWord32SimpleTest<5>();
  ASSERT_EQ(result,"OK");
  result = arrayValueWord32SimpleTest<6>();
  ASSERT_EQ(result,"OK");
  result = arrayValueWord32SimpleTest<7>();
  ASSERT_EQ(result,"OK");
  result = arrayValueWord32SimpleTest<8>();
  ASSERT_EQ(result,"OK");
}


int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
