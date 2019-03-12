#ifndef AKAMAI_MAPPER_RADIX_TREE_TEST_BINARY_TREE_TEST_UTILS_H_
#define AKAMAI_MAPPER_RADIX_TREE_TEST_BINARY_TREE_TEST_UTILS_H_

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

#include <limits>
#include <exception>
#include <algorithm>
#include <string>

#include "../RadixTree/BinaryRadixTree.h"

#include "../RadixTree/SimplePath.h"
#include "../RadixTree/SimpleEdge.h"

#include "PathEdgeTestUtils.h"


// Put together a shallow tree - tractable to fill it up entirely for testing
using Path16 = Akamai::Mapper::RadixTree::BinaryPath<16>;
using SimplePath16 = Akamai::Mapper::RadixTree::SimplePath<2, 16>;
using Path8 = Akamai::Mapper::RadixTree::BinaryPath<8>;
using Path6 = Akamai::Mapper::RadixTree::BinaryPath<6>;
using Path7 = Akamai::Mapper::RadixTree::BinaryPath<7>;
using Path4 = Akamai::Mapper::RadixTree::BinaryPath<4>;

// Varying edge sizes, from trivial on up
using Edge1 = Akamai::Mapper::RadixTree::BinaryWordEdge<uint32_t,8,1>;
using Edge2 = Akamai::Mapper::RadixTree::BinaryWordEdge<uint32_t,8,2>;
using Edge3 = Akamai::Mapper::RadixTree::BinaryWordEdge<uint32_t,8,3>;
using Edge3_Simple = Akamai::Mapper::RadixTree::SimpleEdge<2, 3>;

//using Edge3 = Mapper::RadixTree::SimpleEdge<2, 3>;

using Edge4 = Akamai::Mapper::RadixTree::BinaryWordEdge<uint32_t,8,4>;
using Edge5 = Akamai::Mapper::RadixTree::BinaryWordEdge<uint32_t,8,5>;
using Edge6 = Akamai::Mapper::RadixTree::BinaryWordEdge<uint32_t,8,6>;
using Edge7 = Akamai::Mapper::RadixTree::BinaryWordEdge<uint32_t,8,7>;
using Edge8 = Akamai::Mapper::RadixTree::BinaryWordEdge<uint32_t,8,8>;
// Edge just short of covering the shallow tree
using Edge14 = Akamai::Mapper::RadixTree::BinaryWordEdge<uint32_t,8,14>;
// Just big enough to cover our shallow tree
using Edge15 = Akamai::Mapper::RadixTree::BinaryWordEdge<uint32_t,8,15>;
// Just bigger than the shallow tree
using Edge16 = Akamai::Mapper::RadixTree::BinaryWordEdge<uint32_t,8,16>;
// Edge covering well over the shallow tree
using Edge24 = Akamai::Mapper::RadixTree::BinaryWordEdge<uint32_t,8,24>;

// Edge using up all 32 bits
using EdgeSimple32 = Akamai::Mapper::RadixTree::SimpleBinaryWordEdge<uint32_t>;


template <typename ValueT,typename PathT,typename EdgeT>
using BinaryNodeRadixTree = Akamai::Mapper::RadixTree::RadixTree<PathT,Akamai::Mapper::RadixTree::BinaryTreeNode<ValueT,EdgeT>,Akamai::Mapper::RadixTree::SimpleFixedDepthStack>;

using Tree4_3 = BinaryNodeRadixTree<uint32_t,Path4,Edge3>;
using Tree6_3 = BinaryNodeRadixTree<uint32_t, Path6, Edge3>;
using Tree7_3 = BinaryNodeRadixTree<uint32_t, Path7, Edge3>;
using Tree16_3 = BinaryNodeRadixTree<uint32_t,Path16,Edge3>;
using Tree8_3 = BinaryNodeRadixTree<uint32_t, Path8, Edge3>;
using Tree16_14 = BinaryNodeRadixTree<uint32_t,Path16,Edge14>;
using Tree16_15 = BinaryNodeRadixTree<uint32_t,Path16,Edge15>;
using Tree16_16 = BinaryNodeRadixTree<uint32_t,Path16,Edge16>;
using Tree16_Simple32 = BinaryNodeRadixTree<uint32_t, SimplePath16,EdgeSimple32>;






////////////////////////////////////////
//          Testing Functions         //
////////////////////////////////////////

/*
 * Populate tree with paths and values at specified depth, d. 
 */
