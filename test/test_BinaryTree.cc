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

#include "BinaryTreeTestUtils.h"
#include "RandomUtils.h"
#include "BinaryTestPath.h"
#include "PathEdgeTestUtils.h"
#include "TreeTestUtils.h"
#include "PathSort.h"
#include "TreeTests.h"

#include "BinaryPath.h"
#include "BinaryWordEdge.h"

using namespace Akamai::Mapper::RadixTree;

// Template for initializing a binary tree with any desired max edge length
template <int ext_length> using Tree16 = BinaryNodeRadixTree<uint32_t, Path16, Akamai::Mapper::RadixTree::BinaryWordEdge<uint32_t, 8, ext_length>>;

/*
 * Traverses cursor in pre-order; callback executed when cursor is at a node.
 */
template <typename PathType,typename CursorType,typename CallbackType>
void preOrderTest(PathType&& p,CursorType&& c,CallbackType&& cb) { 
  static constexpr std::size_t radix = std::remove_reference<CursorType>::type::Radix;
  if (c.atNode()) { cb(std::forward<PathType>(p),std::forward<CursorType>(c)); }
  for (std::size_t child = 0; child < radix; ++child) {
    if (c.canGoChildNode(child)) {
      p.push_back(child);
      c.goChild(child);
      preOrderTest(std::forward<PathType>(p),std::forward<CursorType>(c),std::forward<CallbackType>(cb));
      c.goParent();
      p.pop_back();
    }
  }
}

/*
 * Post-order traversal that deletes each node, and ensures that each node
 * was deleted.
 */
template <typename CursorType>
void postOrderDelete(CursorType&& c) {
  static constexpr std::size_t radix = std::remove_reference<CursorType>::type::Radix;
  for (std::size_t i = 0; i < radix; i++){
    if (c.canGoChildNode(i)) {
      c.goChild(i);
      postOrderDelete(std::forward<CursorType>(c));
      c.goParent();
  	}
  }
  if (c.atNode() && (c.getPath().size() > 0)) {
    c.nodeValue().clear();
    ASSERT_TRUE(c.removeNode());
    ASSERT_FALSE(c.atNode()); 
  }
}

/*
 *  Simple "smoke test"
 */
TEST(BinaryTree, SmokeTest) {
  using PathVal = TestPathValue<BinaryTestPath8,uint32_t>;
  Tree16_3 testTree;
  auto c = testTree.cursor();
  ASSERT_TRUE(c.atNode());
  auto nv = c.nodeValue();
  ASSERT_TRUE(nv.atNode());
  ASSERT_FALSE(nv.atValue());
  nv.set(37);
  ASSERT_TRUE(nv.atValue());
  ASSERT_EQ(*(nv.getPtrRO()),37U);

  PathVal pv1({1,1,1,1,1,1,0},12348);
  pv1.setCursor(c);
  nv = c.nodeValue();
  ASSERT_FALSE(nv.atNode());
  ASSERT_FALSE(nv.atValue());
  c.addNode();
  nv = c.nodeValue();
  ASSERT_TRUE(nv.atNode());
  ASSERT_FALSE(nv.atValue());
  nv.set(pv1.value);
  ASSERT_TRUE(nv.atValue());
  ASSERT_EQ(*(nv.getPtrRO()),pv1.value);

  // Now go over what we just did
  auto c2 = testTree.cursorRO();
  ASSERT_TRUE(c2.nodeValue().atValue());
  ASSERT_EQ(*(c2.nodeValue().getPtrRO()),37U);

  pv1.setCursor(c2);
  auto nv2 = c2.nodeValue();
  ASSERT_TRUE(nv2.atValue());
  ASSERT_EQ(*(nv2.getPtrRO()),pv1.value);

  // Should have a node here because the edge bits run out
  BinaryTestPath8 p1ext(pv1.shiftRight(3));
  p1ext.setCursor(c2);
  ASSERT_TRUE(c2.nodeValue().atNode());
  ASSERT_FALSE(c2.nodeValue().atValue());

  auto c3 = testTree.cursorRO();
  c3.goChild(1);
  c3.goChild(1);
  c3.goChild(1);
  c3.goChild(1);
  // This should be where the edge bits ran out
  ASSERT_TRUE(c3.nodeValue().atNode());
  ASSERT_FALSE(c3.nodeValue().atValue());
  c3.goChild(1);
  c3.goChild(1);
  c3.goChild(0);
  // This should be where the value in pv1 is.
  ASSERT_TRUE(c3.nodeValue().atNode());
  ASSERT_EQ(*(c3.nodeValue().getPtrRO()),pv1.value);
}

