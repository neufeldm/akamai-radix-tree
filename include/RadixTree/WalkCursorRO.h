#ifndef AKAMAI_MAPPER_RADIX_TREE_WALK_CURSOR_RO_H_
#define AKAMAI_MAPPER_RADIX_TREE_WALK_CURSOR_RO_H_

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
#include <limits>

#include "NodeValue.h"

namespace Akamai {
namespace Mapper {
namespace RadixTree {

/**
 * \brief Read-only cursor implementation, somewhat faster than the regular CursorRO.
 * 
 * The WalkCursorRO class is an alternative to CursorRO that has shown some small performance
 * increases when traversing trees. It caches more state information from the nodes,
 * so has weaker consistency guarantees when simultaneously reading and writing to a
 * tree.
 */
template <std::size_t R,std::size_t MD,typename AllocatorT,typename NodeT,typename PathT,template<typename,std::size_t> class NodeStackT>
class WalkCursorRO
{
public:
  static constexpr std::size_t Radix = R;
  static constexpr std::size_t MaxDepth = MD;
  using Node = NodeT;
  using NodeImpl = typename Node::NodeImplType;
  using NodeImplRef = typename Node::NodeImplRefType;
  using ValueType = typename Node::ValueType;
  using NodeValue = Mapper::RadixTree::NodeValueRO<Node>;
  using NodeValueRO = Mapper::RadixTree::NodeValueRO<Node>;
  using Allocator = AllocatorT;
  using PathType = PathT;
  using NodeEdge = typename NodeT::Edge;
  static constexpr std::size_t NoChild = Node::NoChild;

  WalkCursorRO(const Allocator& a, NodeImplRef root)
    : alloc_(&a)
  {
    NodePos rootPos(root);
    if (root != Allocator::nullRef) {
      Node rootNode{alloc_,root};
      if (rootNode.hasValue()) {
        rootPos.coveringValueNodeRef = root;
        rootPos.coveringValueNodeDepth = 0;
      }
    }
    nodeStack_.push_back(rootPos);
  }
  WalkCursorRO(const WalkCursorRO& other) = default;
  WalkCursorRO(WalkCursorRO&& other) = default;
  WalkCursorRO() = default;
  WalkCursorRO& operator=(const WalkCursorRO& other) = default;
  WalkCursorRO& operator=(WalkCursorRO&& other) = default;

  // Interface methods start here

  PathType getPath() const { return curPath_; }
  bool atNode() const { return (nodeStack_.back().depthBelow == 0); }
  bool atLeafNode() const { return atNode() && backNode().isLeaf(); }
  bool atValue() const { return (atNode() && backNode().hasValue()); }
  inline bool goChild(std::size_t child);
  bool canGoChild(std::size_t /*child*/) const { return (curPath_.suffixLength() > 0); }
  inline bool canGoChildNode(std::size_t child) const;
  inline bool goParent();
  bool canGoParent() const { return (curPath_.size() > 0); }
  NodeValue coveringNodeValueRO() const {
    if (nodeStack_.back().coveringValueNodeRef == Allocator::nullRef) { return NodeValue{}; }
    return NodeValue{backValueNode()};
  }
  std::size_t coveringNodeValueDepth() const { return nodeStack_.back().coveringValueNodeDepth; }
  NodeValue nodeValue() const { return (atNode() ? NodeValue{backNode()} : NodeValue{}); }
  NodeValue nodeValueRO() const { return nodeValue(); }

private:
  const Allocator* alloc_{nullptr};
  // We keep a stack of internal nodes and their depth in the path
  struct NodePos {
    // Reference to node at/above current position
    NodeImplRef nodeRefAtAbove{Allocator::nullRef};
    // Steps below nodeRefAtAbove
    std::size_t depthBelow{0};
    // A node edge representing the rest of the steps below nodeRefAtAbove (if any)
    NodeEdge edgeToBelow{};
    // Reference to node below the current position (if any)
    NodeImplRef nodeRefBelow{Allocator::nullRef};

    // Track the most recent node reference we've seen that
    // has a value.
    NodeImplRef coveringValueNodeRef{Allocator::nullRef};
    std::size_t coveringValueNodeDepth{0};

