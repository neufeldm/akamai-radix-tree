#ifndef AKAMAI_MAPPER_RADIX_TREE_TREE_TESTS_H_
#define AKAMAI_MAPPER_RADIX_TREE_TREE_TESTS_H_

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

#include "TreeTestUtils.h"
#include "PathSort.h"
#include "RandomUtils.h"


template <typename CursorType>
inline std::string clearTree(CursorType&& c);

template <typename PathValueType,typename CursorType>
inline std::string
checkTree(RandomNumbers<std::size_t>& rn,std::size_t shuffleCount,TreeSpotList<PathValueType>& sl,CursorType&& c);

template <typename PathValueType,typename CursorFactoryType>
inline std::string
checkTreeNewCursor(RandomNumbers<std::size_t>& rn,std::size_t shuffleCount,TreeSpotList<PathValueType>& sl,CursorFactoryType&& c);

template <typename PathValueType,typename TreeType,typename TreeFactoryType>
inline std::string
fillAndCheckTree(RandomNumbers<std::size_t>& rn,std::size_t shuffleCount,TreeSpotList<PathValueType>& treeList,TreeFactoryType&& tf);


template <typename PathValueType,typename TreeType,typename TreeFactoryType>
inline std::string
fillEntireTree(RandomNumbers<std::size_t>& rn,std::size_t shuffleCount,TreeFactoryType&& tf);

template <typename PathValueType,typename TreeType,typename TreeFactoryType>
inline std::string
fillSomeOfTree(RandomNumbers<std::size_t>& rnShuffle,std::size_t shuffleCount,RandomNumbers<uint64_t>& rnChoose,double density,TreeFactoryType&& tf);

template <typename PathValueType,typename TreeType,typename TreeFactoryType>
inline std::string
fillEntireLayer(RandomNumbers<std::size_t>& rn,std::size_t shuffleCount,std::size_t layer,TreeFactoryType&& tf);

template <typename PathValueType,typename TreeType,typename TreeFactoryType>
inline std::string
fillSomeLayers(RandomNumbers<std::size_t>& rn,std::size_t shuffleCount,std::size_t layerCount,TreeFactoryType&& tf);

/////////////////////
// IMPLEMENTATIONS //
/////////////////////

template <typename CursorType>
std::string clearTree(CursorType&& c) {
  for (std::size_t i=0;i < std::decay<CursorType>::type::Radix;++i) {
    if (c.canGoChildNode(i)) {
      if (!c.goChild(i)) { return "Cursor unable to go to child " + std::to_string(i); }
      std::string rr = clearTree(std::forward<CursorType>(c));
      if (rr != "OK") { return rr; }
      if (!c.goParent()) { return "Cursor unable to go to parent from child " + std::to_string(i); }
    }
  }
  if (c.atValue()) { c.clearValue(); }
  if (c.atNode()) {
    if (!c.removeNode() && c.canGoParent()) { return "Unable to remove node"; }
  }
  return "OK";
}



template <typename PathValueType,typename CursorType>
typename std::enable_if<(PathValueType::Radix % 2) != 0,std::string>::type
checkTreeImpl(TreeSpotList<PathValueType>& sl,CursorType&& c) {
  static_assert(PathValueType::Radix == CursorType::Radix,"path/cursor radix mismatch");
  static_assert(PathValueType::MaxDepth == CursorType::MaxDepth,"path/cursor depth mismatch");

  std::string r;

  // check tree in all applicable orders
  sl.resetSequence();
  cursorGotoRoot(std::forward<CursorType>(c));
  r = sl.checkTree(std::forward<CursorType>(c));
  if (r != "OK") { return r; }
  sl.resetSequence();
  sl.reverse();
  cursorGotoRoot(std::forward<CursorType>(c));
  r = sl.checkTree(std::forward<CursorType>(c));
  if (r != "OK") { return r; }

  sl.sort(PathSortPreOrder<PathValueType>{});
  cursorGotoRoot(std::forward<CursorType>(c));
  r = sl.checkTree(std::forward<CursorType>(c));
  if (r != "OK") { return r; }

  sl.sort(PathSortPostOrder<PathValueType>{});
  cursorGotoRoot(std::forward<CursorType>(c));
  r = sl.checkTree(std::forward<CursorType>(c));
  if (r != "OK") { return r; }

  return "OK";
}

