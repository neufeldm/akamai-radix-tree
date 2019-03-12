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

#include "gtest/gtest.h"

#include "BinaryTreeTestUtils.h"
#include "BinaryTestPath.h"
#include "SimplePath.h"
#include "CursorTraversal.h"
#include "CompoundCursor.h"
#include "TreeTestUtils.h"

using namespace Akamai::Mapper::RadixTree;


TEST(RadixTreeFunctions, CursorTraversalFollow) {

  Tree6_3 tree1;
  Tree6_3 tree2;
  Tree6_3 tree3;

  auto leader1 = tree1.cursor();
  auto follower = tree2.cursor();
  auto leader2 = tree3.cursor();

  // Fully populate two, identical binary trees with path depth 2
  SimplePath<2,6> pv1{0,0}, pv2{0,1}, pv3{1,0}, pv4{1,1};
  std::vector<SimplePath<2,6>>tree_paths {pv1, pv2, pv3, pv4};
  for (int i = 0; i < tree_paths.size(); i++) {
    while (leader1.canGoParent()) { leader1.goParent(); }
    while (leader2.canGoParent()) { leader2.goParent(); }
    while (follower.canGoParent()) { follower.goParent(); }
    for (int j = 0; j < pv1.size(); j++) {
      leader1.goChild(tree_paths[i].at(j));
      leader2.goChild(tree_paths[i].at(j));
      follower.goChild(tree_paths[i].at(j));
      leader1.addNode();
      leader2.addNode();
      follower.addNode();
      leader1.nodeValue().set(i);
      leader2.nodeValue().set(i);
      follower.nodeValue().set(i);
    }
  }

  // Return both cursors to the root
  while (leader1.canGoParent() || leader2.canGoParent() || follower.canGoParent()) {
    leader1.goParent();
    leader2.goParent();
    follower.goParent();
  }


  // Check that follower can follow one leader, and then two leaders in pre-order
  preOrderFollow<false, 2>([](const decltype(follower)& f, const decltype(leader1)& L1) {
      ASSERT_EQ(*(L1.nodeValueRO().getPtrRO()), *(f.nodeValueRO().getPtrRO()));
    }, follower, leader1);


  while (leader1.canGoParent() || leader2.canGoParent() || follower.canGoParent()) {
    leader1.goParent();
    leader2.goParent();
    follower.goParent();
  }

  preOrderFollow<false,2>([](const decltype(follower)& f, const decltype(leader1)& L1,
           const decltype(leader2)& L2) {
          ASSERT_EQ(*(L2.nodeValueRO().getPtrRO()), *(f.nodeValueRO().getPtrRO()));
          ASSERT_EQ(*(L1.nodeValueRO().getPtrRO()), *(f.nodeValueRO().getPtrRO()));
        }, follower, leader1, leader2);

  // Check that follower can follow one leader, and then two leaders in post-order
  postOrderFollow<false, 2>([](const decltype(follower)& f, const decltype(leader1)& L1) {
      ASSERT_EQ(*(L1.nodeValueRO().getPtrRO()), *(f.nodeValueRO().getPtrRO()));
    }, follower, leader1);


  while (leader1.canGoParent() || leader2.canGoParent() || follower.canGoParent()) {
    leader1.goParent();
    leader2.goParent();
    follower.goParent();
  }

  postOrderFollow<false,2>([](const decltype(follower)& f, const decltype(leader1)& L1,
            const decltype(leader2)& L2) {
           ASSERT_EQ(*(L2.nodeValueRO().getPtrRO()), *(f.nodeValueRO().getPtrRO()));
           ASSERT_EQ(*(L1.nodeValueRO().getPtrRO()), *(f.nodeValueRO().getPtrRO()));
         }, follower, leader1, leader2);

  // Check that follower can follow one leader, and then two leaders in in-order
  inOrderFollow<false, 2>([](const decltype(follower)& f, const decltype(leader1)& L1) {
      ASSERT_EQ(*(L1.nodeValueRO().getPtrRO()), *(f.nodeValueRO().getPtrRO()));
    }, follower, leader1);


  while (leader1.canGoParent() || leader2.canGoParent() || follower.canGoParent()) {
    leader1.goParent();
    leader2.goParent();
    follower.goParent();
  }

  inOrderFollow<false,2>([](const decltype(follower)& f, const decltype(leader1)& L1,
          const decltype(leader2)& L2) {
         ASSERT_EQ(*(L2.nodeValueRO().getPtrRO()), *(f.nodeValueRO().getPtrRO()));
         ASSERT_EQ(*(L1.nodeValueRO().getPtrRO()), *(f.nodeValueRO().getPtrRO()));
                         }, follower, leader1, leader2);

}

