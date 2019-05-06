#ifndef AKAMAI_MAPPER_RADIX_TREE_BINARY_WORM_TREE_BUILDER_H_
#define AKAMAI_MAPPER_RADIX_TREE_BINARY_WORM_TREE_BUILDER_H_

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

#include <stdint.h>
#include <array>
#include <vector>
#include <utility>

#include "BinaryWORMNode.h"

/**
 * \file BinaryWORMTreeBuilder.h
 * 
 * This file contains an implementation of a class that can construct various
 * forms of a compact, byte-packed binary read-only tree - Write Once, Read Many.
 * These trees may use different size integers to store offset pointers within
 * the tree, depending on the size of the tree. However, it isn't possible
 * to know how large the pointers need to be until the tree has been constructed,
 * creating a "chicken and egg" problem. In order to break this dependency a
 * "dry run" of a tree creation may be performed, producing statistics that
 * determine how large of an offset integer is required for constructing the
 * actual tree. This file contains these statistics classes as well as the
 * actual construction class. A WORM buffer pointer may be given directly
 * to a WORM cursor of matching type for traversal.
 */

namespace Akamai {
namespace Mapper {
namespace RadixTree {

/**
 * \brief Track raw number (count) and byte consumption for a particular node type.
 */
struct BinaryWORMNodeStats {
  std::size_t count{0};
  std::size_t bytes{0};
  BinaryWORMNodeStats() = default;
  BinaryWORMNodeStats(std::size_t c,std::size_t b) : count(c), bytes(b) {}
  bool operator==(const BinaryWORMNodeStats& o) const { return ((count == o.count) && (bytes == o.bytes)); }
  bool operator!=(const BinaryWORMNodeStats& o) const { return !(*this == o); }
  BinaryWORMNodeStats& operator+=(const BinaryWORMNodeStats& rhs) {
    count += rhs.count;
    bytes += rhs.bytes;
    return *this;
  }
  friend BinaryWORMNodeStats operator+(BinaryWORMNodeStats lhs,const BinaryWORMNodeStats& rhs) {
    lhs += rhs;
    return lhs;
  }
  BinaryWORMNodeStats& operator-=(const BinaryWORMNodeStats& rhs) {
    count -= rhs.count;
    bytes -= rhs.bytes;
    return *this;
  }
  friend BinaryWORMNodeStats operator-(BinaryWORMNodeStats lhs,const BinaryWORMNodeStats& rhs) {
    lhs -= rhs;
    return lhs;
  }    
};

/**
 * \brief Track byte/count statistics for all relevant node types in the WORM tree.
 * 
 * Our overall scheme uses different sizes for nodes with no children, single children,
 * both children, and values.
 */
template <typename BinaryWORMNodeT>
struct BinaryWORMNodeStatsTotal {
  using BinaryWORMNodeType = BinaryWORMNodeT;
  BinaryWORMNodeStats values{};
  BinaryWORMNodeStats headersNoChildren{};
  BinaryWORMNodeStats headersSingleChild{};
  BinaryWORMNodeStats headersTwoChildren{};
  static constexpr std::size_t OffsetSize = BinaryWORMNodeType::OffsetSize;
  /**
   * \brief Add the stats for the node to our current state.
   */
  inline void addNode(const BinaryWORMNodeType& n);
  /**
   * \brief Total byte consumption for the native offset pointer size.
   */
  inline std::size_t bytes() const;
  /**
   * \brief Total byte consumption for arbitrary offset pointer sizes.
   */
  inline std::size_t bytes(std::size_t offsetSize) const;
  
