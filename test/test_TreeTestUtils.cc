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
#include <string>
#include <limits.h>

#include "gtest/gtest.h"

#include "TreeTestUtils.h"
#include "BinaryTestPath.h"
#include "TestPath.h"
#include "PathEdgeTestUtils.h"

/**
 * \file test_TreeTestUtils.cc
 */

uint64_t getMax(std::size_t radix,std::size_t size) {
  PathNumIter pn(radix,size);
  pn.max();
  return pn.number();
}

TEST(PathNumIter, Init) {
  PathNumIter zero2(2,0);
  PathNumIter zero3(3,0);
  PathNumIter zero17(17,0);
  ASSERT_EQ(0,zero2.number());
  ASSERT_EQ(0,zero3.number());
  ASSERT_EQ(0,zero17.number());


  ASSERT_EQ(std::numeric_limits<uint8_t>::max(),getMax(2,8));
  ASSERT_EQ(std::numeric_limits<uint16_t>::max(),getMax(2,16));
  ASSERT_EQ(std::numeric_limits<uint32_t>::max(),getMax(2,32));
  ASSERT_EQ(std::numeric_limits<uint64_t>::max(),getMax(2,64));;


  ASSERT_EQ(07,getMax(8,1));
  ASSERT_EQ(0777,getMax(8,3));
  ASSERT_EQ(07777,getMax(8,4));

  ASSERT_EQ(0xF,getMax(16,1));
  ASSERT_EQ(0xFFF,getMax(16,3));
  ASSERT_EQ(0xFFFF,getMax(16,4));

  PathNumIter p7(8,{1,2,3,4,5,6,7});
  ASSERT_EQ(01234567,p7.number());

  ASSERT_EQ(9,getMax(10,1));
  ASSERT_EQ(99,getMax(10,2));
  ASSERT_EQ(999999,getMax(10,6));
}

uint64_t testIncrement(std::size_t radix,std::size_t size,uint64_t& num) {
  PathNumIter pathNum(radix,size);
  num = 0;
  while (pathNum.increment()) { ++num; }
  return pathNum.number();
}

TEST(PathNumIter, Increment) {
  for (std::size_t rad = 2;rad <= 19;++rad) {
    uint64_t n = 0;
    uint64_t r = testIncrement(rad,4,n);
    ASSERT_EQ(r,n);
  }
}

using BinaryPathValue8 = TestPathValue<BinaryTestPath8,uint64_t>;
using BinaryPathValue32 = TestPathValue<BinaryTestPath32,uint64_t>;
using TerenaryPathValue6 = TestPathValue<TestPath<3,6>,uint64_t>;
using TerenaryPathValue13 = TestPathValue<TestPath<3,13>,uint64_t>;

TEST(PathNumIter,AllPathValues) {
  uint64_t curValue = 0;
  std::vector<BinaryPathValue8> all2_4 = allPathValuesThroughLength<BinaryPathValue8>(4,curValue);
  ASSERT_EQ(31,all2_4.size());

  curValue = 0;
  std::vector<BinaryPathValue8> all2_8 = allPathValuesThroughLength<BinaryPathValue8>(8,curValue);
  ASSERT_EQ(511,all2_8.size());

  curValue = 0;
  std::vector<BinaryPathValue32> all2_8_32 = allPathValuesThroughLength<BinaryPathValue32>(8,curValue);
  ASSERT_EQ(511,all2_8_32.size());

  curValue = 0;
  std::vector<BinaryPathValue32> all2_16_32 = allPathValuesThroughLength<BinaryPathValue32>(16,curValue);
  ASSERT_EQ(131071,all2_16_32.size());

  curValue = 0;
  std::vector<BinaryPathValue32> all2_19_32 = allPathValuesThroughLength<BinaryPathValue32>(19,curValue);
  ASSERT_EQ(1048575,all2_19_32.size());

  curValue = 0;
  std::vector<TerenaryPathValue6> all3_6 = allPathValuesThroughLength<TerenaryPathValue6>(6,curValue);
  ASSERT_EQ(1093,all3_6.size());

  curValue = 0;
  std::vector<TerenaryPathValue13> all3_6_13 = allPathValuesThroughLength<TerenaryPathValue13>(6,curValue);
  ASSERT_EQ(1093,all3_6_13.size());

  curValue = 0;
  std::vector<TerenaryPathValue13> all3_13 = allPathValuesThroughLength<TerenaryPathValue13>(13,curValue);
  ASSERT_EQ(2391484,all3_13.size());
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