template <typename PathValueType,typename CursorType>
typename std::enable_if<(PathValueType::Radix % 2) == 0,std::string>::type
checkTreeImpl(TreeSpotList<PathValueType>& sl,CursorType&& c) {
  static_assert(PathValueType::Radix == CursorType::Radix,"path/cursor radix mismatch");
  static_assert(PathValueType::MaxDepth == CursorType::MaxDepth,"path/cursor depth mismatch");

  std::string r;

  // check tree in all applicable orders
  sl.resetSequence();
  cursorGotoRoot(std::forward<CursorType>(c));
  r = sl.checkTree(std::forward<CursorType>(c));
  if (r != "OK") { return r; }

  sl.resetSequence();
  sl.reverse();
  cursorGotoRoot(std::forward<CursorType>(c));
  r = sl.checkTree(std::forward<CursorType>(c));
  if (r != "OK") { return r; }

  sl.sort(PathSortPreOrder<PathValueType>{});
  cursorGotoRoot(std::forward<CursorType>(c));
  sl.checkTree(std::forward<CursorType>(c));
  if (r != "OK") { return r; }
  
  sl.sort(PathSortPostOrder<PathValueType>{});
  cursorGotoRoot(std::forward<CursorType>(c));
  sl.checkTree(std::forward<CursorType>(c));
  if (r != "OK") { return r; }

  sl.sort(PathSortInOrder<PathValueType>{});
  cursorGotoRoot(std::forward<CursorType>(c));
  sl.checkTree(std::forward<CursorType>(c));
  if (r != "OK") { return r; }

  return "OK";
}

template <typename PathValueType,typename CursorType>
std::string
checkTree(TreeSpotList<PathValueType>& sl,CursorType&& c) {
  return checkTreeImpl<PathValueType,CursorType>(sl,std::forward<CursorType>(c));
}


template <typename PathValueType,typename CursorFactoryType>
typename std::enable_if<(PathValueType::Radix % 2) != 0,std::string>::type
checkTreeNewCursorImpl(TreeSpotList<PathValueType>& sl,CursorFactoryType&& cf) {
  //using CursorType = typename std::decay<typename std::result_of<CursorFactoryType>::type>::type;
  //static_assert(PathValueType::Radix == CursorType::Radix,"path/cursor radix mismatch");
  //static_assert(PathValueType::MaxDepth == CursorType::MaxDepth,"path/cursor depth mismatch");

  std::string r;

  // check tree in all applicable orders
  sl.resetSequence();
  r = sl.checkTreeNewCursor(std::forward<CursorFactoryType>(cf));
  if (r != "OK") { return r; }
  sl.resetSequence();
  sl.reverse();

  r = sl.checkTreeNewCursor(std::forward<CursorFactoryType>(cf));
  if (r != "OK") { return r; }

  sl.sort(PathSortPreOrder<PathValueType>{});
  r = sl.checkTreeNewCursor(std::forward<CursorFactoryType>(cf));
  if (r != "OK") { return r; }

  sl.sort(PathSortPostOrder<PathValueType>{});
  r = sl.checkTreeNewCursor(std::forward<CursorFactoryType>(cf));
  if (r != "OK") { return r; }

  return "OK";
}

template <typename PathValueType,typename CursorFactoryType>
typename std::enable_if<(PathValueType::Radix % 2) == 0,std::string>::type
checkTreeNewCursorImpl(TreeSpotList<PathValueType>& sl,CursorFactoryType&& cf) {
  //using CursorType = typename std::decay<typename std::result_of<CursorFactoryType>::type>::type;
  //static_assert(PathValueType::Radix == CursorType::Radix,"path/cursor radix mismatch");
  //static_assert(PathValueType::MaxDepth == CursorType::MaxDepth,"path/cursor depth mismatch");

  std::string r;

  // check tree in all applicable orders
  sl.resetSequence();
  r = sl.checkTreeNewCursor(std::forward<CursorFactoryType>(cf));
  if (r != "OK") { return r; }

  sl.resetSequence();
  sl.reverse();
  r = sl.checkTreeNewCursor(std::forward<CursorFactoryType>(cf));
  if (r != "OK") { return r; }

  sl.sort(PathSortPreOrder<PathValueType>{});
  sl.checkTreeNewCursor(std::forward<CursorFactoryType>(cf));
  if (r != "OK") { return r; }

  sl.sort(PathSortPostOrder<PathValueType>{});
  sl.checkTreeNewCursor(std::forward<CursorFactoryType>(cf));
  if (r != "OK") { return r; }

  sl.sort(PathSortInOrder<PathValueType>{});
  sl.checkTreeNewCursor(std::forward<CursorFactoryType>(cf));
  if (r != "OK") { return r; }

  return "OK";
}

