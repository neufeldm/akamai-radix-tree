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

#include "../RadixTree/BinaryPath.h"
#include "gtest/gtest.h"

#include "PathEdgeTests.h"

/**
 * \file test_BinaryPath.cc
 */

using namespace Akamai::Mapper::RadixTree;

// First do some basic verification of our BinaryTestPath


/*
 * Verify that we get the expected number of bytes used
 * to store our path.
 */
TEST(BinaryPath, PathStorageByteCount) {
  ASSERT_EQ(BinaryPath<0>::byteCapacity(),1u);
  ASSERT_EQ(BinaryPath<1>::byteCapacity(),1u);
  ASSERT_EQ(BinaryPath<2>::byteCapacity(),1u);
  ASSERT_EQ(BinaryPath<3>::byteCapacity(),1u);
  ASSERT_EQ(BinaryPath<4>::byteCapacity(),1u);
  ASSERT_EQ(BinaryPath<5>::byteCapacity(),1u);
  ASSERT_EQ(BinaryPath<6>::byteCapacity(),1u);
  ASSERT_EQ(BinaryPath<7>::byteCapacity(),1u);
  ASSERT_EQ(BinaryPath<8>::byteCapacity(),1u);
  ASSERT_EQ(BinaryPath<9>::byteCapacity(),2u);
  ASSERT_EQ(BinaryPath<10>::byteCapacity(),2u);
  ASSERT_EQ(BinaryPath<11>::byteCapacity(),2u);
  ASSERT_EQ(BinaryPath<12>::byteCapacity(),2u);
  ASSERT_EQ(BinaryPath<13>::byteCapacity(),2u);
  ASSERT_EQ(BinaryPath<14>::byteCapacity(),2u);
  ASSERT_EQ(BinaryPath<15>::byteCapacity(),2u);
  ASSERT_EQ(BinaryPath<16>::byteCapacity(),2u);
  ASSERT_EQ(BinaryPath<17>::byteCapacity(),3u);
  ASSERT_EQ(BinaryPath<32>::byteCapacity(),4u);
  ASSERT_EQ(BinaryPath<33>::byteCapacity(),5u);
}


TEST(BinaryPath_3,RandomOps) {
  std::string testResult = pathRandomOps<BinaryPath<3>>(1000000);
  ASSERT_EQ(testResult,"OK");
}

TEST(BinaryPath_8,RandomOps) {
  std::string testResult = pathRandomOps<BinaryPath<8>>(1000000);
  ASSERT_EQ(testResult,"OK");
}


TEST(BinaryPath_9,RandomOps) {
  std::string testResult = pathRandomOps<BinaryPath<9>>(1000000);
  ASSERT_EQ(testResult,"OK");
}

TEST(BinaryPath_16,RandomOps) {
  std::string testResult = pathRandomOps<BinaryPath<16>>(1000000);
  ASSERT_EQ(testResult,"OK");
}


TEST(BinaryPath_37,RandomOps) {
  std::string testResult = pathRandomOps<BinaryPath<37>>(1000000);
  ASSERT_EQ(testResult,"OK");
}

TEST(BinaryPath_128,RandomOps) {
  std::string testResult = pathRandomOps<BinaryPath<128>>(1000000);
  ASSERT_EQ(testResult,"OK");
}


TEST(BinaryPath_143,RandomOps) {
  std::string testResult = pathRandomOps<BinaryPath<143>>(1000000);
  ASSERT_EQ(testResult,"OK");
}



template <std::size_t bits>
bool fillRefill() {
  BinaryPath<bits> p;
  std::string pBinaryStr;
  for (std::size_t i = 0; i < bits; ++i) {
    p.push_back(1);
    if (p.at(i) != 1) { return false; }
  }
  for (std::size_t i = 0; i < bits; ++i) {
    p.pop_back();
  }
  for (std::size_t i = 0; i < bits; ++i) {
    p.push_back(0);
    if (p.at(i) != 0) { return false; }
  }
  return true;
}

TEST(BinaryPath, FillRefill) {
  ASSERT_TRUE(fillRefill<1>());
  ASSERT_TRUE(fillRefill<2>());
  ASSERT_TRUE(fillRefill<3>());
  ASSERT_TRUE(fillRefill<4>());
  ASSERT_TRUE(fillRefill<5>());
  ASSERT_TRUE(fillRefill<6>());
  ASSERT_TRUE(fillRefill<7>());
  ASSERT_TRUE(fillRefill<8>());
  ASSERT_TRUE(fillRefill<9>());
  ASSERT_TRUE(fillRefill<16>());
  ASSERT_TRUE(fillRefill<18>());
  ASSERT_TRUE(fillRefill<19>());
  ASSERT_TRUE(fillRefill<128>());
  ASSERT_TRUE(fillRefill<129>());
}

