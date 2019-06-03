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

#include <tuple>

#include "gtest/gtest.h"

#include "BinaryTreeTestUtils.h"
#include "BinaryTestPath.h"
#include "TestPath.h"
#include "CursorTraversal.h"
#include "CompoundCursor.h"
#include "TreeTestUtils.h"

using namespace Akamai::Mapper::RadixTree;

TEST(CompoundCursor, SmokeTest) {
  Tree6_3 tree1;
  Tree6_3 tree2;

  std::vector<TestPath<2,6>> paths{{0,0},{0,1},{1,0},{1,1}};
  
  auto compoundCursor = make_compound_cursor(tree1.cursor(),tree2.cursor());
  uint32_t curVal = 0;
  for (const auto& path : paths) {
    cursorGoto(compoundCursor,path);
    compoundCursor.addNode();
    auto bothValues = compoundCursor.nodeValue();
    std::get<0>(bothValues).set(curVal++);
    std::get<1>(bothValues).set(curVal++);
  }

  curVal = 0;
  cursorGotoRoot(compoundCursor);
  for (const auto& path : paths) {
    cursorGoto(compoundCursor,path);
    ASSERT_TRUE(compoundCursor.allAtValue());
    ASSERT_EQ(*compoundCursor.nodeValue<0>().getPtrRO(),curVal++);
    ASSERT_EQ(*compoundCursor.nodeValue<1>().getPtrRO(),curVal++);
  }

  auto compoundCursorRO = make_compound_cursor_ro(tree1.cursorRO(),tree2.cursorRO());
  curVal = 0;
  for (const auto& path : paths) {
    cursorGoto(compoundCursorRO,path);
    ASSERT_TRUE(compoundCursorRO.allAtValue());
    ASSERT_EQ(*compoundCursorRO.nodeValue<0>().getPtrRO(),curVal++);
    ASSERT_EQ(*compoundCursorRO.nodeValue<1>().getPtrRO(),curVal++);
  }
}

template <typename CursorType,typename PathValueType>
void addValuesAtPaths(CursorType&& c,const std::vector<PathValueType>& paths) {
  uint32_t v = 0;
  for (const auto& path : paths) {
    cursorGoto(c,path);
    c.addNode();
    c.nodeValue().set(v++);
  }
}

template <typename CompoundType>
std::size_t
countCompoundSpots(CompoundType&& cc) {
  std::size_t countSoFar = 0;
  if (cc.atValue()) { ++countSoFar; }
  for (std::size_t child = 0;child < std::decay<CompoundType>::type::Radix;++child) {
    if (cc.canGoChildNode(child)) {
      cc.goChild(child);
      countSoFar += countCompoundSpots(std::forward<CompoundType>(cc));
      cc.goParent();
    }
  }
  return countSoFar;
}

TEST(CompoundCursor, FollowTest) {
  Tree6_3 leader1;
  Tree6_3 leader2;
  Tree6_3 follower;

  std::vector<TestPath<2,6>> leader1Paths{{1,0,0},{1,0,1},{1,1,0},{1,1,1}};
  std::vector<TestPath<2,6>> leader2Paths{{0,0,0},{0,0,1},{0,1,0},{0,1,1}};
  std::vector<TestPath<2,6>> followerPaths{{0,0},{0,1},{1,0},{1,1},
                                           {0,0,0,0},{0,1,0,1},{1,0,1,0},{1,1,1,1}};
  addValuesAtPaths(leader1.cursor(),leader1Paths);
  addValuesAtPaths(leader2.cursor(),leader2Paths);
  addValuesAtPaths(follower.cursor(),followerPaths);

  // If we traverse all of the spots in leader1,leader2, and follower then we
  // will hit a total of (4 + 4 + 8) spots.
  constexpr std::size_t allSpotCount = (4 + 4 + 8);
  // If we follow the two leaders then there are only (4 + 4) spots that we should hit.
  constexpr std::size_t followSpotCount = (4 + 4);
  // If we follow over the two leaders then we hit the leader spots (4 + 4) as well as
  // the follower spots that hapen to lie along the paths that get us down to either
  // leader (just the 4 paths of length 2 in the follower).
  constexpr std::size_t followOverSpotCount = (4 + 4 + 4);

  auto compoundCursor = make_compound_cursor_ro(follower.cursor(),leader1.cursor(),leader2.cursor());
  std::size_t compoundSpotCount = countCompoundSpots(compoundCursor);
  ASSERT_EQ(allSpotCount,compoundSpotCount);

  auto compoundFollowCursor = make_compound_follow_cursor_ro(follower.cursor(),leader1.cursor(),leader2.cursor());
  std::size_t compoundFollowSpotCount = countCompoundSpots(compoundFollowCursor);
  ASSERT_EQ(followSpotCount,compoundFollowSpotCount);

  auto compoundFollowOverCursor = make_compound_follow_over_cursor_ro(follower.cursor(),leader1.cursor(),leader2.cursor());
  std::size_t compoundFollowOverSpotCount = countCompoundSpots(compoundFollowOverCursor);
  ASSERT_EQ(followOverSpotCount,compoundFollowOverSpotCount);

  // Check some specific covering node values
  auto compoundCheckCovering = make_compound_cursor_ro(follower.cursorRO(),leader1.walkCursorRO(),leader2.lookupCursorRO());
  cursorGoto(compoundCheckCovering,TestPath<2,6>{0,0,0,0});
  auto allCoveringDepths = compoundCheckCovering.coveringNodeValueDepth();
  auto allCoveringNodeValues = compoundCheckCovering.coveringNodeValueRO();
  ASSERT_EQ(std::get<0>(allCoveringDepths),compoundCheckCovering.coveringNodeValueDepth<0>());
  ASSERT_EQ(std::get<0>(allCoveringDepths),4);
  ASSERT_EQ(*(compoundCheckCovering.coveringNodeValueRO<0>().getPtrRO()),4);
  ASSERT_EQ(*(std::get<0>(allCoveringNodeValues).getPtrRO()),4);

  ASSERT_EQ(std::get<1>(allCoveringDepths),compoundCheckCovering.coveringNodeValueDepth<1>());
  ASSERT_EQ(std::get<1>(allCoveringDepths),0);
  ASSERT_EQ(compoundCheckCovering.coveringNodeValueRO<1>().getPtrRO(),nullptr);
  ASSERT_EQ(std::get<1>(allCoveringNodeValues).getPtrRO(),nullptr);

  ASSERT_EQ(std::get<2>(allCoveringDepths),compoundCheckCovering.coveringNodeValueDepth<2>());
  ASSERT_EQ(std::get<2>(allCoveringDepths),3);
  ASSERT_EQ(*(compoundCheckCovering.coveringNodeValueRO<2>().getPtrRO()),0);
  ASSERT_EQ(*(std::get<2>(allCoveringNodeValues).getPtrRO()),0);
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