template <typename PathValueType,typename CursorFactoryType>
std::string
checkTreeNewCursor(TreeSpotList<PathValueType>& sl,CursorFactoryType&& cf) {
  return checkTreeNewCursorImpl<PathValueType,CursorFactoryType>(sl,std::forward<CursorFactoryType>(cf));
}

// We've got multiple cursor types we can use - exercise all of them
template <typename PathValueType,typename TreeType>
std::string checkTreeWithAllCursors(TreeSpotList<PathValueType>& treeList,TreeType* tree) {
  std::string r;
  r = checkTree(treeList,tree->cursorRO());
  if (r != "OK") { return r; }
  r = checkTree(treeList,tree->walkCursorRO());
  if (r != "OK") { return r; }
  r = checkTreeNewCursor(treeList,[tree](){return tree->lookupCursorRO();});
  if (r != "OK") { return r; }
 
  return "OK";
}

// Verifies that we can do random shuffle adding of whatever items are in our spot list
template <typename PathValueType,typename TreeType,typename TreeFactoryType>
std::string checkShuffleTreeWithAllCursors(RandomNumbers<std::size_t>& rn,std::size_t shuffleCount,TreeSpotList<PathValueType>& treeList,TreeFactoryType&& tf) {
  std::string r{};
  TreeType tShuffle = tf();
  TreeType tShuffleWO = tf();
  for (std::size_t i=0;i<shuffleCount;++i) {
    treeList.shuffle(rn);

    treeList.addToTree(tShuffle.cursor());
    r = checkTreeWithAllCursors(treeList,&tShuffle);
    if (r != "OK") { return r; }

    treeList.addToTreeNewCursor([&tShuffleWO](){return tShuffleWO.lookupCursorWO();});
    r = checkTreeWithAllCursors(treeList,&tShuffleWO);
    if (r != "OK") { return r; }

    r = clearTree(tShuffle.cursor());
    if (r != "OK") { return r; }
    r = clearTree(tShuffleWO.cursor());
    if (r != "OK") { return r; }

    for (std::size_t i=0;i<TreeType::Radix;++i) {
      if (tShuffle.cursorRO().canGoChildNode(i)) { return "CursorRO can goto child node " + std::to_string(i) + " at root of empty tree"; }
      if (tShuffleWO.cursorRO().canGoChildNode(i)) { return "CursorRO can goto child node " + std::to_string(i) + " at root of empty WO tree"; }

      if (tShuffle.lookupCursorRO().canGoChildNode(i)) { return "LookupCursorRO can goto child node " + std::to_string(i) + " at root of empty tree"; }
      if (tShuffleWO.lookupCursorRO().canGoChildNode(i)) { return "LookupCursorRO can goto child node " + std::to_string(i) + " at root of empty WO tree"; }

      if (tShuffle.lookupCursorWO().canGoChildNode(i)) { return "LookupCursorWO can goto child node " + std::to_string(i) + " at root of empty tree"; }
      if (tShuffleWO.lookupCursorWO().canGoChildNode(i)) { return "LookupCursorWO can goto child node " + std::to_string(i) + " at root of empty WO tree"; }

      if (tShuffle.walkCursorRO().canGoChildNode(i)) { return "WalkCursorRO can goto child node " + std::to_string(i) + " at root of empty tree"; }
      if (tShuffleWO.walkCursorRO().canGoChildNode(i)) { return "WalkCursorRO can goto child node " + std::to_string(i) + " at root of empty WO tree"; }
    }
  }

  return "OK";
}