// Depth 16 tree requires 0 - 16 inclusive depth so we can't use an IntPath16
using PathVal32 = TestPathValue<BinaryTestPath32,uint32_t>;

/*
 * Fill the entirety of a depth 16 binary tree with paths and values, ensuring that
 * pre-order, in-order, and post-order traversal yields expected results. 
 */
TEST(BinaryTree, FillTest) {
  // Generate enough path/value pairs to fill a depth 16 tree.
  // The values are uint32_t, assigned in increasing order
  // from left/right at each layer of the tree, starting at the
  // root and going down one layer at a time. Same scheme
  // as used in IntPathSortTest only we've mechanized it here.
  std::vector<PathVal32> vals;
  uint32_t v{0};
  addAllThroughDepth<PathVal32>(16,v,std::back_inserter(vals));
  uint32_t expectedEndValue = countAtAllThroughDepth(16);
  ASSERT_EQ(v,expectedEndValue);

  // Now fill some trees, use different fill orders as well as how
  // we move between each fill point in the tree.

  // In order of generation - "layer cake" ordering shallow to deep, left to right
  std::vector<uint32_t> shallowToDeepLR = makeIdentityMap<uint32_t>(expectedEndValue);
  std::vector<uint32_t> deepToShallowRL(shallowToDeepLR);
  std::reverse(deepToShallowRL.begin(),deepToShallowRL.end());


  Tree16_3 testTree3_shallowToDeepLR_move;
  addToTreeMove(testTree3_shallowToDeepLR_move,vals,shallowToDeepLR);
  ASSERT_TRUE(checkTreeMove(testTree3_shallowToDeepLR_move,vals,shallowToDeepLR));
  ASSERT_TRUE(checkTreeMove(testTree3_shallowToDeepLR_move,vals,deepToShallowRL));

  Tree16_3 testTree3_deepToShallowRL_move;
  addToTreeMove(testTree3_deepToShallowRL_move,vals,deepToShallowRL);
  ASSERT_TRUE(checkTreeMove(testTree3_deepToShallowRL_move,vals,deepToShallowRL));
  ASSERT_TRUE(checkTreeMove(testTree3_deepToShallowRL_move,vals,shallowToDeepLR));

  // Pre-order left-right
  std::vector<uint32_t> preOrderLR(shallowToDeepLR);
  std::sort(preOrderLR.begin(),preOrderLR.end(),[&vals](std::size_t a,std::size_t b) -> bool { return PathSortPreOrder<BinaryTestPath32>{}(vals[a],vals[b]); });

  // Post-order left-right
  std::vector<uint32_t> postOrderLR(shallowToDeepLR);
  std::sort(postOrderLR.begin(),postOrderLR.end(),[&vals](std::size_t a,std::size_t b) -> bool { return PathSortPostOrder<BinaryTestPath32>{}(vals[a],vals[b]); });

  // In-order left-right
  std::vector<uint32_t> inOrderLR(shallowToDeepLR);
  std::sort(inOrderLR.begin(),inOrderLR.end(),[&vals](std::size_t a,std::size_t b) -> bool { return PathSortInOrder<BinaryTestPath32>{}(vals[a],vals[b]); });

  Tree16_3 testTree3_preLR_move;
  addToTreeMove(testTree3_preLR_move,vals,preOrderLR);
  ASSERT_TRUE(checkTreeMove(testTree3_preLR_move,vals,preOrderLR));
  ASSERT_TRUE(checkTreeMove(testTree3_preLR_move,vals,postOrderLR));
  ASSERT_TRUE(checkTreeMove(testTree3_preLR_move,vals,inOrderLR));
  ASSERT_TRUE(checkTreeSet(testTree3_preLR_move,vals,preOrderLR));
  ASSERT_TRUE(checkTreeSet(testTree3_preLR_move,vals,postOrderLR));
  ASSERT_TRUE(checkTreeSet(testTree3_preLR_move,vals,inOrderLR));  

  Tree16_3 testTree3_inLR_move;
  addToTreeMove(testTree3_inLR_move,vals,inOrderLR);
  ASSERT_TRUE(checkTreeMove(testTree3_inLR_move,vals,inOrderLR));
  ASSERT_TRUE(checkTreeMove(testTree3_inLR_move,vals,preOrderLR));
  ASSERT_TRUE(checkTreeMove(testTree3_inLR_move,vals,postOrderLR));
  ASSERT_TRUE(checkTreeSet(testTree3_inLR_move,vals,inOrderLR));
  ASSERT_TRUE(checkTreeSet(testTree3_inLR_move,vals,preOrderLR));
  ASSERT_TRUE(checkTreeSet(testTree3_inLR_move,vals,postOrderLR));

  Tree16_3 testTree3_preLR_set;
  addToTreeSet(testTree3_preLR_set,vals,preOrderLR);
  ASSERT_TRUE(checkTreeMove(testTree3_preLR_set,vals,preOrderLR));
  ASSERT_TRUE(checkTreeMove(testTree3_preLR_set,vals,postOrderLR));
  ASSERT_TRUE(checkTreeMove(testTree3_preLR_set,vals,inOrderLR));
  ASSERT_TRUE(checkTreeSet(testTree3_preLR_set,vals,preOrderLR));
  ASSERT_TRUE(checkTreeSet(testTree3_preLR_set,vals,postOrderLR));
  ASSERT_TRUE(checkTreeSet(testTree3_preLR_set,vals,inOrderLR));

  Tree16_3 testTree3_postLR_set;
  addToTreeSet(testTree3_postLR_set,vals,postOrderLR);
  ASSERT_TRUE(checkTreeMove(testTree3_postLR_set,vals,postOrderLR));
  ASSERT_TRUE(checkTreeMove(testTree3_postLR_set,vals,preOrderLR));
  ASSERT_TRUE(checkTreeMove(testTree3_postLR_set,vals,inOrderLR));
  ASSERT_TRUE(checkTreeSet(testTree3_postLR_set,vals,postOrderLR));
  ASSERT_TRUE(checkTreeSet(testTree3_postLR_set,vals,preOrderLR));
  ASSERT_TRUE(checkTreeSet(testTree3_postLR_set,vals,inOrderLR));


  Tree16_3 testTree3_postLR_move;
  addToTreeMove(testTree3_postLR_move,vals,postOrderLR);
  ASSERT_TRUE(checkTreeMove(testTree3_postLR_move,vals,postOrderLR));
  ASSERT_TRUE(checkTreeMove(testTree3_postLR_move,vals,preOrderLR));
  ASSERT_TRUE(checkTreeMove(testTree3_postLR_move,vals,inOrderLR));
  ASSERT_TRUE(checkTreeSet(testTree3_postLR_move,vals,postOrderLR));
  ASSERT_TRUE(checkTreeSet(testTree3_postLR_move,vals,preOrderLR));
  ASSERT_TRUE(checkTreeSet(testTree3_postLR_move,vals,inOrderLR));

  Tree16_3 testTree3_inLR_set;
  addToTreeSet(testTree3_inLR_set,vals,inOrderLR);
  ASSERT_TRUE(checkTreeMove(testTree3_inLR_set,vals,inOrderLR));
  ASSERT_TRUE(checkTreeMove(testTree3_inLR_set,vals,preOrderLR));
  ASSERT_TRUE(checkTreeMove(testTree3_inLR_set,vals,postOrderLR));
  ASSERT_TRUE(checkTreeSet(testTree3_inLR_set,vals,inOrderLR));
  ASSERT_TRUE(checkTreeSet(testTree3_inLR_set,vals,preOrderLR));
  ASSERT_TRUE(checkTreeSet(testTree3_inLR_set,vals,postOrderLR));

  // Pre-order right-left
  std::vector<uint32_t> preOrderRL(shallowToDeepLR);
  std::sort(preOrderRL.begin(),preOrderRL.end(),[&vals](std::size_t a,std::size_t b) -> bool { return PathSortPreOrder<BinaryTestPath32,true>{}(vals[a],vals[b]); });

  // Post-order right-left
  std::vector<uint32_t> postOrderRL(shallowToDeepLR);
  std::sort(postOrderRL.begin(),postOrderRL.end(),[&vals](std::size_t a,std::size_t b) -> bool { return PathSortPostOrder<BinaryTestPath32,true>{}(vals[a],vals[b]); });

  // In-order right-left
  std::vector<uint32_t> inOrderRL(shallowToDeepLR);
  std::sort(inOrderRL.begin(),inOrderRL.end(),[&vals](std::size_t a,std::size_t b) -> bool { return PathSortInOrder<BinaryTestPath32,true>{}(vals[a],vals[b]); });

  Tree16_3 testTree3_preRL_move;
  addToTreeMove(testTree3_preRL_move,vals,preOrderRL);
  ASSERT_TRUE(checkTreeMove(testTree3_preRL_move,vals,preOrderRL));
  ASSERT_TRUE(checkTreeMove(testTree3_preRL_move,vals,postOrderRL));
  ASSERT_TRUE(checkTreeMove(testTree3_preRL_move,vals,inOrderRL));
  ASSERT_TRUE(checkTreeSet(testTree3_preRL_move,vals,preOrderRL));
  ASSERT_TRUE(checkTreeSet(testTree3_preRL_move,vals,postOrderRL));
  ASSERT_TRUE(checkTreeSet(testTree3_preRL_move,vals,inOrderRL));

  Tree16_3 testTree3_postRL_move;
  addToTreeMove(testTree3_postRL_move,vals,postOrderRL);
  ASSERT_TRUE(checkTreeMove(testTree3_postRL_move,vals,postOrderRL));
  ASSERT_TRUE(checkTreeMove(testTree3_postRL_move,vals,preOrderRL));
  ASSERT_TRUE(checkTreeMove(testTree3_postRL_move,vals,inOrderRL));
  ASSERT_TRUE(checkTreeSet(testTree3_postRL_move,vals,postOrderRL));
  ASSERT_TRUE(checkTreeSet(testTree3_postRL_move,vals,preOrderRL));
  ASSERT_TRUE(checkTreeSet(testTree3_postRL_move,vals,inOrderRL));

  Tree16_3 testTree3_inRL_move;
  addToTreeMove(testTree3_inRL_move,vals,inOrderRL);
  ASSERT_TRUE(checkTreeMove(testTree3_inRL_move,vals,inOrderRL));
  ASSERT_TRUE(checkTreeMove(testTree3_inRL_move,vals,preOrderRL));
  ASSERT_TRUE(checkTreeMove(testTree3_inRL_move,vals,postOrderRL));
  ASSERT_TRUE(checkTreeSet(testTree3_inRL_move,vals,inOrderRL));
  ASSERT_TRUE(checkTreeSet(testTree3_inRL_move,vals,preOrderRL));
  ASSERT_TRUE(checkTreeSet(testTree3_inRL_move,vals,postOrderRL));


  Tree16_3 testTree3_preRL_set;
  addToTreeSet(testTree3_preRL_set,vals,preOrderRL);
  ASSERT_TRUE(checkTreeMove(testTree3_preRL_set,vals,preOrderRL));
  ASSERT_TRUE(checkTreeMove(testTree3_preRL_set,vals,postOrderRL));
  ASSERT_TRUE(checkTreeMove(testTree3_preRL_set,vals,inOrderRL));
  ASSERT_TRUE(checkTreeSet(testTree3_preRL_set,vals,preOrderRL));
  ASSERT_TRUE(checkTreeSet(testTree3_preRL_set,vals,postOrderRL));
  ASSERT_TRUE(checkTreeSet(testTree3_preRL_set,vals,inOrderRL));

  Tree16_3 testTree3_postRL_set;
  addToTreeSet(testTree3_postRL_set,vals,postOrderRL);
  ASSERT_TRUE(checkTreeMove(testTree3_postRL_set,vals,postOrderRL));
  ASSERT_TRUE(checkTreeMove(testTree3_postRL_set,vals,preOrderRL));
  ASSERT_TRUE(checkTreeMove(testTree3_postRL_set,vals,inOrderRL));
  ASSERT_TRUE(checkTreeSet(testTree3_postRL_set,vals,postOrderRL));
  ASSERT_TRUE(checkTreeSet(testTree3_postRL_set,vals,preOrderRL));
  ASSERT_TRUE(checkTreeSet(testTree3_postRL_set,vals,inOrderRL));

  Tree16_3 testTree3_inRL_set;
  addToTreeSet(testTree3_inRL_set,vals,inOrderRL);
  ASSERT_TRUE(checkTreeMove(testTree3_inRL_set,vals,inOrderRL));
  ASSERT_TRUE(checkTreeMove(testTree3_inRL_set,vals,preOrderRL));
  ASSERT_TRUE(checkTreeMove(testTree3_inRL_set,vals,postOrderRL));
  ASSERT_TRUE(checkTreeSet(testTree3_inRL_set,vals,inOrderRL));
  ASSERT_TRUE(checkTreeSet(testTree3_inRL_set,vals,preOrderRL));
  ASSERT_TRUE(checkTreeSet(testTree3_inRL_set,vals,postOrderRL));
}


