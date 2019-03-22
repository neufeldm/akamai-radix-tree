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
    nodeStack_.push_back(rootPos);
  }
  WalkCursorRO(const WalkCursorRO& other) = default;
  WalkCursorRO(WalkCursorRO&& other) = default;
  WalkCursorRO() = delete;
  WalkCursorRO& operator=(const WalkCursorRO& other) = default;
  WalkCursorRO& operator=(WalkCursorRO&& other) = default;

  // Interface methods start here

  PathType getPath() const { return curPath_; }
  bool atNode() const { return (nodeStack_.back().depthBelow == 0); }
  bool atValue() const { return (atNode() && backNode().hasValue()); }
  inline bool goChild(std::size_t child);
  bool canGoChild(std::size_t /*child*/) const { return (curPath_.suffixLength() > 0); }
  inline bool canGoChildNode(std::size_t child) const;
  bool hasChildNode(std::size_t child) const { return canGoChildNode(child); }
  inline PathType goChildNode(std::size_t child);
  inline PathType childNodePath(std::size_t child) const;
  inline bool goParent();
  bool canGoParent() const { return (curPath_.size() > 0); }
  inline std::size_t parentNodeDistance() const;
  inline std::size_t goParentNode();
  NodeValue coveringNodeValueRO() const { return NodeValue{coveringValueNode()}; }
  NodeValue nodeValue() const { return (atNode() ? NodeValue{backNode()} : NodeValue{}); }
  NodeValue nodeValueRO() const { return nodeValue(); }

private:
  const Allocator* alloc_;
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

    NodePos() = default;
    NodePos(NodeImplRef nref) : nodeRefAtAbove(nref) {}
  };
  using NodeStack =  NodeStackT<NodePos,MaxDepth+1>;
  // Keep a stack of nodes around
  NodeStack nodeStack_{};
  Node backNode() const { return Node{alloc_,nodeStack_.back().nodeRefAtAbove}; }
  Node coveringValueNode() const { return Node{alloc_,nodeStack_.back().nodeRefAtAbove}; }

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
std::size_t
WalkCursorRO<R,MaxDepth,Alloc,NodeT,PathT,NodeStackT>::parentNodeDistance() const {
  return nodeStack_.back().depthBelow;
}

template <std::size_t R,std::size_t MaxDepth,typename Alloc,typename NodeT,typename PathT,template<typename,std::size_t> class NodeStackT>
std::size_t
WalkCursorRO<R,MaxDepth,Alloc,NodeT,PathT,NodeStackT>::goParentNode() {
  if (!canGoParent()) return 0;
  std::size_t depthBelow = nodeStack_.back().depthBelow;
  while (nodeStack_.back().depthBelow > 0) {
    nodeStack_.pop_back();
    curPath_.pop_back();
  }
  return depthBelow;
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

template <std::size_t R,std::size_t MaxDepth,typename Alloc,typename NodeT,typename PathT,template<typename,std::size_t> class NodeStackT>
PathT WalkCursorRO<R,MaxDepth,Alloc,NodeT,PathT,NodeStackT>::goChildNode(std::size_t child) {
  if (!canGoChildNode(child)) { return PathType{}; }
  PathType childPath{};
  const NodePos& np = nodeStack_.back();
  Node coveringNode{alloc_,np.nodeRefAtAbove};
  NodeImplRef childNodeRef{(np.depthBelow == 0) ? coveringNode.getChild(child) : np.nodeRefBelow};
  std::size_t depthBelow = np.depthBelow;
  Node childNode{alloc_,childNodeRef};
  if (depthBelow == 0) {
    NodePos newNodePos = nodeStack_.back();
    newNodePos.nodeRefBelow = childNodeRef;
    newNodePos.depthBelow++;
    newNodePos.edgeToBelow = childNode.edge();
    nodeStack_.push_back(std::move(newNodePos));
    curPath_.push_back(child);
    childPath.push_back(child);
    ++depthBelow;
  }
  while (!nodeStack_.back().edgeToBelow.empty()) {
    NodePos newNodePos = nodeStack_.back();
    newNodePos.depthBelow++;
    nodeStack_.push_back(std::move(newNodePos));
    std::size_t nextStep = newNodePos.edgeToBelow.at(0);
    curPath_.push_back(nextStep);
    childPath.push_back(nextStep);
    newNodePos.edgeToBelow.trim_front(1);
  }
  NodePos& finalNodePos(nodeStack_.back());
  finalNodePos.depthBelow = 0;
  finalNodePos.nodeRefAtAbove = finalNodePos.nodeRefBelow;
  finalNodePos.nodeRefBelow = Allocator::nullRef;

  return childPath;
}

template <std::size_t R,std::size_t MaxDepth,typename Alloc,typename NodeT,typename PathT,template<typename,std::size_t> class NodeStackT>
PathT
WalkCursorRO<R,MaxDepth,Alloc,NodeT,PathT,NodeStackT>::childNodePath(std::size_t child) const {
  PathType childPath{};
  if (!canGoChildNode(child)) { return childPath; }
  const NodePos& np(nodeStack_.back());
  if (np.depthBelow == 0) { childPath.push_back(child); }
  for (std::size_t i = 0; i < np.edgeToBelow.size(); ++i) {
    childPath.push_back(np.edgeToBelow.at(i));
  }
  return childPath;
}

} //  namespace RadixTree
} //  namespace Mapper
} //  namespace Akamai

#endif