template <typename PathValueType,typename TreeType,typename TreeFactoryType>
std::string
fillAndCheckTreeImplAllRadix(RandomNumbers<std::size_t>& rn,std::size_t shuffleCount,TreeSpotList<PathValueType>& treeList,TreeFactoryType&& tf) {
  static_assert(PathValueType::Radix == TreeType::Radix,"path/tree radix mismatch");
  static_assert(PathValueType::MaxDepth == TreeType::MaxDepth,"path/tree depth mismatch");

  std::string r;

  // add to trees in all applicable orders, check after adding

  //////////////////// baseline sequence - by layer downward
  treeList.resetSequence();

  // add using regular cursor
  TreeType tByLayerDown = tf();
  treeList.addToTree(tByLayerDown.cursor());
  r = checkTreeWithAllCursors(treeList,&tByLayerDown);
  if (r != "OK") { return r; }

  // add using lookupWO cursor
  TreeType tByLayerDownWO = tf();
  treeList.addToTreeNewCursor([&tByLayerDownWO](){return tByLayerDownWO.lookupCursorWO();});
  r = checkTreeWithAllCursors(treeList,&tByLayerDownWO);
  if (r != "OK") { return r; }

  //////////////////// baseline sequence reversed - by layer upward
  treeList.resetSequence();
  treeList.reverse();

  // add using regular cursor
  TreeType tByLayerUp = tf();
  treeList.addToTree(tByLayerUp.cursor());
  r = checkTreeWithAllCursors(treeList,&tByLayerUp);
  if (r != "OK") { return r; }

  // add using lookupWO cursor
  TreeType tByLayerUpWO = tf();
  treeList.addToTreeNewCursor([&tByLayerUpWO](){return tByLayerUpWO.lookupCursorWO();});
  r = checkTreeWithAllCursors(treeList,&tByLayerUpWO);
  if (r != "OK") { return r; }

  //////////////////// pre order
  treeList.sort(PathSortPreOrder<PathValueType>{});

  // add using regular cursor
  TreeType tPreOrder = tf();
  treeList.addToTree(tPreOrder.cursor());
  r = checkTreeWithAllCursors(treeList,&tPreOrder);
  if (r != "OK") { return r; }

  // add using lookupWO cursor
  TreeType tPreOrderWO = tf();
  treeList.addToTreeNewCursor([&tPreOrderWO](){return tPreOrderWO.lookupCursorWO();});
  r = checkTreeWithAllCursors(treeList,&tPreOrderWO);
  if (r != "OK") { return r; }

  //////////////////// post order
  treeList.sort(PathSortPostOrder<PathValueType>{});

  // add using regular cursor
  TreeType tPostOrder = tf();
  treeList.addToTree(tPostOrder.cursor());
  r = checkTreeWithAllCursors(treeList,&tPostOrder);
  if (r != "OK") { return r; }

  // add using lookupWO cursor
  TreeType tPostOrderWO = tf();
  treeList.addToTreeNewCursor([&tPostOrderWO](){return tPostOrderWO.lookupCursorWO();});
  r = checkTreeWithAllCursors(treeList,&tPostOrderWO);
  if (r != "OK") { return r; }

  r = checkShuffleTreeWithAllCursors<PathValueType,TreeType>(rn,shuffleCount,treeList,std::forward<TreeFactoryType>(tf));
  if (r != "OK") { return r; }

  return "OK";
}


// For odd radix do everything, no in-order
template <typename PathValueType,typename TreeType,typename TreeFactoryType>
typename std::enable_if<(PathValueType::Radix % 2) != 0,std::string>::type
fillAndCheckTreeImpl(RandomNumbers<std::size_t>& rn,std::size_t shuffleCount,TreeSpotList<PathValueType>& treeList,TreeFactoryType&& tf) {
  return fillAndCheckTreeImplAllRadix<PathValueType,TreeType>(rn,shuffleCount,treeList,std::forward<TreeFactoryType>(tf));
}