  bool operator==(const BinaryWORMNodeStatsTotal<BinaryWORMNodeT>& o) const {
    return ((values == o.values) &&
            (headersNoChildren == o.headersNoChildren) &&
            (headersSingleChild == o.headersSingleChild) &&
            (headersTwoChildren == o.headersTwoChildren));
  }
  bool operator!=(const BinaryWORMNodeStatsTotal<BinaryWORMNodeT>& o) const { return !(*this == o); }
  BinaryWORMNodeStatsTotal& operator+=(const BinaryWORMNodeStatsTotal& rhs) {
    values += rhs.values;
    headersNoChildren += rhs.headersNoChildren;
    headersSingleChild += rhs.headersSingleChild;
    headersTwoChildren += rhs.headersTwoChildren;
    return *this;
  }
  friend BinaryWORMNodeStatsTotal operator+(BinaryWORMNodeStatsTotal lhs,const BinaryWORMNodeStatsTotal& rhs) {
    lhs += rhs;
    return lhs;
  }
  BinaryWORMNodeStatsTotal& operator-=(const BinaryWORMNodeStatsTotal& rhs) {
    values -= rhs.values;
    headersNoChildren -= rhs.headersNoChildren;
    headersSingleChild -= rhs.headersSingleChild;
    headersTwoChildren -= rhs.headersTwoChildren;
    return *this;
  }
  friend BinaryWORMNodeStatsTotal operator-(BinaryWORMNodeStatsTotal lhs,const BinaryWORMNodeStatsTotal& rhs) {
    lhs -= rhs;
    return lhs;
  }    
};

/**
 * \brief Maintain node count/byte consumption stats for the entire tree.
 */
template <typename BinaryWORMNodeT>
struct TreeNodeStats {
  using BinaryWORMNodeType = BinaryWORMNodeT;

  /**
   * \brief Stats for the entire tree.
   */
  BinaryWORMNodeStatsTotal<BinaryWORMNodeType> allNodeStats{};

  static constexpr std::size_t MaxOffsetBytes = sizeof(void*);
  using OffsetByteValues = std::array<std::size_t,MaxOffsetBytes+1>;
  /**
   * \brief Track the longest gap in bytes that an offset pointer must span.
   * 
   * In order to determine the smallest possible number of bytes required
   * for our offset integer we need to know how big of a span it has to cover.
   * The tree building code tracks the maximum gap required for an offset
   * based on all possible offset byte sizes, ranging from 0 up to the size
   * of a pointer on the current architecture. Walking this list until
   * you find the smallest byte count that can cover the full range of
   * its largest pointer gives you the smallest pointer size that can
   * be used for a particular tree.
   */
  OffsetByteValues longestOffsetGap{};

  /**
   * \brief Compute the minimum offset pointer size needed for this tree.
   */
  std::size_t minBytesForOffset() const { return minBytesFor(longestOffsetGap); }

  bool operator==(const TreeNodeStats<BinaryWORMNodeT>& o) const {
    return ((allNodeStats == o.allNodeStats) && (longestOffsetGap == o.longestOffsetGap));
  }
  bool operator!=(const TreeNodeStats<BinaryWORMNodeT>& o) const { return !(*this == o); }

  static std::size_t minBytesFor(const OffsetByteValues& obv) {
    for (std::size_t i=1;i<obv.size();++i) {
      if (obv.at(i) <= maxUIntForBytes(i)) { return i; }
    }
    return 0;
  }
  static std::size_t maxUIntForBytes(std::size_t byteCount) {
    return ((static_cast<std::size_t>(0x1) << 8*byteCount) - 1);
  }
};

/**
 * \brief Builds a binary WORM tree, optionally performing a dry run that allocates no memory.
 * 
 * This class constructs a buffer containing bytes representing a binary WORM tree.
 * The nodes must be added pre-order, and you have to know in advance what children
 * each node has. Only nodes that have two children and/or have values must be added,
 * the builder will add any internal "scaffolding" nodes required along the way.
 * The buffer is allocated using a templated buffer management class. This class
 * must implement two methods: "resize(new size)" and "uint8_t* data()". Not coincidentally
 * a std::vector<uint8_t> may be used directly as a buffer manager. Note that the
 * buffer manager must be "std::moved" into the tree builder class.
 */
template <typename BufferT,typename PathT,typename BinaryWORMNodeT>
class BinaryWORMTreeBuilder {
public:
  using PathType = PathT;
  using BinaryWORMNodeType = BinaryWORMNodeT;
  static constexpr std::size_t OffsetSize = BinaryWORMNodeT::OffsetSize;
  using WriteValueType = typename BinaryWORMNodeType::WriteValueType;
  using OffsetType = typename BinaryWORMNodeT::OffsetType;
  using HasChild = std::array<bool,2>;
  using ValueType = typename BinaryWORMNodeT::ValueType;
  using EdgeType = typename BinaryWORMNodeT::EdgeType;
  using Buffer = BufferT;
  using TreeStats = TreeNodeStats<BinaryWORMNodeT>;

