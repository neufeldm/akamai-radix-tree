#ifndef AKAMAI_MAPPER_RADIX_TREE_BINARY_WORM_CURSOR_RO_H_
#define AKAMAI_MAPPER_RADIX_TREE_BINARY_WORM_CURSOR_RO_H_

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
#include <cstddef>

#include "BinaryWORMNode.h"

namespace Akamai {
namespace Mapper {
namespace RadixTree {

/**
 * \brief Read-only cursor for walking binary WORM trees, full navigation allowed.
 */
template <typename PathT,typename BinaryWORMNodeT,template<typename,std::size_t> class NodeStackT>
class BinaryWORMCursorRO {
public:
  static constexpr std::size_t Radix = 2;
  static constexpr std::size_t MaxDepth = PathT::MaxDepth;
  using PathType = PathT;
  using ValueType = typename BinaryWORMNodeT::ValueType;
  using NodeValueRO = BinaryWORMValueCopyRO<ValueType>;
  using NodeValue = NodeValueRO;
  using BinaryWORMNodeType = BinaryWORMNodeT;
  using OffsetType = typename BinaryWORMNodeT::OffsetType;
  using EdgeWordType = typename BinaryWORMNodeT::EdgeWordType;

  BinaryWORMCursorRO(const uint8_t* rootPtr = nullptr) {
    NodePos rootPos{rootPtr};
    if (rootPtr != nullptr) {
      BinaryWORMNodeType rootNode{rootPtr};
      if (rootNode.hasValue()) {
        rootPos.coveringValueNode = rootPtr;
        rootPos.coveringValueNodeDepth = 0;
      }
    }
    nodeStack_.push_back(std::move(rootPos));
  }

  const PathType& getPath() const { return curPath_; }
  bool atNode() const { return (nodeStack_.back().depthBelow == 0); }
  bool atLeafNode() const { return atNode() && backNode().isLeaf(); }
  bool atValue() const { return (atNode() && backNode().hasValue()); }
  inline bool goChild(std::size_t child);
  bool canGoChild(std::size_t /* child */) const { return (curPath_.size() != MaxDepth); }
  inline bool canGoChildNode(std::size_t child) const;
 
  inline bool goParent();
  bool canGoParent() const { return (curPath_.size() > 0); }

  NodeValueRO coveringNodeValueRO() const {
    if (nodeStack_.back().coveringValueNode == nullptr) { return NodeValueRO{}; }
    ValueType v{};
    coveringValueNode().readValue(&v);
    return NodeValueRO{std::move(v)};
  }
  std::size_t coveringNodeValueDepth() const { return nodeStack_.back().coveringValueNodeDepth; }
  NodeValueRO nodeValueRO() const {
    if (atValue()) {
      ValueType v{};
      backNode().readValue(&v);
      return NodeValueRO{std::move(v)};
    }
    return NodeValueRO{};
  }
  NodeValue nodeValue() const { return nodeValueRO(); }  

private:
  using Node = BinaryWORMNodeType;
  PathType curPath_{};
  struct NodePos {
    // Reference to node at/above current position
    const uint8_t* nodeAtAbove{nullptr};
    // Steps below nodeRefAtAbove
    std::size_t depthBelow{0};
    // A node edge representing the rest of the steps below nodeRefAtAbove (if any)
    EdgeWordType edgeToBelow{0};
    std::size_t edgeStepsRemaining{0};
    static std::size_t firstEdgeStep(EdgeWordType etb) { return (etb >> (8*sizeof(EdgeWordType) - 1)); }
    std::size_t firstEdgeStep() const { return firstEdgeStep(edgeToBelow); }
    void trimFirstEdgeStep() {
      if (edgeStepsRemaining > 0) {
        edgeToBelow = (edgeToBelow << 1);
        --edgeStepsRemaining;
      }
    }
    void clearEdge() { edgeStepsRemaining = 0; edgeToBelow = 0; }
    // Reference to node below the current position (if any)
    const uint8_t* nodeBelow{nullptr};

    const uint8_t* coveringValueNode{nullptr};
    std::size_t coveringValueNodeDepth{0};

    NodePos() = default;
    NodePos(const uint8_t* np) : nodeAtAbove(np) {}
  };
  using NodeStack =  NodeStackT<NodePos,MaxDepth+1>;
  BinaryWORMNodeType backNode() const { return BinaryWORMNodeType{nodeStack_.back().nodeAtAbove}; }
  BinaryWORMNodeType coveringValueNode() const { return BinaryWORMNodeType{nodeStack_.back().coveringValueNode}; }
  // Keep a stack of nodes around
  NodeStack nodeStack_{};
};

/**
 * \brief Lookup-only RO cursor for binary WORM trees.
 */
template <typename PathT,typename BinaryWORMNodeT>
class BinaryWORMLookupCursorRO
{
public:
  static constexpr std::size_t Radix = 2;
  static constexpr std::size_t MaxDepth = PathT::MaxDepth;
  using PathType = PathT;
  using ValueType = typename BinaryWORMNodeT::ValueType;
  using NodeValueRO = BinaryWORMValueCopyRO<ValueType>;
  using NodeValue = NodeValueRO;
  using BinaryWORMNodeType = BinaryWORMNodeT;
  using OffsetType = typename BinaryWORMNodeT::OffsetType;
  using EdgeWordType = typename BinaryWORMNodeT::EdgeWordType;