// For even radix do everything plus in-order
template <typename PathValueType,typename TreeType,typename TreeFactoryType>
typename std::enable_if<(PathValueType::Radix % 2) == 0,std::string>::type
fillAndCheckTreeImpl(RandomNumbers<std::size_t>& rn,std::size_t shuffleCount,TreeSpotList<PathValueType>& treeList,TreeFactoryType&& tf) {
  std::string r;
  r = fillAndCheckTreeImplAllRadix<PathValueType,TreeType>(rn,shuffleCount,treeList,std::forward<TreeFactoryType>(tf));
  if (r != "OK") { return r; }

  //////////////////// in order
  treeList.sort(PathSortInOrder<PathValueType>{});

  // add using regular cursor
  TreeType tInOrder = tf();
  treeList.addToTree(tInOrder.cursor());
  r = checkTreeWithAllCursors(treeList,&tInOrder);
  if (r != "OK") { return r; }

  // add using lookupWO cursor
  TreeType tInOrderWO = tf();
  treeList.addToTreeNewCursor([&tInOrderWO](){return tInOrderWO.lookupCursorWO();});
  r = checkTreeWithAllCursors(treeList,&tInOrderWO);
  if (r != "OK") { return r; }

  return "OK";
}

template <typename PathValueType,typename TreeType,typename TreeFactoryType>
std::string
fillAndCheckTree(RandomNumbers<std::size_t>& rn,std::size_t shuffleCount,TreeSpotList<PathValueType>& treeList,TreeFactoryType&& tf)
{
  return fillAndCheckTreeImpl<PathValueType,TreeType,TreeFactoryType>(rn,shuffleCount,treeList,std::forward<TreeFactoryType>(tf));
}


template <typename PathValueType,typename TreeType,typename TreeFactoryType>
std::string
fillEntireTree(RandomNumbers<std::size_t>& rn,std::size_t shuffleCount,TreeFactoryType&& tf) {
  TreeSpotList<PathValueType> fullTree = spotListFillTree<PathValueType>();
  return fillAndCheckTree<PathValueType,TreeType>(rn,shuffleCount,fullTree,std::forward<TreeFactoryType>(tf));
}

template <typename PathValueType,typename TreeType,typename TreeFactoryType>
std::string
fillSomeOfTree(RandomNumbers<std::size_t>& rnShuffle,std::size_t shuffleCount,RandomNumbers<uint64_t>& rnChoose,double density,TreeFactoryType&& tf) {
  TreeSpotList<PathValueType> someOfFullTree = spotListFillSomeOfTree<PathValueType>(rnChoose,density);
  return fillAndCheckTree<PathValueType,TreeType>(rnShuffle,shuffleCount,someOfFullTree,std::forward<TreeFactoryType>(tf));
}

template <typename PathValueType,typename TreeType,typename TreeFactoryType>
std::string
fillEntireLayer(RandomNumbers<std::size_t>& rn,std::size_t shuffleCount,std::size_t layer,TreeFactoryType&& tf) {
  TreeSpotList<PathValueType> layerTree = spotListFillLayer<PathValueType>(layer);
  return fillAndCheckTree<PathValueType,TreeType>(rn,shuffleCount,layerTree,std::forward<TreeFactoryType>(tf));
}

template <typename PathValueType,typename TreeType,typename TreeFactoryType>
std::string
fillSomeLayers(RandomNumbers<std::size_t>& rn,std::size_t shuffleCount,std::size_t layerCount,TreeFactoryType&& tf) {
  if (layerCount > (TreeType::MaxDepth+1)) { throw std::range_error("fillSomeLayers: layer count too large for tree"); }
  std::vector<std::size_t> layers;
  layers.reserve(TreeType::MaxDepth + 1);
  for (std::size_t i = 0;i <= TreeType::MaxDepth;++i) { layers.push_back(i); }
  rn.shuffleContainer(layers);
  std::vector<PathValueType> pathValues;
  typename PathValueType::ValueType val{0};
  for (std::size_t i = 0;i < layerCount;++i) {
    std::vector<PathValueType> layerPathValues = allPathValuesAtLength<PathValueType>(layers[i],val);
    pathValues.insert(pathValues.end(),layerPathValues.begin(),layerPathValues.end());
  }
  TreeSpotList<PathValueType> randomLayerTree(std::move(pathValues));
  return fillAndCheckTree<PathValueType,TreeType>(rn,shuffleCount,randomLayerTree,std::forward<TreeFactoryType>(tf));
}

// - recursive pre/post/in order walking routines?
// - fill entire tree (pre/post/in/random orders)
// - fill entire layer (pre/post/in/random orders)
// - fill random sample of paths at layer (pre/post/in/random orders)
// - fill random sample of paths at random layers (pre/post/in/random orders)

#endif