  BinaryWORMTreeBuilder() = default;
  /**
   * \brief Constructor providing a write value object to the underlying node.
   * 
   * The underlying WORM node writing class as an associated value writing object.
   * For the most part the default-constructed version of this class will likely
   * be sufficient, but if you need to override it with a specific writer that
   * maintains some extra state then you can use this constructor.
   */
  BinaryWORMTreeBuilder(const WriteValueType& wv,bool rejectEmptyLeaf = false)
    : rejectEmptyLeaf_(rejectEmptyLeaf), writeValue_(wv) {}
  /**
   * \brief Provide a buffer manager as well as an optional value writer.
   */
  BinaryWORMTreeBuilder(Buffer&& mb,bool rejectEmptyLeaf = false,const WriteValueType& wv = WriteValueType{})
    : rejectEmptyLeaf_(rejectEmptyLeaf), buffer_(std::move<Buffer>(mb)), writeValue_(wv) {}

  virtual ~BinaryWORMTreeBuilder() = default;
  
  /**
   * \brief Begin construction of a binary WORM tree, use buffer, optionally stats only.
   */
  inline bool start(Buffer&& buffer,bool statsOnly = false);
  /**
   * \brief Begin construction of a binary WORM tree, optionally stats only.
   */
  inline bool start(bool statsOnly = false);
  /**
   * \brief Has construction of a tree been started?
   */
  inline bool started() const;
  /**
   * \brief Add a node at a particular path in the tree.
   * 
   * Nodes must be added in pre-order. Any value pointer must be valid until
   * the call returns. Will throw an exception if constraints are violated.
   */
  inline void addNode(const PathType& path,bool hasValue,const ValueType* v,const HasChild& hasChild);

  /**
   * \brief Indicate that the tree is complete.
   * 
   * The builder tracks what added nodes still require children, and if finish
   * is called before all outstanding children have been added then finish will fail.
   */
  inline bool finish();
  /**
   * \brief Did we start and subsequently finish a tree?
   */
  inline bool finished() const;
  /**
   * \brief Return current buffer size.
   */
  std::size_t sizeofBuffer() const { return curSize_; }
  /**
   * \brief Moves the current buffer manager out, clears the internal tree state.
   */
  inline Buffer extractBuffer();
  /**
   * \brief Const access to the current buffer manager.
   */
  const  Buffer& buffer() const { return buffer_; };
  /**
   * \brief Statistics for whatever tree is under construction.
   */
  const TreeStats& treeStats() const { return treeNodeStats_; } 

private:
  bool started_{false};
  bool finished_{false};
  bool statsOnly_{false};
  /**
   * \brief If true then throw exception if empty leaf node is added to tree.
   *
   * An empty leaf node means that we've got at least one branch of the tree
   * that contributes no value whatsoever to tree lookups, just adds overhead.
   * During building we can't silently ignore them without rewinding the write
   * process, which is more complexity than seems worth it.
   */
  bool rejectEmptyLeaf_{false};
  std::size_t curSize_{0};
  Buffer buffer_{};
  WriteValueType writeValue_{};
  TreeNodeStats<BinaryWORMNodeType> treeNodeStats_{};