/*
 * Check that a binary radix tree can be filled in
 * random order and correctly traversed.
 */
TEST(BinaryTree, RandomFill) {

  // Same basic procedure as FillTest, except testing that we can perform
  // "random" path layering
  std::vector<PathVal32> vals;
  uint32_t v{0};
  addAllThroughDepth<PathVal32>(8, v, std::back_inserter(vals));
  uint32_t endVal = countAtAllThroughDepth(8);

  RandomSeeds seeds;
  // Make identity "layering" of tree - to be shuffled in subsequent line
  std::vector<uint32_t> shuffled = makeIdentityMap<uint32_t>(endVal);
  shuffleContainer(seeds.next(),shuffled);

  // Create pre-order, in-order, and post-order moves to go over shuffled move
  std::vector<uint32_t> preOrder(shuffled);
  std::sort(preOrder.begin(), preOrder.end(), [&vals](std::size_t a, std::size_t b) -> bool { return PathSortPreOrder<BinaryTestPath32>{}(vals[a], vals[b]); });

  std::vector<uint32_t> postOrder(shuffled);
  std::sort(postOrder.begin(), postOrder.end(), [&vals](std::size_t a, std::size_t b) -> bool { return PathSortPostOrder<BinaryTestPath32>{}(vals[a], vals[b]); });

  std::vector<uint32_t> inOrder(shuffled);
  std::sort(inOrder.begin(), inOrder.end(), [&vals](std::size_t a, std::size_t b) -> bool { return PathSortInOrder<BinaryTestPath32>{}(vals[a], vals[b]); });

	Tree8_3 fillTree;
  addToTreeMove(fillTree, vals, shuffled);
  ASSERT_TRUE(checkTreeMove(fillTree, vals, shuffled));

  // Traverse shuffled in various orders 
  ASSERT_TRUE(checkTreeMove(fillTree, vals, preOrder));
  ASSERT_TRUE(checkTreeMove(fillTree, vals, postOrder));
  ASSERT_TRUE(checkTreeMove(fillTree, vals, inOrder));

}


