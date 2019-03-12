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

#include "../RadixTree/SimplePath.h"
#include "PathEdgeTestUtils.h"
#include "RandomUtils.h"
#include "PathEdgeTests.h"

/**
 * \file test_SimplePath.cc
 */

using namespace Akamai::Mapper::RadixTree;

TEST(SimplePath_2_16,RandomOps) {
  std::string testResult = pathRandomOps<SimplePath<2,16>>(1000000);
  ASSERT_EQ(testResult,"OK");
}

TEST(SimplePath_3_16,RandomOps) {
  std::string testResult = pathRandomOps<SimplePath<3,16>>(1000000);
  ASSERT_EQ(testResult,"OK");
}

TEST(SimplePath_8_16,RandomOps) {
  std::string testResult = pathRandomOps<SimplePath<8,16>>(1000000);
  ASSERT_EQ(testResult,"OK");
}

TEST(SimplePath_256_16,RandomOps) {
  std::string testResult = pathRandomOps<SimplePath<256,16>>(1000000);
  ASSERT_EQ(testResult,"OK");
}

TEST(SimplePath_8000_16,RandomOps) {
  std::string testResult = pathRandomOps<SimplePath<8000,16>>(1000000);
  ASSERT_EQ(testResult,"OK");
}

TEST(SimplePath_70000_16,RandomOps) {
  std::string testResult = pathRandomOps<SimplePath<70000,16>>(1000000);
  ASSERT_EQ(testResult,"OK");
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
