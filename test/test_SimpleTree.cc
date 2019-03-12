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
#include "TestPath.h"
#include "TreeTestUtils.h"
#include "PathSort.h"
#include "TreeTests.h"

#include "SimpleRadixTree.h"

using namespace Akamai::Mapper::RadixTree;

using SimpleBinaryTree16 = SimpleRadixTree<uint64_t,2,16,4>;
using BinaryPathValue16 = TestPathValue<TestPath<2,16>,uint64_t>;


using SimpleTerenaryTree10 = SimpleRadixTree<uint64_t,3,10,3>;
using SimpleTerenaryTreeMap10 = SimpleRadixTreeMap<uint64_t,3,10,3>;
using TerenaryPathValue10 = TestPathValue<TestPath<3,10>,uint64_t>;

using SimpleQuatenaryTree7 = SimpleRadixTree<uint64_t,4,7,3>;
using QuatenaryPathValue7 = TestPathValue<TestPath<4,7>,uint64_t>;


TEST(SimpleBinaryTree, FillTest) {
  auto newTree = [](){ return SimpleBinaryTree16{}; };
  RandomNumbers<std::size_t> rn(RandomSeeds::seed(0));
  std::string result = fillEntireTree<BinaryPathValue16,SimpleBinaryTree16>(rn,2,newTree);
  ASSERT_EQ(result,"OK");
}

TEST(SimpleBinaryTree, FillSomeOfTest) {
  RandomSeeds seeds;
  RandomNumbers<std::size_t> rnShuffle(seeds.next());
  RandomNumbers<uint64_t> rnChoose(seeds.next());
  auto newTree = [](){ return SimpleBinaryTree16{}; };
  std::vector<float> fillRatios{0.5,0.25,0.1};
  for (float fillRatio : fillRatios) {
    std::string result = fillSomeOfTree<BinaryPathValue16,SimpleBinaryTree16>(rnShuffle,2,rnChoose,fillRatio,newTree);
    ASSERT_EQ(result,"OK");
  }
}

TEST(SimpleTerenaryTree, FillTest) {
  auto newTree = [](){ return SimpleTerenaryTree10{}; };
  RandomNumbers<std::size_t> rn(RandomSeeds::seed(0));
  std::string result = fillEntireTree<TerenaryPathValue10,SimpleTerenaryTree10>(rn,2,newTree);
  ASSERT_EQ(result,"OK");
}

TEST(SimpleTerenaryTree, FillSomeOfTest) {
  RandomSeeds seeds;
  RandomNumbers<std::size_t> rnShuffle(seeds.next());
  RandomNumbers<uint64_t> rnChoose(seeds.next());
  auto newTree = [](){ return SimpleTerenaryTree10{}; };
  std::vector<float> fillRatios{0.75,0.5,0.25,0.1};
  for (float fillRatio : fillRatios) {
    std::string result = fillSomeOfTree<TerenaryPathValue10,SimpleTerenaryTree10>(rnShuffle,2,rnChoose,fillRatio,newTree);
    ASSERT_EQ(result,"OK");
  }
}


TEST(SimpleTerenaryTreeMap, FillTest) {
  auto newTree = [](){ return SimpleTerenaryTreeMap10{}; };
  RandomNumbers<std::size_t> rn(RandomSeeds::seed(0));
  std::string result = fillEntireTree<TerenaryPathValue10,SimpleTerenaryTreeMap10>(rn,2,newTree);
  ASSERT_EQ(result,"OK");
}

TEST(SimpleTerenaryTreeMap, FillSomeOfTest) {
  RandomSeeds seeds;
  RandomNumbers<std::size_t> rnShuffle(seeds.next());
  RandomNumbers<uint64_t> rnChoose(seeds.next());
  auto newTree = [](){ return SimpleTerenaryTreeMap10{}; };
  std::vector<float> fillRatios{0.75,0.5,0.25,0.1};
  for (float fillRatio : fillRatios) {
    std::string result = fillSomeOfTree<TerenaryPathValue10,SimpleTerenaryTreeMap10>(rnShuffle,2,rnChoose,fillRatio,newTree);
    ASSERT_EQ(result,"OK");
  }
}

TEST(SimpleQuatenaryTree, FillTest) {
  auto newTree = [](){ return SimpleQuatenaryTree7{}; };
  RandomNumbers<std::size_t> rn(RandomSeeds::seed(0));
  std::string result = fillEntireTree<QuatenaryPathValue7,SimpleQuatenaryTree7>(rn,2,newTree);
  ASSERT_EQ(result,"OK");
}

TEST(SimpleQuatenaryTree, FillSomeOfTest) {
  RandomSeeds seeds;
  RandomNumbers<std::size_t> rnShuffle(seeds.next());
  RandomNumbers<uint64_t> rnChoose(seeds.next());
  auto newTree = [](){ return SimpleQuatenaryTree7{}; };
  std::vector<float> fillRatios{0.75,0.5,0.25,0.1};
  for (float fillRatio : fillRatios) {
    std::string result = fillSomeOfTree<QuatenaryPathValue7,SimpleQuatenaryTree7>(rnShuffle,2,rnChoose,fillRatio,newTree);
    ASSERT_EQ(result,"OK");
  }
}



int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