  struct NodeWritten {
    BinaryWORMNodeType node;
    PathType path{};
    std::size_t chainStartsAt{0};
    std::size_t nodeWrittenAt{0};    
    std::size_t needsChild{0};
    // Track all of the node stats written up to (not including) the written node.
    BinaryWORMNodeStatsTotal<BinaryWORMNodeType> nodeStatsTotal{};
  };
  std::vector<NodeWritten> nodesWritten_{};

  struct HasChildren {
    bool noChildren{false};
    bool oneChild{false};
    bool bothChildren{false};
    HasChildren() = default;
    HasChildren(const BinaryWORMNodeType& n)
      : noChildren(!(n.hasChild(0) || n.hasChild(1)))
      , oneChild((n.hasChild(0) || n.hasChild(1)) && !(n.hasChild(0) && n.hasChild(1)))
      , bothChildren(n.hasChild(0) && n.hasChild(1)) 
    {}
    HasChildren(const HasChild& hc)
      : noChildren(!(hc.at(0) || hc.at(1)))
      , oneChild((hc.at(0) || hc.at(1)) && !(hc.at(0) && hc.at(1)))
      , bothChildren(hc.at(0) && hc.at(1))
    {}

  };

  inline PathType pathFromParent(const NodeWritten& parent,const PathT& path) const;

  inline std::vector<BinaryWORMNodeType>
  buildHeadersFromConnectingPath(const PathType& connectingPath,const BinaryWORMNodeType& newNode);

  struct NodeWriteOpStats {
    BinaryWORMNodeStatsTotal<BinaryWORMNodeType> total{};
    std::size_t chainStartsAt{0};
    std::size_t nodeWrittenAt{0};
  };

  // A node chain is a series of single child nodes without values followed by a terminating node
  // which may have 0, 1, or 2 children and an optional value. The initial single-child nodes
  // effectively form a single "edge" connecting the terminating node with the rest of the tree.
  inline NodeWriteOpStats
  writeNodeChain(const std::vector<BinaryWORMNodeType>& nodeChain);