template <typename IntPathValueType,typename InsertIterator>
inline void addAllAtDepth(uint8_t d,typename IntPathValueType::ValueType& v,InsertIterator&& ii) {
  using IntType = typename IntPathValueType::PathIntType;
  using PathType = typename IntPathValueType::PathType;
  if (d > 8*sizeof(IntType)) { throw std::out_of_range{"depth exceeds capacity of path"}; }
  IntType max((static_cast<IntType>(0x1) << d) - 1);
  IntType x = 0;
  // Can't use a "for" loop here - at maximum d we exceed IntType capacity
  do { *ii = IntPathValueType{PathType::FromInt(x++,d),v++}; } while ((x != 0) && (x <= max));
 }

/*
 * Populate tree with paths and values through specified depth, d
 */
template <typename IntPathValueType,typename InsertIterator>
inline void addAllThroughDepth(uint8_t d,typename IntPathValueType::ValueType& v,InsertIterator&& ii) {
  using IntType = typename IntPathValueType::PathIntType;
  if (d > 8*sizeof(IntType)) { throw std::out_of_range{"depth exceeds capacity of path"}; }
  for (uint8_t i = 0U; i <= d; ++i) {
    addAllAtDepth<IntPathValueType>(i,v,std::forward<InsertIterator>(ii));
  }
}

/*
 * Returns number of values stored at depth d
 */
inline uint64_t countAtDepth(uint8_t d) {
  if (d > 8*sizeof(uint64_t)) { throw std::out_of_range{"result exceeds capacity of uint64_t"}; }
  return (static_cast<uint64_t>(0x1) << d);
}

/*
 * Returns number of values stored through depth d
 */
inline uint64_t countAtAllThroughDepth(uint8_t d) {
  if (d > (8*sizeof(uint64_t) - 1)) { throw std::out_of_range{"result exceeds capacity of uint64_t"}; }
  return ((static_cast<uint64_t>(0x1) << (d + 1)) - 1);
}

/*
 * Create identity map of values to be stored in testing tree - e.g. for depth 2 binary tree,
 * we have values 0, 1, 2, 3, 4, 5 to be stored in tree nodes
 */
template <typename IntType>
inline std::vector<IntType> makeIdentityMap(IntType n) {
  static_assert(!std::numeric_limits<IntType>::is_signed,"IntType must be unsigned");
  std::vector<IntType> v;
  v.reserve(n);
  for (uint32_t i=0;i<n;++i) { v.push_back(i); }
  return std::move(v);
}

/*
 * Add traversal to existing tree; used to ensure, in conjunction with 
 * checkTreeMove(), that a tree may be travered in specified order.
 */
template <typename TreeType,typename PathVal,typename IntType>
inline void addToTreeMove(TreeType&& t,const std::vector<PathVal>& pv,const std::vector<IntType>& order) {
  PathVal prev;
  auto c = t.cursor();
  for (IntType i : order) {
    pv[i].moveCursorFromSetValue(c,prev);
    prev = pv[i];
  }
}

/*
 * For the tree, add nodes with associated values at path intersections and ends.
 */
template <typename TreeType,typename PathVal,typename IntType>
inline void addToTreeSet(TreeType&& t,const std::vector<PathVal>& pv,const std::vector<IntType>& order) {
  auto c = t.cursor();
  for (IntType i : order) { pv[i].setCursorValue(c); }
}

/*
 * Check that a tree may be traversed in given order.
 */
template<typename TreeType, typename PathVal, typename IntType>
inline bool checkTreeMove(TreeType&& t,const std::vector<PathVal>& pv,const std::vector<IntType>& order) {
  PathVal prev;
  auto c = t.cursorRO();
  for (IntType i : order) {
    pv[i].moveCursorFrom(c,prev);
    if (!c.atValue()) {
      std::cout << "checkTreeMove: missing value at " << pathToString(pv[i]) << std::endl;
      return false;
    }
    if (*(c.nodeValue().getPtrRO()) != pv[i].value) {
      std::cout << "checkTreeMove: incorrect value '" << std::to_string(*(c.nodeValue().getPtrRO()))
                << "' at " << pathToString(pv[i]) << std::endl;
      return false;
    }
    prev = pv[i];
  }
  return true;
}

/*
 * Check that values in tree are as expected.
 */
template <typename TreeType,typename PathVal,typename IntType>
inline bool checkTreeSet(TreeType&& t,const std::vector<PathVal>& pv,const std::vector<IntType>& order) {
  auto c = t.cursorRO();
  for (IntType i : order) {
    pv[i].setCursor(c);
    if (!c.atValue()) {
      std::cout << "checkTreeSet: missing value at "
                << pathToString(pv[i]) << std::endl;
      return false;
    }
    if (*(c.nodeValue().getPtrRO()) != pv[i].value) {
      std::cout << "checkTreeSet: incorrect value '" << std::to_string(*(c.nodeValue().getPtrRO())) << "' at "
                << pathToString(pv[i]) << std::endl;
      return false;
    }
  }
  return true;
}


#endif