/*
 * Ensure that a binary radix tree can be filled in
 * random order and correctly traversed; as a side, checks correctness
 * of edge trim_front() member function. 
 */
TEST(BinaryTree, sparseRandomFill) {

  // Same basic procedure as Random, except without the use
  // of our random number generator and the tree is only
  // partly filled.
  std::vector<PathVal32> vals;
  uint32_t v{0};
  addAllThroughDepth<PathVal32>(8, v, std::back_inserter(vals));
  uint32_t endVal = countAtAllThroughDepth(8);

  // Make identity "layering" of tree - to be shuffled in subsequent line
  RandomSeeds seeds;
  std::vector<uint32_t> shuffled = makeIdentityMap<uint32_t>(endVal);
  shuffleContainer(seeds.next(),shuffled);

  // The shuffled vector contains all paths to fill tree of depth 8;
  // create new vector with only a few paths from shuffled
  std::vector<uint32_t> copied;
  for (int i = 0; i < 17; i++) { copied.push_back(shuffled[i]); }

  // Create pre-order, in-order, and post-order moves to go over shuffled move
  std::vector<uint32_t> preOrder(copied);
  std::sort(preOrder.begin(), preOrder.end(), [&vals](std::size_t a, std::size_t b) -> bool { return PathSortPreOrder<BinaryTestPath32>{}(vals[a], vals[b]); });

  std::vector<uint32_t> postOrder(copied);
  std::sort(postOrder.begin(), postOrder.end(), [&vals](std::size_t a, std::size_t b) -> bool { return PathSortPostOrder<BinaryTestPath32>{}(vals[a], vals[b]); });

  std::vector<uint32_t> inOrder(copied);
  std::sort(inOrder.begin(), inOrder.end(), [&vals](std::size_t a, std::size_t b) -> bool { return PathSortInOrder<BinaryTestPath32>{}(vals[a], vals[b]); });

	Tree8_3 fillTree;
  addToTreeMove(fillTree, vals, copied);

  ASSERT_TRUE(checkTreeMove(fillTree, vals, copied));

  // Traverse shuffled in various orders 
  ASSERT_TRUE(checkTreeMove(fillTree, vals, preOrder));
  ASSERT_TRUE(checkTreeMove(fillTree, vals, postOrder));
  ASSERT_TRUE(checkTreeMove(fillTree, vals, inOrder));

}