    NodePos() = default;
    NodePos(NodeImplRef nref) : nodeRefAtAbove(nref) {}
  };
  using NodeStack =  NodeStackT<NodePos,MaxDepth+1>;
  // Keep a stack of nodes around
  NodeStack nodeStack_{};
  Node backNode() const { return Node{alloc_,nodeStack_.back().nodeRefAtAbove}; }
  Node backValueNode() const { return Node{alloc_,nodeStack_.back().coveringValueNodeRef}; }

  // Keep our current position in the tree
  PathType curPath_{};
};

///////////////////////////////////////////////////////////////////////
/////////// IMPLEMENTATION - RadixTree WalkCursorRO ///////////////////
///////////////////////////////////////////////////////////////////////

template <std::size_t R,std::size_t MaxDepth,typename Alloc,typename NodeT,typename PathT,template<typename,std::size_t> class NodeStackT>
bool WalkCursorRO<R,MaxDepth,Alloc,NodeT,PathT,NodeStackT>::goChild(std::size_t child) {
  // A bit more complicated than you might think because we have
  // to track nonexistent positions in the tree.

  if (!canGoChild(child)) return false;

  NodePos newNodePos = nodeStack_.back();
  if (newNodePos.depthBelow == 0) {
    // We're at a node, drop down into the immediate child
    newNodePos.nodeRefBelow = Node{alloc_,newNodePos.nodeRefAtAbove}.getChild(child);
    if (newNodePos.nodeRefBelow != Allocator::nullRef) {
      newNodePos.edgeToBelow = Node{alloc_,newNodePos.nodeRefBelow}.edge();
    }
  } else if (newNodePos.nodeRefBelow != Allocator::nullRef) {
    // Check to see if we match the next step in the edge down to
    // the node below. If so, trim it off. If not, then reset the edge
    // as well as the node below ref - we've gone outside the edge.
    if (child == newNodePos.edgeToBelow.at(0)) {
      newNodePos.edgeToBelow.trim_front(1);
    } else {
      newNodePos.edgeToBelow.clear();
      newNodePos.nodeRefBelow = Allocator::nullRef;
    }
  }
  newNodePos.depthBelow++;
  // See if we've arrived at the node below (if there is one).
  if ((newNodePos.nodeRefBelow != Allocator::nullRef) && newNodePos.edgeToBelow.empty()) {
    newNodePos.nodeRefAtAbove = newNodePos.nodeRefBelow;
    newNodePos.nodeRefBelow = Allocator::nullRef;
    newNodePos.depthBelow = 0;
    Node atNode{alloc_,newNodePos.nodeRefAtAbove};
    if (atNode.hasValue()) {
      newNodePos.coveringValueNodeRef = newNodePos.nodeRefAtAbove;
      newNodePos.coveringValueNodeDepth = curPath_.size() + 1;
    }
  }
  nodeStack_.push_back(std::move(newNodePos));
  curPath_.push_back(child);

  return true;
}

template <std::size_t R,std::size_t MaxDepth,typename Alloc,typename NodeT,typename PathT,template<typename,std::size_t> class NodeStackT>
bool WalkCursorRO<R,MaxDepth,Alloc,NodeT,PathT,NodeStackT>::goParent() {
  if (curPath_.size() == 0) return false;
  nodeStack_.pop_back();
  curPath_.pop_back();

  return true;
}

template <std::size_t R,std::size_t MaxDepth,typename Alloc,typename NodeT,typename PathT,template<typename,std::size_t> class NodeStackT>
bool WalkCursorRO<R,MaxDepth,Alloc,NodeT,PathT,NodeStackT>::canGoChildNode(std::size_t child) const {
  if (!canGoChild(child)) { return false; }
  std::size_t depthBelow = nodeStack_.back().depthBelow;
  Node nodeAbove{alloc_,nodeStack_.back().nodeRefAtAbove};
  if (depthBelow == 0) { return (nodeAbove.getChild(child) != Allocator::nullRef); }
  NodeImplRef nodeRefBelow = nodeStack_.back().nodeRefBelow;
  if (nodeRefBelow == Allocator::nullRef) { return false; }
  return (nodeStack_.back().edgeToBelow.at(0) == child);
}

} //  namespace RadixTree
} //  namespace Mapper
} //  namespace Akamai

#endif