TEST(RadixTreeFunctions, CursorTraversalFollowDiff) {

  Tree6_3 tree1;
  Tree6_3 tree2;

  auto leader1 = tree1.cursor();
  auto follower = tree2.cursor();

  SimplePath<2,6> pv1{0,0,0}, pv2{0,0,1}, pv3{0,1,0}, pv4{0,1,1},
                  pv5{1,1,1}, pv6{1,1,0}, pv7{1,0,0}, pv8{1,0,1};

  SimplePath<2,6> p1{0,0,0}, p2{0,1,1}, p3{1,0,1};


  std::vector<SimplePath<2,6>> tree1_paths {pv1, pv2, pv3, pv4, pv5, pv6, pv7, pv8};
  std::vector<SimplePath<2,6>> tree2_paths {p1, p2, p3};

  // Make fully populated binary tree of depth 3
  for (size_t i = 0; i < tree1_paths.size(); i++) {
    while (leader1.canGoParent()) { leader1.goParent(); }
    for (size_t j = 0; j < pv1.size(); j++) {
      leader1.goChild(tree1_paths[i].at(j));
    }
    leader1.addNode();
    leader1.nodeValue().set(i);
  }

  // Populate sparse tree, a subset of the full depth 3 tree
  // with different values at each node
  for (size_t i = 0; i < tree2_paths.size(); i++) {
    while(follower.canGoParent()) { follower.goParent(); }
    for (size_t j = 0; j < p1.size(); j++) {
      if (follower.canGoChild(tree2_paths[i].at(j))) {
        follower.goChild(tree2_paths[i].at(j));
      }
      follower.addNode();
      follower.nodeValue().set(i*2);
    }
  }

  while(leader1.canGoParent() || follower.canGoParent()) {
    leader1.goParent();
    follower.goParent();
  }

  // Check that leader1 and follower show different values at same path locations
  postOrderFollow<false,2>([](const decltype(follower)& f, const decltype(leader1)& L1) {
      if (*(L1.nodeValueRO().getPtrRO()) == 3) {
        ASSERT_EQ(*(f.nodeValueRO().getPtrRO()), 2);}}, follower, leader1);

}

TEST(RadixTreeFunctions, CursorTraversalFollowOver) {

  Tree6_3 tree1;
  Tree6_3 tree2;

  auto leader1 = tree1.cursor();
  auto follower = tree2.cursor();
  int callCount = 0;

  // Fully populate the two binary trees with path depth 2
  SimplePath<2,6> p1{0,0}, p2{0,1}, p3{1,0}, p4{1,1};

  std::vector<SimplePath<2,6>> tree_paths {p1,p2,p3,p4};

  // Set values at leaves of both trees
  for (size_t i = 0; i < tree_paths.size(); i++) {
    while (leader1.canGoParent()) { leader1.goParent(); }
    while (follower.canGoParent()) { follower.goParent(); }
    for (size_t j = 0; j < p1.size(); j++) {
      leader1.goChild(tree_paths[i].at(j));
      follower.goChild(tree_paths[i].at(j));
    }
    leader1.addNode();
    follower.addNode();
    follower.nodeValue().set(i);
  }

  // Add extra value to tree2; follower triggers callback when at value
  while(follower.canGoParent()) { follower.goParent(); }
  follower.goChild(0);
  ASSERT_TRUE(follower.atNode());
  follower.nodeValue().set(700);

  // Return both cursors to the root
  while (leader1.canGoParent() || follower.canGoParent()) {
    leader1.goParent();
    follower.goParent();
  }

  preOrderFollowOver<false,2>([&callCount](const decltype(follower)& f, const decltype(leader1)& L1) {
      callCount++;}, follower, leader1);
  ASSERT_EQ(callCount, 5);

  // Reset callCount for postOrder check
  callCount = 0;
  postOrderFollowOver<false,2>(
    [&callCount](const decltype(follower)& f, const decltype(leader1)& L1){callCount++;},
    follower, leader1);
  ASSERT_EQ(callCount, 5);

  callCount = 0;
  inOrderFollowOver<false,2>(
      [&callCount](const decltype(follower)& f, const decltype(leader1)& L1){callCount++;},
      follower, leader1);
  ASSERT_EQ(callCount, 5);

}