/*
 * Test total value/node removal, using post-order traversal.
 * Note that this approach is necessary because only leaf nodes may
 * be deleted.  
 */
TEST(BinaryTree, fullDelete) {

  // Fully populate a tree of depth 16
  std::vector<PathVal32> vals;
  uint32_t v{0};
  addAllThroughDepth<PathVal32>(16,v,std::back_inserter(vals));
  uint32_t expectedEndValue = countAtAllThroughDepth(16);

  std::vector<uint32_t> shallowToDeepLR = makeIdentityMap<uint32_t>(expectedEndValue);
 
  Tree16_3 delete_tree;
  auto c = delete_tree.cursor();
  addToTreeMove(delete_tree, vals, shallowToDeepLR);
  ASSERT_TRUE(checkTreeMove(delete_tree, vals, shallowToDeepLR));
  

  BinaryPath<16> p;

  // Using preorder traversal, clear all values created
  preOrderTest(p, c, [](decltype(p)& /*pth*/, decltype(c)& cbc) {
                      if (cbc.atValue()) { cbc.clearValue(); }
                      });

  cursorGotoRoot(c);
  preOrderTest(p, c, [](decltype(p)& /*pth*/, decltype(c)& cbc) {
                      	ASSERT_TRUE(!(cbc.atValue()));
                        });
  cursorGotoRoot(c);
  postOrderDelete(c);
}


