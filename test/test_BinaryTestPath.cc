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

#include "BinaryTestPath.h"
#include "PathEdgeTestUtils.h"
#include "RandomUtils.h"
#include "PathEdgeTests.h"
#include "PathSort.h"

/**
 * \file test_BinaryPath.cc
 */


// First do some basic verification of our BinaryTestPath

// Build some simple binary paths, verify that we
// have values in expected places.
TEST(BinaryTestPath, BasicPattern) {
  std::vector<std::size_t> arr1{0x1};
  auto path1 = makePath<BinaryTestPath<32,uint32_t>>(arr1);
  std::string str1 = pathToString(path1);
  ASSERT_EQ(str1,"1/1");

  std::vector<std::size_t> arr10{0x1,0x0};
  auto path10 = makePath<BinaryTestPath<32,uint32_t>>(arr10);
  std::string str10 = pathToString(path10);
  ASSERT_EQ(str10,"1-0/2");
  ASSERT_TRUE(pathsEqual(arr10,path10));

  std::vector<std::size_t> expectedFillvec10{1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0};
  std::vector<std::size_t> checkFillvec10;
  vectorFillPattern(checkFillvec10,32,{1,0});
  ASSERT_EQ(checkFillvec10,expectedFillvec10);

  std::string expectedFillstr10{"1-0-1-0-1-0-1-0-1-0-1-0-1-0-1-0-1-0-1-0-1-0-1-0-1-0-1-0-1-0-1-0/32"};
  std::string checkFillstr10 = pathToString(expectedFillvec10);
  ASSERT_EQ(expectedFillstr10,checkFillstr10);

  auto fillpath10 = makePath<BinaryTestPath<32,uint32_t>>({});
  pathFillPattern(fillpath10,{1,0});
  ASSERT_EQ(fillpath10.size(),32);
  std::string fillstr10 = pathToString(fillpath10);
  ASSERT_EQ(fillstr10,expectedFillstr10);
  std::vector<std::size_t> fillvec10 = pathToVector(fillpath10);
  ASSERT_EQ(fillvec10,expectedFillvec10);
}

TEST(BinaryTestPath,RandomOps) {
  std::string testResult = pathRandomOps<BinaryTestPath<32,uint32_t>>(1000000);
  ASSERT_EQ(testResult,"OK");
  
  testResult = pathRandomOps<BinaryTestPath<31,uint32_t>>(1000000);
  ASSERT_EQ(testResult,"OK");

  testResult = pathRandomOps<BinaryTestPath<9,uint32_t>>(1000000);
  ASSERT_EQ(testResult,"OK");

  testResult = pathRandomOps<BinaryTestPath<1,uint32_t>>(1000000);
  ASSERT_EQ(testResult,"OK");
}


/*
 * Ensure that the member function commonPrefixSize() within 
 * IntPath class (in BinaryTreeUtils.h) works. This is important for
 * traversing intersections of IntPaths (through calls to moveCursorTo()).
 */
TEST(BinaryTestPath, CommonPrefixTest) {
  BinaryTestPath32 empty{},m1{1,1,1,1},m2{1,1,1,1,0,0,0,0,0};
  ASSERT_EQ(m1.commonPrefixSize(m2),4);
  ASSERT_EQ(m2.commonPrefixSize(m1),4);
  ASSERT_EQ(m1.commonPrefixSize(empty),0);
  ASSERT_EQ(empty.commonPrefixSize(m1),0);

  BinaryTestPath32 m4{1,1,1,1,0,0,0,0,0,0,0,0},m5{1,1,1,0};
  ASSERT_EQ(m4.commonPrefixSize(m5),3);
  ASSERT_EQ(m5.commonPrefixSize(m4),3);
  ASSERT_EQ(m4.commonPrefixSize(m4),12);
  ASSERT_EQ(m5.commonPrefixSize(m5),4);
}

/*
 * Check that IntPaths may be sorted in pre-order, in-order, and post-order
 */