  // Adds the node at the end of the node chain input vector as a written node.
  inline void
  addWrittenNodeChain(const PathType& p, const std::vector<BinaryWORMNodeType>& n,const NodeWriteOpStats& nw);
};

/**
 * \brief Convenience typedef - a std::vector may be used directly as a buffer manager.
 */
template <typename PathT,typename BinaryWORMNodeT>
using BinaryWORMTreeBuilderVector = BinaryWORMTreeBuilder<std::vector<uint8_t>,PathT,BinaryWORMNodeT>;

/**
 * \brief Example buffer manager using malloc.
 */
class MallocBufferManagerRW {
public:
  MallocBufferManagerRW() = default;
  MallocBufferManagerRW(const MallocBufferManagerRW& o) = delete;
  MallocBufferManagerRW(MallocBufferManagerRW&& o) : buffer_(o.buffer_) { free(o.buffer_); o.buffer_ = nullptr; }
  MallocBufferManagerRW& operator=(const MallocBufferManagerRW& o) = delete;
  MallocBufferManagerRW& operator=(MallocBufferManagerRW&& o) { free(buffer_); buffer_ = nullptr; std::swap(buffer_,o.buffer_); return *this; }
  virtual ~MallocBufferManagerRW() { free(buffer_); buffer_ = nullptr; }
  uint8_t* data() { return buffer_; }
  const uint8_t* data() const { return buffer_; }
  uint8_t* extractBuffer() { uint8_t* b{nullptr}; std::swap(b,buffer_); return b; }
  void resize(std::size_t s) {
    uint8_t* nb = static_cast<uint8_t*>(realloc(buffer_,s));
    if (nb == nullptr) { throw std::runtime_error("MallocBufferManagerRW: resize failed"); }
    buffer_ = nb;
  }
private:
  uint8_t* buffer_{nullptr};
};

/////////////////////
// IMPLEMENTATIONS //
/////////////////////

template <typename BufferT,typename PathT,typename BinaryWORMNodeT>
bool
BinaryWORMTreeBuilder<BufferT,PathT,BinaryWORMNodeT>::start(BufferT&& buffer,bool statsOnly) {
  buffer_ = std::move(buffer);
  return start(statsOnly);
}

template <typename BufferT,typename PathT,typename BinaryWORMNodeT>
bool
BinaryWORMTreeBuilder<BufferT,PathT,BinaryWORMNodeT>::start(bool statsOnly) {
  if (started_ && !finished_) { return false; }
  curSize_ = 0;
  statsOnly_ = statsOnly;
  treeNodeStats_ = TreeNodeStats<BinaryWORMNodeType>{};
  started_ = true;
  finished_ = false;
  return true;
}

template <typename BufferT,typename PathT,typename BinaryWORMNodeT>
bool
BinaryWORMTreeBuilder<BufferT,PathT,BinaryWORMNodeT>::started() const {
  return (started_ && !finished_);
}

template <typename BufferT,typename PathT,typename BinaryWORMNodeT>
void
BinaryWORMTreeBuilder<BufferT,PathT,BinaryWORMNodeT>::addNode(const PathType& path,bool hasValue,const ValueType* v,const HasChild& hasChild) {
  const HasChildren has(hasChild);
  const bool isRoot = (path.empty());
  // An empty leaf node has no children and no value set.
  // This sort of node is useless, but if someone has been
  // a bit sloppy with tree construction they might show up.
  bool isEmptyLeaf = (has.noChildren && !hasValue);
  // Nodes that don't have a value and don't have both children
  // are basically "scaffolding" nodes, i.e. those that get added
  // when the maximum edge size of a single node is too small to
  // cover the entire desired edge. We'll add our own scaffolding
  // nodes later on as required by the node type in use.
  bool isScaffolding = !(hasValue || has.bothChildren);

  if (rejectEmptyLeaf_ && isEmptyLeaf && !isRoot) {
    throw std::runtime_error("BinaryWORMTreeBuilder: attempt to add empty leaf node to non-empty tree");
  }

  if (nodesWritten_.empty()) {
    if (!(started_ && !finished_ && (treeNodeStats_.allNodeStats.bytes() == 0))) {
      throw std::runtime_error("BinaryWORMTreeBuilder: attempt to add child node without available parents");
    }
    BinaryWORMNodeType newRoot{};
    // If the first node in is at the root then we just add it.
    // Otherwise we have to add a root node compatible with what's being
    // added before moving on.
    if (isRoot) {
      newRoot.setHasChild(hasChild);
      newRoot.setHasValue(hasValue);
      if (hasValue) { newRoot.setValue(v); }
    } else {
      newRoot.setHasChild(path.at(0),true);
    }
    std::vector<BinaryWORMNodeType> rootNodeChain;
    rootNodeChain.push_back(std::move(newRoot));
    NodeWriteOpStats rootWritten = writeNodeChain(rootNodeChain);
    addWrittenNodeChain(PathType{},rootNodeChain,rootWritten);

    // if the incoming node is the root then we're done, otherwise
    // continue on and do a regular write of the incoming node
    if (isRoot) { return; }
  }

  // Quietly ignore scaffolding nodes, let empty leaf nodes through
  // if we aren't rejecting them.
  if (isScaffolding && !(isEmptyLeaf && !rejectEmptyLeaf_)) {
    return;
  }
  NodeWritten& parent(nodesWritten_.back());
  PathType connectingPath = pathFromParent(parent,path);
  // Now need to walk the connecting path and string together
  // a sequence of nodes that covers the whole connector.
  BinaryWORMNodeType newNode{writeValue_};
  newNode.setHasValue(hasValue);
  if (hasValue) { newNode.setValue(v); }
  newNode.setHasChild(hasChild);
  std::vector<BinaryWORMNodeType> connectingNodes =
    buildHeadersFromConnectingPath(connectingPath,newNode);
  NodeWriteOpStats nodeWritten = writeNodeChain(connectingNodes);
  addWrittenNodeChain(path,connectingNodes,nodeWritten);
}

template <typename BufferT,typename PathT,typename BinaryWORMNodeT>
bool
BinaryWORMTreeBuilder<BufferT,PathT,BinaryWORMNodeT>::finish() {
  // If we're already finished then finish is a NOP.
  if (finished_) { return true; }
  // If we never started we can't finish.
  if (!started_) { return false; }
  // If we've got any nodes waiting for children then we can't finish yet.
  if (!nodesWritten_.empty()) { return false; }
  // Have we written anything? Can't finish until we've written something.
  if (curSize_ == 0) { return false; }
  finished_ = true;
  return true;
}

template <typename BufferT,typename PathT,typename BinaryWORMNodeT>
bool
BinaryWORMTreeBuilder<BufferT,PathT,BinaryWORMNodeT>::finished() const {
  return ((started_ == true) && (finished_ == true));
}

// remove the current buffer manager from the tree builder
template <typename BufferT,typename PathT,typename BinaryWORMNodeT>
BufferT
BinaryWORMTreeBuilder<BufferT,PathT,BinaryWORMNodeT>::extractBuffer() {
  curSize_ = 0;
  statsOnly_ = false;
  treeNodeStats_ = TreeNodeStats<BinaryWORMNodeType>{};
  started_ = false;
  finished_ = false;
  nodesWritten_.clear();
  return std::move(buffer_);
}


template <typename BufferT,typename PathT,typename BinaryWORMNodeT>
PathT
BinaryWORMTreeBuilder<BufferT,PathT,BinaryWORMNodeT>::pathFromParent(const NodeWritten& parent,const PathType& path) const {
  if (parent.path.size() >= path.size()) {
    throw std::runtime_error("BinaryWORMTreeBuilder: new node path shorter than parent node");
  }

  for (std::size_t c = 0;c < parent.path.size(); ++c) {
    if (parent.path.at(c) != path.at(c)) {
      throw std::runtime_error("BinaryWORMTreeBuilder: new node path doesn't cover parent");
    }
  }

  if (path.at(parent.path.size()) != parent.needsChild) {
    throw std::runtime_error("BinaryWORMTreeBuilder: new node path for incorrect child");
  }
  PathType connectingPath{};
  for (std::size_t c = parent.path.size();c < path.size(); ++c) {
    connectingPath.push_back(path.at(c));
  }
  return connectingPath;
}

template <typename BufferT,typename PathT,typename BinaryWORMNodeT>
std::vector<typename BinaryWORMTreeBuilder<BufferT,PathT,BinaryWORMNodeT>::BinaryWORMNodeType>
BinaryWORMTreeBuilder<BufferT,PathT,BinaryWORMNodeT>::buildHeadersFromConnectingPath(const PathType& connectingPath,const BinaryWORMNodeType& newNode)
{
  // Walk the connecting path, place whatever extension nodes we might need
  // to reach newNode along connectingPath.
  std::vector<BinaryWORMNodeType> connectingNodes{};
  // We always need at least one node in our chain.
  connectingNodes.emplace_back();
  // We get the first step in the path as part of the tree node topology,
  // so start at offset 1 in the path.
  for (std::size_t i=1;i<connectingPath.size();++i) {
    if (connectingNodes.back().edge().full()) {
      // If our edge is full then absorb the current connecting
      // step into our child and add a new node in the chain.
      connectingNodes.back().setHasChild(connectingPath.at(i),true);
      connectingNodes.emplace_back();
    } else {
      // If we have room in our edge then keep adding to it.
      connectingNodes.back().edge().push_back(connectingPath.at(i));
    }
  }
  // Now configure our final node in the chain to match the children/value status
  // of what was passed in, keeping the edge we computed to get us to the
  // right place in the tree.
  EdgeType backEdge = connectingNodes.back().edge();
  connectingNodes.back() = newNode;
  connectingNodes.back().edge() = backEdge;
  return connectingNodes;
}

// A node chain is a series of single child nodes without values followed by a terminating node
// which may have 0, 1, or 2 children and an optional value. The initial single-child nodes
// effectively form a single "edge" connecting the terminating node with the rest of the tree.
template <typename BufferT,typename PathT,typename BinaryWORMNodeT>
typename BinaryWORMTreeBuilder<BufferT,PathT,BinaryWORMNodeT>::NodeWriteOpStats
BinaryWORMTreeBuilder<BufferT,PathT,BinaryWORMNodeT>::writeNodeChain(const std::vector<BinaryWORMNodeT>& nodeChain)
{
  NodeWriteOpStats stats{};
  if (nodeChain.empty()) { return stats; }
  for (std::size_t i = 0;i < (nodeChain.size() - 1); ++i) {
    HasChildren curNodeHas(nodeChain[i]);
    if (nodeChain[i].hasValue() || curNodeHas.noChildren || curNodeHas.bothChildren) {
      throw std::runtime_error("BinaryWORMTreeBuilder: invalid chain node - has a value and/or child count != 1");
    }
    stats.total.addNode(nodeChain[i]);
  }
  stats.total.addNode(nodeChain.back());
  std::size_t totalAddedBytes = stats.total.bytes();
  std::size_t startAt = curSize_;
  std::size_t newSize = curSize_ + totalAddedBytes;
  curSize_ = newSize;
  stats.chainStartsAt = startAt;
  stats.nodeWrittenAt = (newSize - nodeChain.back().size());
  if (!statsOnly_) {
    buffer_.resize(newSize);
    std::size_t writeAt = startAt;
    std::size_t lastNodeWrittenAt = startAt;
    for (const auto& curNode : nodeChain) {
      std::size_t curWrittenSize = curNode.write(buffer_.data() + writeAt);
      if (curWrittenSize == 0) {
        throw std::runtime_error("BinaryWORMTreeBuilder: empty node");
      }
      lastNodeWrittenAt = writeAt;
      writeAt += curWrittenSize;
    }
    if (writeAt != curSize_) {
      throw std::runtime_error("BinaryWORMTreeBuilder: actual write size different from expected");
    }
    if (stats.nodeWrittenAt != lastNodeWrittenAt) {
      throw std::runtime_error("BinaryWORMTreeBuilder: last node written at position different from expected");
    }
  }
  return stats;
}

template <typename BufferT,typename PathT,typename BinaryWORMNodeT>
void
BinaryWORMTreeBuilder<BufferT,PathT,BinaryWORMNodeT>::
  addWrittenNodeChain(const PathT& p,
                      const std::vector<BinaryWORMNodeT>& n,
                      const NodeWriteOpStats& nw)
{
  bool parentFinished{false};  
  if (!nodesWritten_.empty()) {
    NodeWritten& parent(nodesWritten_.back());
    HasChildren parentHas(parent.node);
    // If the parent has both children and this was the left child, then move on
    // to the right child. Otherwise we must have added all children required.
    if (parentHas.bothChildren) {
      if (parent.needsChild == 0) {
        parent.needsChild = 1;
      } else {
        for (std::size_t i = 0; i <= TreeNodeStats<BinaryWORMNodeType>::MaxOffsetBytes; ++i) {
          std::size_t curOffsetGap = parent.nodeStatsTotal.bytes(i);
          if (curOffsetGap > treeNodeStats_.longestOffsetGap.at(i)) {
            treeNodeStats_.longestOffsetGap.at(i) = curOffsetGap;
          }
        }
        std::size_t offsetToUse = (nw.chainStartsAt - parent.nodeWrittenAt);
        OffsetType rightNodeOffset = static_cast<OffsetType>(offsetToUse);
        if (static_cast<std::size_t>(rightNodeOffset) != offsetToUse) {
          throw std::runtime_error("BinaryWORMTreeBuilder: exceeded offset capacity");
        }
        parent.node.setRightChildOffset(rightNodeOffset);
        if (!statsOnly_) { parent.node.write(buffer_.data() + parent.nodeWrittenAt); }
        parentFinished = true;
      }
    } else {
      parentFinished = true;
    }
  } else {
    if (!p.empty()) { throw std::runtime_error("BinaryWORMTreeBuilder: attempt to add non-root to empty tree"); }
  }

  if (parentFinished) {
    BinaryWORMNodeStatsTotal<BinaryWORMNodeType> parentNodeStatsTotal = nodesWritten_.back().nodeStatsTotal;
    nodesWritten_.pop_back();
    // Empty nodesWritten_ stack implies no nodes prior care about tracking
    // our offset.
    if (!nodesWritten_.empty()) { nodesWritten_.back().nodeStatsTotal += parentNodeStatsTotal;}
  }

  const HasChildren newHas(n.back());
  if (newHas.oneChild || newHas.bothChildren) {
    NodeWritten newNodeWritten{};
    newNodeWritten.node = n.back();
    newNodeWritten.path = p;
    newNodeWritten.chainStartsAt = nw.chainStartsAt;
    newNodeWritten.nodeWrittenAt = nw.nodeWrittenAt;
    if (newHas.bothChildren || newNodeWritten.node.hasChild(0)) {
      newNodeWritten.needsChild = 0;
    } else {
      newNodeWritten.needsChild = 1;
    }
    newNodeWritten.nodeStatsTotal = nw.total;
    nodesWritten_.push_back(newNodeWritten);
  } else {
    // This is a terminal node so it won't go on the stack,
    // but we still need to record what got written for it.
    // As if we'd pushed the terminal node onto the stack,
    // then immediately popped it off again and propagated
    // its stats back up to its parent (if there's a parent that cares).
    if (!nodesWritten_.empty()) { nodesWritten_.back().nodeStatsTotal += nw.total; }
  }
  treeNodeStats_.allNodeStats += nw.total;
  if (treeNodeStats_.allNodeStats.bytes() != curSize_) {
    throw std::runtime_error("BinaryWORMTreeBuilder: mismatch between expected and actual size");
  }
}

template <typename BinaryWORMNodeT>
void
BinaryWORMNodeStatsTotal<BinaryWORMNodeT>::addNode(const BinaryWORMNodeT& nh)
{
  if (nh.hasValue()) { ++values.count; values.bytes += nh.valueSize(); }
  BinaryWORMNodeStats* nodeStats = &headersNoChildren;
  if (nh.hasChild(0) && nh.hasChild(1)) { nodeStats = &headersTwoChildren; }
  else if (nh.hasChild(0) || nh.hasChild(1)) { nodeStats = &headersSingleChild; }
  ++(nodeStats->count);
  nodeStats->bytes += nh.headerSize();
}

template <typename BinaryWORMNodeT>  
std::size_t
BinaryWORMNodeStatsTotal<BinaryWORMNodeT>::bytes() const
{
  return (values.bytes + headersNoChildren.bytes +
         headersSingleChild.bytes + headersTwoChildren.bytes);
}

template <typename BinaryWORMNodeT>  
std::size_t
BinaryWORMNodeStatsTotal<BinaryWORMNodeT>::bytes(std::size_t offsetSize) const
{
  std::size_t baseline = bytes();
  if (offsetSize == OffsetSize) { return baseline; }
  const bool targetBytesSmaller = (offsetSize < OffsetSize);
  const std::size_t offsetDiff = (targetBytesSmaller ? (OffsetSize - offsetSize) : (offsetSize - OffsetSize));
  const std::size_t offsetBytesDiff = (offsetDiff * headersTwoChildren.count);
  return (targetBytesSmaller ? (baseline - offsetBytesDiff) : (baseline + offsetBytesDiff));
}

} // namespace RadixTree
} // namespace Mapper
} // namespae Akamai
#endif