/*
 * Test some topologies that gave us some specific issues.
 */
TEST(BinaryTree, TestKnownProblems) { 

	using PathVal = TestPathValue<BinaryTestPath16, uint32_t>; 
	
	Tree16<3> sparseTree;	

	auto c = sparseTree.cursor();

  /* Test the following tree topography:
   * 
   *              (root)
   *                  \1
   *									()
   *                0/	\1	
   *               0/\1  \1
   *              0/ ()   \1
   *             0/ 0/     \1
   *              \1 \1     \1
   *               \1()    0/
   *               ()      ()
   * 
   * Topology is unique in two ways: (1) There is a node immediately preceding the
   * root node, and (2) there is a path that "covers" another smaller path creating
   * two nodes off of the cumulative path 10101. Traversing this topology with a
   * single cursor checks both the correctness of the cursor state and the underlying tree. 
   *
   */

  // Generate 'BasePath' - 101 with a value at the end of the path		
	std::mt19937 g(RandomSeeds::at(8));
  int randValue = g();
	PathVal BasePath({1,0,1}, randValue);
	cursorGotoRoot(c);
	BasePath.setCursor(c);
	c.addNode();
	ASSERT_TRUE(c.nodeValue().atNode());
	c.nodeValue().set(BasePath.value);
	ASSERT_EQ(*(c.nodeValue().getPtrRO()), BasePath.value);
	cursorGotoRoot(c);

  // Generate first 'MergePath' - 1111110 with end path value
	uint32_t randValue2 = g();
	auto c2 = sparseTree.cursor();
	PathVal MergePath({1,1,1,1,1,1,0}, randValue2);
	MergePath.setCursor(c2);
	ASSERT_FALSE(c2.nodeValue().atNode());
	c2.addNode();
	ASSERT_TRUE(c2.nodeValue().atNode());
	c2.nodeValue().set(MergePath.value);
	ASSERT_EQ(*(c2.nodeValue().getPtrRO()), MergePath.value);	
	cursorGotoRoot(c2);

  // Go over BasePath after adding MergePath
	auto c3 = sparseTree.cursor();
	cursorGotoRoot(c3);
	ASSERT_TRUE(c3.nodeValue().atNode());
	BasePath.moveCursor(c3);
	ASSERT_TRUE(c3.atNode());
	ASSERT_EQ(*(c3.nodeValue().getPtrRO()), randValue);
	cursorGotoRoot(c3);

	// Generate 2nd MergePath - 10101 with end path value
	uint32_t M2_value = g();
	PathVal MergePath2({1,0,1,0,1}, M2_value);
	auto c4 = sparseTree.cursor();
	MergePath2.setCursor(c4);
	c4.addNode();
	c4.nodeValue().set(MergePath2.value);
	ASSERT_TRUE(c4.atNode());
	
	// Go down to BasePath node, then back up, and then down again to 
  // MergePath2 node 
	BasePath.setCursor(c4);
	ASSERT_EQ(*(c4.nodeValue().getPtrRO()), randValue) ;
	MergePath2.setCursor(c4);
	ASSERT_TRUE(c4.atNode());
	ASSERT_EQ(*(c4.nodeValue().getPtrRO()), M2_value);	
	MergePath.moveCursorFrom(c4, MergePath2);	

  // Generate 3rd MergePath - 1000011 with end path value
	uint32_t M3_value = g();
	PathVal MergePath3({1,0,0,0,0,1,1}, M3_value);
	MergePath3.setCursor(c4);
	c4.addNode();
	c4.nodeValue().set(MergePath3.value);
	ASSERT_EQ(*(c4.nodeValue().getPtrRO()), M3_value);	

	/* Test the following tree topology:
   *  
   *              (root)
   *             0/
   *             ()
   *            0/
   *            .
   *           .
   *          .
   *        0/
   *        ()
   *
   * Topology is unique in that the tree is a series of paths
   * that are built on one another.
   */

	Tree16<3> seq_Tree;

	auto seqC = seq_Tree.cursor();

	PathVal seqPath1({0}, g());

  // Create first path 
	seqPath1.setCursor(seqC);
	seqC.addNode();
	seqC.nodeValue().set(seqPath1.value);
	ASSERT_TRUE(seqC.nodeValue().atNode());
	ASSERT_EQ(*(seqC.nodeValue().getPtrRO()), seqPath1.value);	
	cursorGotoRoot(seqC);

	PathVal seqPath2({0,0}, g());
	PathVal seqPath3({0,0,0}, g());
	PathVal seqPath4({0,0,0,0}, g());
  PathVal seqPath5({0,0,0,0,0}, g());
  PathVal seqPath6({0,0,0,0,0,0}, g());
  PathVal seqPath7({0,0,0,0,0,0,0}, g());
  PathVal seqPath8({0,0,0,0,0,0,0,0}, g());
  PathVal seqPath9({0,0,0,0,0,0,0,0,0}, g());
  PathVal seqPath10({0,0,0,0,0,0,0,0,0,0}, g());
  PathVal seqPath11({0,0,0,0,0,0,0,0,0,0,0}, g());
  PathVal seqPath12({0,0,0,0,0,0,0,0,0,0,0,0}, g()); 
  PathVal seqArray[] = {seqPath1, seqPath2, seqPath3, seqPath4, seqPath5, seqPath6,
  seqPath7, seqPath8, seqPath9, seqPath10, seqPath11, seqPath12};

	// Set nodes and values between each edge
  for (int i = 0; i < 11; i++) {
    seqArray[i].setCursor(seqC);
    seqC.addNode();
		seqC.nodeValue().set(seqArray[i].value);	
	}

	seqArray[11].setCursor(seqC);
  
  // Traverse upward from bottom node, jumping along previous paths
	for (int i = 11; i > 0; i--){
		seqArray[i-1].moveCursorFrom(seqC, seqArray[i]); 
		ASSERT_TRUE(seqC.atNode());
    ASSERT_EQ(*(seqC.nodeValue().getPtrRO()), seqArray[i-1].value);
  }

  /* Test the following tree topology:
   * 
   *            (root)
   *           0/
   *           ()
   *          0/\1
   *          ()()
   *         0/\1
   *         . () 
   *        .
   *       .
   *     0/\1
   *       ()
   *
   * Topology was designed to put an emphasis on traversing through
   * nodes at junctions of paths, rather than first returning to the
   * root and then moving to end of target path
   */
	Tree16<3> hookedTree;
	
	auto hookedC = hookedTree.cursor();

  PathVal backbone({0,0,0,0,0,0,0,0,0,0}, g());
	backbone.setCursor(hookedC);
	
	PathVal finger1({0,1}, g());
	PathVal finger2({0,0,1}, g());
	PathVal finger3({0,0,0,1}, g());
  PathVal finger4({0,0,0,0,1}, g());
  PathVal finger5({0,0,0,0,0,1}, g());
  PathVal finger6({0,0,0,0,0,0,1}, g());
  PathVal finger7({0,0,0,0,0,0,0,1}, g());
  PathVal finger8({0,0,0,0,0,0,0,0,1}, g());
  PathVal finger9({0,0,0,0,0,0,0,0,0,1}, g());
  PathVal finger10({0,0,0,0,0,0,0,0,0,0,1}, g());

	
	PathVal fingers[] = {finger1, finger2, finger3, finger4, finger5, finger6,
  finger7, finger8, finger9, finger10};

  // Initialize nodes with values at end of each "finger"
	for (int i = 0; i < 9; i++) {
    fingers[i].setCursor(hookedC);
    hookedC.addNode();
    hookedC.nodeValue().set(fingers[i].value);
  }

  // Move to very bottom of path "backbone"
	backbone.setCursor(hookedC);
	finger10.moveCursorFrom(hookedC, backbone);

	// Starting at bottom finger, hook around to the next finger, 
  // moving toward the root
  for (int i = 9; i > 0; i--) {
    fingers[i-1].moveCursorFrom(hookedC, fingers[i]);
    ASSERT_TRUE(hookedC.atNode());
    ASSERT_EQ(*(hookedC.nodeValue().getPtrRO()), fingers[i-1].value);
  }
}