TEST(BinaryPath, ToFromBinaryString) {
  
  /* Test for valid binary string with separators */
  
  std::string binaryStr("1010.1111.1010");
  BinaryPath<12> p12;
  ASSERT_TRUE(p12.fromBinaryString(binaryStr));
  std::string r12 = p12.toBinaryString();
  ASSERT_EQ(binaryStr,r12);
  
  /* Test that binary string of length 12 will not fit in path of depth 6 */

  BinaryPath<6> p6;
  ASSERT_FALSE(p6.fromBinaryString(binaryStr));
  
  BinaryPath<128> p128;
  ASSERT_TRUE(p128.fromBinaryString(binaryStr));
  std::string r128 = p128.toBinaryString();
  ASSERT_EQ(binaryStr,r128);
  
  /*  Test on input "10..1111.1011" which has two consecutive
   *  separators (marked by '.'). The string is bad regardless of path 
   *  depth.
   */

  std::string badStr("10..1111.1011"); 
  BinaryPath<12> Path12;
  ASSERT_FALSE(Path12.fromBinaryString(badStr));
  
  std::string badStr2("1000.01.100..");
  BinaryPath<192> Path192;
  ASSERT_FALSE(Path192.fromBinaryString(badStr2));


  /*  An empty string has an empty path, so return true */
  
  std::string emptyStr("");
  BinaryPath<24> Path24;
  ASSERT_TRUE(Path24.fromBinaryString(emptyStr));

  /* Any string with characters other than 0 or 1 should be rejected; 
   * the functionality of fromBinaryString() requires binary input (with 
   * possible separators, e.g. ".") 
   */
  
  std::string nonBinary("This_Is_Not_A_Valid_Binary_Path_!");
  BinaryPath<32> Path32;
  ASSERT_FALSE(Path32.fromBinaryString(nonBinary));
}

// Add corner cases - bad strings, overall lengths taking up partial hex digit
TEST(BinaryPath, ToFromHexString) {
  std::string hexStr("abcd/16");
  BinaryPath<16> p1;
  ASSERT_TRUE(p1.fromHexString(hexStr));
  std::string r = p1.toHexString();
  ASSERT_EQ(hexStr,r);

  /* String characters must be in the ranges 0-9, a-f, A-F */
  std::string badHex("92zf/16");
  ASSERT_FALSE(p1.fromHexString(badHex));

  /* Empty string returns empty path */
  std::string emptyStr("");
  ASSERT_TRUE(p1.fromHexString(emptyStr));

}


TEST(BinaryPath, TrimFront) {

  std::string binaryStr("1111010111101010");

   /* Test for case when size of the path is == to desired trim amount;
   * BinaryPath should have length 0 
   */	
	BinaryPath<16> p1;
 	p1.fromBinaryString(binaryStr);
 	p1.trim_front(16);
  ASSERT_EQ(p1.size(), 0);

  /* Test for case when desired trimmed size is 1 bit smaller than a multiple
   * of a 8 (byte size_) 
   */

  BinaryPath<16> p2;
  p2.fromBinaryString(binaryStr);
  p2.trim_front(1);
  ASSERT_EQ(p2.size(), 15);

  /* Test for case when desired trimmed size is 1 larger than a multiple
   * of 8
   */
  BinaryPath<16> p3;
  p3.fromBinaryString(binaryStr);
  p3.trim_front(7);
  ASSERT_EQ(p3.size(), 9);

  /* Test for case when desired trim amount is + or - 4 bytes from
   * nearest multiple of 8 
   */
  BinaryPath<16> p4;
  p4.fromBinaryString(binaryStr);
  p4.trim_front(12);
  ASSERT_EQ(p4.size(), 4);


 	/* Test that continual calls to trim_back for the length of original 
	 * binary path will yield empty path
   */
 	BinaryPath<16> p5;
  p5.fromBinaryString(binaryStr);
  int init_size = p5.size();
  for (int i = 0; i < init_size; i++){
    p5.trim_front(1);
  }
  ASSERT_EQ(p5.size(), 0);
}

TEST(BinaryPath, TrimBack) {

  std::string binaryStr("1001110011111111"); /* init binary string 
							with length 16 */

  /* Test for case when size of the path is == to desired trim amount;
   * BinaryPath should have length 0 
   */	
  BinaryPath<16> p1;
  p1.fromBinaryString(binaryStr);
  p1.trim_back(16);
  ASSERT_EQ(p1.size(), 0);

  /* Test for case when desired trimmed size is 1 bit smaller than a multiple
   * of a 8 (byte size_) 
   */

  BinaryPath<16> p2;
  p2.fromBinaryString(binaryStr);
  p2.trim_back(1);
  ASSERT_EQ(p2.size(), 15);

  /* Test for case when desired trimmed size is 1 larger than a multiple
   * of 8
   */
  BinaryPath<16> p3;
  p3.fromBinaryString(binaryStr);
  p3.trim_back(7);
  ASSERT_EQ(p3.size(), 9);

  /* Test for case when desired trim amount is + or - 4 bytes from
   * nearest multiple of 8 
   */
  BinaryPath<16> p4;
  p4.fromBinaryString(binaryStr);
  p4.trim_back(12);
  ASSERT_EQ(p4.size(), 4);

  /* Test that continual calls to trim_back for the length of original 
	 * binary path will yield empty path
   */
 	BinaryPath<16> p5;
  p5.fromBinaryString(binaryStr);
  int init_size = p5.size();
  for (int i = 0; i < init_size; i++){
    p5.trim_back(1);
  }
  ASSERT_EQ(p5.size(), 0);
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