  BinaryWORMLookupCursorRO() = default;
  BinaryWORMLookupCursorRO(const uint8_t* rootPtr)
    : nodeAtAbove_(rootPtr)
  {
    if (rootPtr == nullptr) { return; }
    if (BinaryWORMNodeType{rootPtr}.hasValue()) {
      coveringValueNode_ = rootPtr;
      coveringValueNodeDepth_ = 0;
    }
  }

  // Interface methods start here
  
  const PathType& getPath() const { return curPath_; }
  
  bool atNode() const { return (depthBelow_ == 0); }
  bool atValue() const { return (atNode() && coveringNode().hasValue()); }
  inline bool goChild(std::size_t child);

  bool canGoChild(std::size_t) const { return (curPath_.size() < MaxDepth); }
 
  bool canGoChildNode(std::size_t child) const {
    if (depthBelow_ == 0) { return (coveringNode().getChild(child) != nullptr); }
    if (nodeBelow_ == nullptr) { return false; }
    return (firstEdgeStep() == child);
  }
  bool hasChildNode(std::size_t child) const { return canGoChildNode(child); }
 
  bool goParent() { throw std::runtime_error("BinaryWORMLookupCursorRO: can't return"); return false; }
  bool canGoParent() const { return false; }

  inline NodeValue coveringNodeValueRO() const;
  std::size_t coveringNodeValueDepth() const { return coveringValueNodeDepth_; }
  inline NodeValue nodeValue() const;
  NodeValue nodeValueRO() const { return nodeValue(); }

  bool atLeafNode() const { return (atNode() && !(hasChildNode(0) || hasChildNode(1))); }

private:
  using Node = BinaryWORMNodeType;
  // Reference to node at/above current position
  const uint8_t* nodeAtAbove_{nullptr};
  // Steps below nodeRefAtAbove
  std::size_t depthBelow_{0};
  // A node edge representing the rest of the steps below nodeRefAtAbove (if any)
  EdgeWordType edgeToBelow_{0};
  std::size_t edgeStepsRemaining_{0};
  static std::size_t firstEdgeStep(EdgeWordType etb) { return (etb >> (8*sizeof(EdgeWordType) - 1)); }
  std::size_t firstEdgeStep() const { return firstEdgeStep(edgeToBelow_); }
  void trimFirstEdgeStep() {
    if (edgeStepsRemaining_ > 0) {
      edgeToBelow_ = (edgeToBelow_ << 1);
      --edgeStepsRemaining_;
    }
  }
  void clearEdge() { edgeStepsRemaining_ = 0; edgeToBelow_ = 0; }
  // Reference to node below the current position (if any)
  const uint8_t* nodeBelow_{nullptr};
  bool edgeEmpty() const { return (edgeStepsRemaining_ == 0); }
  const uint8_t* coveringValueNode_{nullptr};
  std::size_t coveringValueNodeDepth_{0};

  BinaryWORMNodeType coveringNode() const { return BinaryWORMNodeType{nodeAtAbove_}; }
  BinaryWORMNodeType coveringValueNode() const { return BinaryWORMNodeType{coveringValueNode_}; }

