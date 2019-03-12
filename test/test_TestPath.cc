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

#include <inttypes.h>
#include <array>
#include <string>

#include "gtest/gtest.h"

#include "TestPath.h"
#include "PathEdgeTestUtils.h"
#include "RandomUtils.h"
#include "PathEdgeTests.h"
#include "PathSort.h"

/**
 * \file test_TestPath.cc
 */

using TestPath3_14 = TestPath<3,14>;
using TestPath3_2 = TestPath<3,2>;
using TestPath37_12 = TestPath<37,12>;

// Build some simple paths, verify that we
// have values in expected places.
TEST(TestPath, BasicPattern) {
  TestPath3_14 p1_initlist{0,1,2,2,1,0};
  std::vector<std::size_t> v1{0,1,2,2,1,0};
  TestPath3_14 p1_vector(v1);
  ASSERT_EQ(p1_vector,pathToVector(p1_initlist));
  ASSERT_EQ("0-1-2-2-1-0/6",pathToString(p1_initlist));
  ASSERT_TRUE(pathsEqual(p1_initlist,p1_vector));
}

TEST(TestPath,RandomOps) {
  std::string testResult = pathRandomOps<TestPath3_14>(1000000);
  ASSERT_EQ(testResult,"OK");

  testResult = pathRandomOps<TestPath37_12>(1000000);
  ASSERT_EQ(testResult,"OK");
}


TEST(TestPath, CommonPrefixTest) {
  TestPath3_14 empty{},m1{1,2,1,0},m2{1,2,1,0,0,0,0,0,0};
  ASSERT_EQ(m1.commonPrefixSize(m2),4);
  ASSERT_EQ(m2.commonPrefixSize(m1),4);
  ASSERT_EQ(m1.commonPrefixSize(empty),0);
  ASSERT_EQ(empty.commonPrefixSize(m1),0);

  TestPath3_14 m4{1,2,1,0,0,0,0,0,0,0,0,0},m5{1,0,1};
  ASSERT_EQ(m4.commonPrefixSize(m5),1);
  ASSERT_EQ(m5.commonPrefixSize(m4),1);
  ASSERT_EQ(m4.commonPrefixSize(m4),12);
  ASSERT_EQ(m5.commonPrefixSize(m5),3);
}

TEST(TestPath,SortTest) {
  // This array represents paths for a fully populated terenary tree, depth 2. The ordering specified in seq
  // is equivalent to labelling the nodes like this:
  //                          0
  //                   1      2       3
  //                 4 5 6  7 8 9  10 11 12 
  //
  std::array<TestPath3_2,13> allDepth2{
    // All 0 trit numbers
    TestPath3_2{},
    // All 1 trit numbers
    TestPath3_2{0},TestPath3_2{1},TestPath3_2{2},
    // All 2 trit integers
    TestPath3_2{0,0},TestPath3_2{0,1},TestPath3_2{0,2},
    TestPath3_2{1,0},TestPath3_2{1,1},TestPath3_2{1,2},
    TestPath3_2{2,0},TestPath3_2{2,1},TestPath3_2{2,2},
  };
  std::array<std::size_t,13> allDepth2Seq{0,1,2,3,4,5,6,7,8,9,10,11,12};
  
  // Doing a pre-order left-to-right traversal of the fully populated tree above gives us the following sequence:
  std::array<std::size_t,13> allDepth2PreSeqLR{0,1,4,5,6,2,7,8,9,3,10,11,12};
  // Doing a pre-order right-to-left traversal of the fully populated tree above gives us the following sequence:
  std::array<std::size_t,13> allDepth2PreSeqRL{0,3,12,11,10,2,9,8,7,1,6,5,4};

  // Check the pre-order sort
  std::array<std::size_t,13> checkPreLR(allDepth2Seq);
  std::sort(checkPreLR.begin(),checkPreLR.end(),[&allDepth2](std::size_t a,std::size_t b) -> bool { return PathSortPreOrder<TestPath3_2>{}(allDepth2[a],allDepth2[b]); });
  ASSERT_EQ(checkPreLR,allDepth2PreSeqLR);
  // Check the pre-order right-to-left sort
  std::array<std::size_t,13> checkPreRL(allDepth2Seq);
  std::sort(checkPreRL.begin(),checkPreRL.end(),[&allDepth2](std::size_t a,std::size_t b) -> bool { return PathSortPreOrder<TestPath3_2,true>{}(allDepth2[a],allDepth2[b]); });
  ASSERT_EQ(checkPreRL,allDepth2PreSeqRL);


  // Doing a post-order left-to-right traversal of the fully populated tree above gives us the following sequence:
  std::array<std::size_t,13> allDepth2PostSeqLR{4,5,6,1,7,8,9,2,10,11,12,3,0};
  // Doing a post-order right-to-left traversal of the fully populated tree above gives us the following sequence:
  std::array<std::size_t,13> allDepth2PostSeqRL{12,11,10,3,9,8,7,2,6,5,4,1,0};
 
  // Check the post-order left-to-right sort
  std::array<std::size_t,13> checkPostLR(allDepth2Seq);
  std::sort(checkPostLR.begin(),checkPostLR.end(),[&allDepth2](std::size_t a,std::size_t b) -> bool { return PathSortPostOrder<TestPath3_2>{}(allDepth2[a],allDepth2[b]); });
  ASSERT_EQ(checkPostLR,allDepth2PostSeqLR);
 // Check the post-order right-to-left sort
  std::array<std::size_t,13> checkPostRL(allDepth2Seq);
  std::sort(checkPostRL.begin(),checkPostRL.end(),[&allDepth2](std::size_t a,std::size_t b) -> bool { return PathSortPostOrder<TestPath3_2,true>{}(allDepth2[a],allDepth2[b]); });
  ASSERT_EQ(checkPostRL,allDepth2PostSeqRL);
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