#if 0
TEST(RadixTreeFunctions, Depth16Traversal) {

  using PathVal32 = IntPathValue<IntPath32, uint32_t>;
  std::vector<PathVal32> vals;
  uint32_t v{0};
  addAllThroughDepth<PathVal32>(16, v, std::back_inserter(vals));
  uint32_t expectedEndVal = countAtAllThroughDepth(16);

  std::vector<uint32_t> shallowToDeep = makeIdentityMap<uint32_t>(expectedEndVal);
  std::vector<uint32_t> preOrder(shallowToDeep);

  std::sort(preOrder.begin(),preOrder.end(),[&vals](size_t a, size_t b) -> bool { return IntPathSortPreOrder<uint32_t>{}(vals[a],vals[b]); });

  Tree16_3 tree1;
  Tree16_3 tree2;
  Tree16_3 tree3;

  addToTreeMove(tree1, vals, preOrder);
  addToTreeMove(tree2, vals, preOrder);
  addToTreeMove(tree3, vals, preOrder);

  auto leader1 = tree1.cursor();
  auto follower = tree2.cursor();
  auto leader2 = tree3.cursor();

  // Check that follower can follow one leader, and then two leaders in pre-order
  preOrderFollow<false, 2>([](const decltype(follower)& f, const decltype(leader1)& L1) {
      ASSERT_EQ(*(L1.nodeValueRO().getPtrRO()), *(f.nodeValueRO().getPtrRO()));
    }, follower, leader1);


  while (leader1.canGoParent() || leader2.canGoParent() || follower.canGoParent()) {
    leader1.goParent();
    leader2.goParent();
    follower.goParent();
  }

  preOrderFollow<false,2>([](const decltype(follower)& f, const decltype(leader1)& L1,
           const decltype(leader2)& L2) {
          ASSERT_EQ(*(L2.nodeValueRO().getPtrRO()), *(f.nodeValueRO().getPtrRO()));
          ASSERT_EQ(*(L1.nodeValueRO().getPtrRO()), *(f.nodeValueRO().getPtrRO()));
        }, follower, leader1, leader2);

  // Check that follower can follow one leader, and then two leaders in post-order
  postOrderFollow<false, 2>([](const decltype(follower)& f, const decltype(leader1)& L1) {
      ASSERT_EQ(*(L1.nodeValueRO().getPtrRO()), *(f.nodeValueRO().getPtrRO()));
    }, follower, leader1);


  while (leader1.canGoParent() || leader2.canGoParent() || follower.canGoParent()) {
    leader1.goParent();
    leader2.goParent();
    follower.goParent();
  }

  postOrderFollow<false,2>([](const decltype(follower)& f, const decltype(leader1)& L1,
            const decltype(leader2)& L2) {
           ASSERT_EQ(*(L2.nodeValueRO().getPtrRO()), *(f.nodeValueRO().getPtrRO()));
           ASSERT_EQ(*(L1.nodeValueRO().getPtrRO()), *(f.nodeValueRO().getPtrRO()));
         }, follower, leader1, leader2);

  // Check that follower can follow one leader, and then two leaders in in-order
  inOrderFollow<false, 2>([](const decltype(follower)& f, const decltype(leader1)& L1) {
      ASSERT_EQ(*(L1.nodeValueRO().getPtrRO()), *(f.nodeValueRO().getPtrRO()));
    }, follower, leader1);


  while (leader1.canGoParent() || leader2.canGoParent() || follower.canGoParent()) {
    leader1.goParent();
    leader2.goParent();
    follower.goParent();
  }

  inOrderFollow<false,2>([](const decltype(follower)& f, const decltype(leader1)& L1,
          const decltype(leader2)& L2) {
         ASSERT_EQ(*(L2.nodeValueRO().getPtrRO()), *(f.nodeValueRO().getPtrRO()));
         ASSERT_EQ(*(L1.nodeValueRO().getPtrRO()), *(f.nodeValueRO().getPtrRO()));
                         }, follower, leader1, leader2);
}
#endif

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