  // Keep our current position in the tree
  PathType curPath_;
};

//////////////////////////////////////////
// IMPLEMENTATIONS - BinaryWORMCursorRO //
//////////////////////////////////////////

template <typename PathT,typename BinaryWORMNodeT,template<typename,std::size_t> class NodeStackT>
bool BinaryWORMCursorRO<PathT,BinaryWORMNodeT,NodeStackT>::goChild(std::size_t child) {
  // A bit more complicated than you might think because we have
  // to track nonexistent positions in the tree.
  if (!canGoChild(child)) { return false; }

  NodePos newNodePos = nodeStack_.back();
  if (newNodePos.depthBelow == 0) {
    // We're at a node, drop down into the immediate child
    newNodePos.nodeBelow = Node{newNodePos.nodeAtAbove}.getChild(child);
    if (newNodePos.nodeBelow != nullptr) {
     Node nodeBelow{newNodePos.nodeBelow};
     newNodePos.edgeToBelow = nodeBelow.edgeBitsAsWord();
     newNodePos.edgeStepsRemaining = nodeBelow.edgeStepCount();
    }
  } else if (newNodePos.nodeBelow != nullptr) {
    // Check to see if we match the next step in the edge down to
    // the node below. If so, trim it off. If not, then reset the edge
    // as well as the node below ref - we've gone outside the edge.
    if (child == newNodePos.firstEdgeStep()) {
      newNodePos.trimFirstEdgeStep();
    } else {
      newNodePos.clearEdge();
      newNodePos.nodeBelow = nullptr;
    }
  }
  newNodePos.depthBelow++;
  // See if we've arrived at the node below (if there is one).
  if ((newNodePos.nodeBelow != nullptr) && (newNodePos.edgeStepsRemaining == 0)) {
    newNodePos.nodeAtAbove = newNodePos.nodeBelow;
    newNodePos.nodeBelow = nullptr;
    newNodePos.depthBelow = 0;
    BinaryWORMNodeType atNode{newNodePos.nodeAtAbove};
    if (atNode.hasValue()) {
      newNodePos.coveringValueNode = newNodePos.nodeAtAbove;
      newNodePos.coveringValueNodeDepth = curPath_.size() + 1;
    }
  }
  nodeStack_.push_back(std::move(newNodePos));
  curPath_.push_back(child);

  return true;
}

template <typename PathT,typename BinaryWORMNodeT,template<typename,std::size_t> class NodeStackT>
bool BinaryWORMCursorRO<PathT,BinaryWORMNodeT,NodeStackT>::goParent() {
  if (curPath_.size() == 0) return false;
  nodeStack_.pop_back();
  curPath_.pop_back();

  return true;
}

template <typename PathT,typename BinaryWORMNodeT,template<typename,std::size_t> class NodeStackT>
bool
BinaryWORMCursorRO<PathT,BinaryWORMNodeT,NodeStackT>::canGoChildNode(std::size_t child) const {
  if (!canGoChild(child)) { return false; }
  std::size_t depthBelow = nodeStack_.back().depthBelow;
  Node nodeAbove{nodeStack_.back().nodeAtAbove};
  if (depthBelow == 0) { return (nodeAbove.getChild(child) != nullptr); }
  const uint8_t* nodeBelow = nodeStack_.back().nodeBelow;
  if (nodeBelow == nullptr) { return false; }
  return (nodeStack_.back().firstEdgeStep() == child);
}

////////////////////////////////////////////////
// IMPLEMENTATIONS - BinaryWORMLookupCursorRO //
////////////////////////////////////////////////
template <typename PathT,typename BinaryWORMNodeT>
bool BinaryWORMLookupCursorRO<PathT,BinaryWORMNodeT>::goChild(std::size_t child) {
  if (!canGoChild(child)) { return false; }
  if (depthBelow_ == 0) {
    // We're at a node, drop down into the immediate child
    nodeBelow_ = Node{nodeAtAbove_}.getChild(child);
    if (nodeBelow_ != nullptr) {
      Node nodeBelow{nodeBelow_};
      edgeToBelow_ = nodeBelow.edgeBitsAsWord();
      edgeStepsRemaining_ = nodeBelow.edgeStepCount();
    }
  } else if (nodeBelow_ != nullptr) {
    // Check to see if we match the next step in the edge down to
    // the node below. If so, trim it off. If not, then reset the edge
    // as well as the node below ref - we've gone outside the edge.
    if (child == firstEdgeStep()) {
      trimFirstEdgeStep();
    } else {
      clearEdge();
      nodeBelow_ = nullptr;
    }
  }
  ++depthBelow_;
  // See if we've arrived at the node below (if there is one).
  if ((nodeBelow_ != nullptr) && edgeEmpty()) {
    nodeAtAbove_ = nodeBelow_;
    nodeBelow_ = nullptr;
    depthBelow_ = 0;
    Node nodeAtAbove{nodeAtAbove_};
    if (Node{nodeAtAbove_}.hasValue()) {
      coveringValueNode_ = nodeAtAbove_;
      coveringValueNodeDepth_ = curPath_.size() + 1;
    }
  }
  curPath_.push_back(child);
  return true;
}

template <typename PathT,typename BinaryWORMNodeT>
typename BinaryWORMLookupCursorRO<PathT,BinaryWORMNodeT>::NodeValue
BinaryWORMLookupCursorRO<PathT,BinaryWORMNodeT>::coveringNodeValueRO() const {
  if (coveringValueNode_ == nullptr) { return NodeValue{}; }
  Node coveringValueNode{coveringValueNode_};
  ValueType v{};
  coveringValueNode.readValue(&v);
  return NodeValue{std::move(v)};;
}

template <typename PathT,typename BinaryWORMNodeT>
typename BinaryWORMLookupCursorRO<PathT,BinaryWORMNodeT>::NodeValue
BinaryWORMLookupCursorRO<PathT,BinaryWORMNodeT>::nodeValue() const {
  Node nodeAtAbove{nodeAtAbove_};
  if (nodeAtAbove.hasValue() && (depthBelow_ == 0)) {
    ValueType v{};
    nodeAtAbove.readValue(&v);
    return NodeValueRO{std::move(v)};
  }
  return NodeValueRO{};
}

} // namespace RadixTree
} // namespace Mapper
} // namespace Akamai

#endif