TEST(BinaryTestPath,SortTest) {
  // This array represents paths for a fully populated binary tree, depth 3. The ordering specified in seq
  // is equivalent to labelling the nodes like this:
  //                          0
  //                   1             2
  //                3     4       5     6
  //               7 8   9 10   11 12 13 14
  //
  std::array<BinaryTestPath8,15> allDepth3{
    // All 0 bit integers
    BinaryTestPath8{},
    // All 1 bit integers
    BinaryTestPath8{0},BinaryTestPath8{1},
    // All 2 bit integers
    BinaryTestPath8{0,0},BinaryTestPath8{0,1},BinaryTestPath8{1,0},BinaryTestPath8{1,1},
    // All 3 bit integers
    BinaryTestPath8{0,0,0},BinaryTestPath8{0,0,1},BinaryTestPath8{0,1,0},BinaryTestPath8{0,1,1},BinaryTestPath8{1,0,0},BinaryTestPath8{1,0,1},BinaryTestPath8{1,1,0},BinaryTestPath8{1,1,1}
  };
  std::array<uint8_t,15> allDepth3Seq{0,1,2,3,4,5,6,7,8,9,10,11,12,13,14};
  
  // Doing a pre-order left-to-right traversal of the fully populated tree above gives us the following sequence:
  std::array<uint8_t,15> allDepth3PreSeqLR{0,1,3,7,8,4,9,10,2,5,11,12,6,13,14};
  // Doing a pre-order right-to-left traversal of the fully populated tree above gives us the following sequence:
  std::array<uint8_t,15> allDepth3PreSeqRL{0,2,6,14,13,5,12,11,1,4,10,9,3,8,7};

  // Check the pre-order IntPath<> sort routine
  std::array<uint8_t,15> checkPreLR(allDepth3Seq);
  std::sort(checkPreLR.begin(),checkPreLR.end(),[&allDepth3](uint8_t a,uint8_t b) -> bool { return PathSortPreOrder<BinaryTestPath8>{}(allDepth3[a],allDepth3[b]); });
  ASSERT_EQ(checkPreLR,allDepth3PreSeqLR);
  // Check the pre-order right-to-left IntPath<> sort routine
  std::array<uint8_t,15> checkPreRL(allDepth3Seq);
  std::sort(checkPreRL.begin(),checkPreRL.end(),[&allDepth3](uint8_t a,uint8_t b) -> bool { return PathSortPreOrder<BinaryTestPath8,true>{}(allDepth3[a],allDepth3[b]); });
  ASSERT_EQ(checkPreRL,allDepth3PreSeqRL);


  // Doing a post-order left-to-right traversal of the fully populated tree above gives us the following sequence:
  std::array<uint8_t,15> allDepth3PostSeqLR{7,8,3,9,10,4,1,11,12,5,13,14,6,2,0};
  // Doing a post-order right-to-left traversal of the fully populated tree above gives us the following sequence:
  std::array<uint8_t,15> allDepth3PostSeqRL{14,13,6,12,11,5,2,10,9,4,8,7,3,1,0};
 
 // Check the post-order left-to-right IntPath<> sort routine
  std::array<uint8_t,15> checkPostLR(allDepth3Seq);
  std::sort(checkPostLR.begin(),checkPostLR.end(),[&allDepth3](uint8_t a,uint8_t b) -> bool { return PathSortPostOrder<BinaryTestPath8>{}(allDepth3[a],allDepth3[b]); });
  ASSERT_EQ(checkPostLR,allDepth3PostSeqLR);
 // Check the post-order right-to-left IntPath<> sort routine
  std::array<uint8_t,15> checkPostRL(allDepth3Seq);
  std::sort(checkPostRL.begin(),checkPostRL.end(),[&allDepth3](uint8_t a,uint8_t b) -> bool { return PathSortPostOrder<BinaryTestPath8,true>{}(allDepth3[a],allDepth3[b]); });
  ASSERT_EQ(checkPostRL,allDepth3PostSeqRL);

  // Doing an in-order left-to-right traversal of the fully populated tree above gives us the following sequence:
  std::array<uint8_t,15> allDepth3InSeqLR{7,3,8,1,9,4,10,0,11,5,12,2,13,6,14};
  // Doing an in-order right-to-left traversal of the fully populated tree above gives us the following sequence:
  std::array<uint8_t,15> allDepth3InSeqRL{14,6,13,2,12,5,11,0,10,4,9,1,8,3,7};

  // Check the in-order left-to-right IntPath<> sort routine
  std::array<uint8_t,15> checkInLR(allDepth3Seq);
  std::sort(checkInLR.begin(),checkInLR.end(),[&allDepth3](uint8_t a,uint8_t b) -> bool { return PathSortInOrder<BinaryTestPath8>{}(allDepth3[a],allDepth3[b]); });
  ASSERT_EQ(checkInLR,allDepth3InSeqLR);
  // Check the in-order left-to-right IntPath<> sort routine
  std::array<uint8_t,15> checkInRL(allDepth3Seq);
  std::sort(checkInRL.begin(),checkInRL.end(),[&allDepth3](uint8_t a,uint8_t b) -> bool { return PathSortInOrder<BinaryTestPath8,true>{}(allDepth3[a],allDepth3[b]); });
  ASSERT_EQ(checkInRL,allDepth3InSeqRL);
}


int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