/* 
 * Fills a tree to half capacity, adds values to existing leaves,
 * and walks from leaf-to-leaf.   
 * 
 */ 

TEST(BinaryTree, WalkLeaves) {

  // Create paths for full tree 
  std::vector<PathVal32> paths;
  uint32_t v{0};
  addAllAtDepth<PathVal32>(16, v, std::back_inserter(paths));
  uint32_t endVal = countAtDepth(16);
 
  // Shuffle the paths, so we can create a pseudo-random path order for the tree
  RandomSeeds seeds;
  shuffleContainer(seeds.next(),paths);

  // Vector of paths that will store 1/2 of paths
  std::vector<PathVal32> sparsePaths;

  // Do not create a complete binary tree; fill tree only halfway 
  for (std::size_t i  = 0; i < ((paths.size())/2); i++) { 
    sparsePaths.push_back(paths[i]);
  }

  std::vector<uint32_t> shallowToDeep = makeIdentityMap<uint32_t>(endVal/2);

  // Initialize binary tree with sparse paths and values only at leaf nodes at
  // depth 16
  Tree16<3> leafTree;
  auto leafC = leafTree.cursor();
  addToTreeMove(leafTree, sparsePaths, shallowToDeep);

  for (std::size_t i = 0; i < sparsePaths.size(); i++) { 
    sparsePaths[i].setCursor(leafC);
    ASSERT_TRUE(leafC.atNode());
    ASSERT_TRUE(leafC.atValue());
  }

  cursorGotoRoot(leafC);
  ASSERT_TRUE(leafC.atNode());

  // In pre-order traversal, check that all values at leaves are what we expect them to be 
  std::vector<uint32_t> preOrder(shallowToDeep);
  std::sort (preOrder.begin(), preOrder.end(), [&sparsePaths](std::size_t a, std::size_t b) -> bool { return PathSortPreOrder<BinaryTestPath32>{}(sparsePaths[a], sparsePaths[b]); });
  ASSERT_TRUE(checkTreeMove(leafTree, sparsePaths, preOrder));

}

using BinaryPath16 = TestPathValue<BinaryTestPath<16,uint16_t>,uint64_t>;
TEST(BinaryTree, NewFillTest) {
  RandomNumbers<std::size_t> rn(RandomSeeds::seed(0));
  auto newTree = [](){ return Tree16_3{}; };
  std::string result = fillEntireTree<BinaryPath16,Tree16_3>(rn,5,newTree);
  ASSERT_EQ(result,"OK");
}

TEST(BinaryTree, FillSomeOfTest) {
  RandomSeeds seeds;
  RandomNumbers<std::size_t> rnShuffle(seeds.next());
  RandomNumbers<uint64_t> rnChoose(seeds.next());
  auto newTree = [](){ return Tree16_3{}; };
  std::vector<float> fillRatios{0.9,0.75,0.5,0.25,0.1};
  for (float fillRatio : fillRatios) {
    std::string result = fillSomeOfTree<BinaryPath16,Tree16_3>(rnShuffle,5,rnChoose,fillRatio,newTree);
    ASSERT_EQ(result,"OK");
  }
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
