#ifndef AKAMAI_MAPPER_RADIX_TREE_LOOKUP_CURSOR_H_
#define AKAMAI_MAPPER_RADIX_TREE_LOOKUP_CURSOR_H_

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
#include <stdexcept>

#include "NodeValue.h"

namespace Akamai {
namespace Mapper {
namespace RadixTree {

/**
 * \brief Cursor intended for value lookup, not general traversal.
 * 
 * The baseline CursorRO class allows general traversal of a radix tree.
 * The LookupCursorRO only allows one-way downward tree traversal, as
 * required for individual longest prefix value lookups. This limitation
 * allows for a much simpler and faster tree traversal.
 */
template <std::size_t R,std::size_t MD,typename AllocatorT,typename NodeT,typename PathT>
class LookupCursorRO
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

  LookupCursorRO(const Allocator& a, NodeImplRef root)
    : alloc_(&a)
    , nodeRefAtAbove_(root)
    , coveringValue_(Node{&a,root})
  {}
  LookupCursorRO(const LookupCursorRO& other) = default;
  LookupCursorRO(LookupCursorRO&& other) = default;
  LookupCursorRO() = delete;
  LookupCursorRO& operator=(const LookupCursorRO& other) = default;
  LookupCursorRO& operator=(LookupCursorRO&& other) = default;

  // Interface methods start here
  
  PathType getPath() const { return curPath_; }
  
  bool atNode() const { return (depthBelow_ == 0); }

  bool atValue() const { return (atNode() && coveringNode().hasValue()); }

  inline bool goChild(std::size_t child);

  bool canGoChild(std::size_t /*child*/) const { return (curPath_.suffixLength() > 0); }
 
  bool canGoChildNode(std::size_t child) const {
    if (depthBelow_ == 0) { return (coveringNode().getChild(child) != Allocator::nullRef); }
    if (nodeRefBelow_ == Allocator::nullRef) { return false; }
    return (edgeToBelow_.at(0) == child);
  }
  bool hasChildNode(std::size_t child) const { return canGoChildNode(child); }
 
  PathType goChildNode(std::size_t) { throw std::runtime_error("LookupCursorRO: goChildNode unimplemented"); return PathType{}; }
  PathType childNodePath(std::size_t) const { throw std::runtime_error("LookupCursorRO: childNodePath unimplemented"); return PathType{}; }
 
  bool goParent() { throw std::runtime_error("LookupCursorRO: can't return"); return false; }
  bool canGoParent() const { return false; }

  std::size_t parentNodeDistance() const { throw std::runtime_error("LookupCursorRO: parentNodeDistance unimplemented"); return 0; }
 
  std::size_t goParentNode() { throw std::runtime_error("LookupCursorRO: can't return to parent node"); }
  NodeValue coveringValueRO() const { return coveringValue_; }
  NodeValue nodeValue() const { return (atNode() ? NodeValue{coveringNode()} : NodeValue{}); }
  NodeValue nodeValueRO() const { return nodeValue(); }

private:
  const Allocator* alloc_{nullptr};

  // Reference to node at/above current position
  NodeImplRef nodeRefAtAbove_{Allocator::nullRef};
  // Steps below nodeRefAtAbove
  std::size_t depthBelow_{0};
  // A node edge representing the rest of the steps below nodeRefAtAbove (if any)
  NodeEdge edgeToBelow_{};
  // Reference to node below the current position (if any)
  NodeImplRef nodeRefBelow_{Allocator::nullRef};
  Node coveringNode() const { return Node{alloc_,nodeRefAtAbove_}; }

  // Keep our current position in the tree
  PathType curPath_;
  // Keep the last actual node value we saw during our descent
  NodeValue coveringValue_{};

  inline Node getChildNode(std::size_t child) const;
  inline NodeEdge edgeMatch() const;
};

/**
 * \brief Cursor intended for adding individual values to a tree, not general traversal.
 * 
 * The baseline Cursor class allows general tree traversal. The LookupCursorWO cursor
 * only allows downward traversal of the tree, as used when inserting individual items
 * into the tree. As it traverses the tree it also adds nodes as required as it goes,
 * with the assumption that the intention is to write a new terminal value in the tree.
 * This is in contrast to the usual cursor implementations which do not make alterations
 * to the tree as they traverse. The advantage of this cursor is that it is significantly
 * simpler and faster.
 */
template <std::size_t R,std::size_t MD,typename AllocatorT,typename NodeT,typename PathT>
class LookupCursorWO
{
public:
  static constexpr std::size_t Radix = R;
  static constexpr std::size_t MaxDepth = MD;
  using Node = NodeT;
  using NodeImpl = typename Node::NodeImplType;
  using NodeImplRef = typename Node::NodeImplRefType;
  using ValueType = typename Node::ValueType;
  using NodeValue = Mapper::RadixTree::NodeValue<Node>;
  using NodeValueRO = Mapper::RadixTree::NodeValueRO<Node>;
  using Allocator = AllocatorT;
  using PathType = PathT;
  using NodeEdge = typename NodeT::Edge;
  static constexpr std::size_t NoChild = Node::NoChild;

  LookupCursorWO(Allocator& a, NodeImplRef root)
    : alloc_(&a)
    , nodeRefAtAbove_(root)
  {}
  // Don't allow copying of write-only cursors - can't think of any
  // benefits offhand that outweigh the potential for undesirable behavior.
  LookupCursorWO(const LookupCursorWO& other) = delete;
  LookupCursorWO(LookupCursorWO&& other) = default;
  LookupCursorWO() = delete;
  LookupCursorWO& operator=(const LookupCursorWO& other) = delete;
  LookupCursorWO& operator=(LookupCursorWO&& other) = default;

  // Interface methods start here
  
  PathType getPath() const { return curPath_; }
  
  bool atNode() const { return (depthBelow_ == 0); }

  bool atValue() const { return (atNode() && coveringNode().hasValue()); }

  inline bool goChild(std::size_t child);

  bool canGoChild(std::size_t /*child*/) const { return (curPath_.suffixLength() > 0); }
 
  bool canGoChildNode(std::size_t child) const {
    if (depthBelow_ == 0) { return (coveringNode().getChild(child) != Allocator::nullRef); }
    if (nodeRefBelow_ == Allocator::nullRef) { return false; }
    return (edgeToBelow_.at(0) == child);
  }
  bool hasChildNode(std::size_t child) const { return canGoChildNode(child); }
 
  PathType goChildNode(std::size_t) { throw std::runtime_error("LookupCursorWO: goChildNode unimplemented"); return PathType{}; }
  PathType childNodePath(std::size_t) const { throw std::runtime_error("LookupCursorWO: childNodePath unimplemented"); return PathType{}; }
 
  bool goParent() { throw std::runtime_error("LookupCursorWO: can't return"); return false; }
  bool canGoParent() const { return false; }

  std::size_t parentNodeDistance() const { throw std::runtime_error("LookupCursorWO: parentNodeDistance unimplemented"); return 0; }
 
  std::size_t goParentNode() { throw std::runtime_error("LookupCursorWO: can't return to parent node"); }
  NodeValue nodeValue() { return (atNode() ? NodeValue{coveringNode()} : NodeValue{}); }
  NodeValue nodeValueRO() const { return (atNode() ? NodeValueRO{coveringNode()} : NodeValueRO{}); }

  inline NodeValue addNode();

  bool removeNode() { throw std::runtime_error("LookupCursorWO: can't remove nodes"); }
  bool canRemoveNode() const { return false; }

private:
  Allocator* alloc_{nullptr};

  // Reference to node at/above current position
  NodeImplRef nodeRefAtAbove_{Allocator::nullRef};
  // Steps below nodeRefAtAbove
  std::size_t depthBelow_{0};
  std::size_t childFromAbove_{NoChild};
  NodeEdge edgeFromAbove_{};

  // A node edge representing the rest of the steps below nodeRefAtAbove (if any)
  NodeEdge edgeToBelow_{};
  // Reference to node below the current position (if any)
  NodeImplRef nodeRefBelow_{Allocator::nullRef};
  Node coveringNode() const { return Node{alloc_,nodeRefAtAbove_}; }

  // Keep our current position in the tree
  PathType curPath_;

  inline Node getChildNode(std::size_t child) const;
  inline NodeEdge edgeMatch() const;
};

/////////////////////
// IMPLEMENTATIONS //
/////////////////////

template <std::size_t R,std::size_t MD,typename AllocatorT,typename NodeT,typename PathT>
bool LookupCursorRO<R,MD,AllocatorT,NodeT,PathT>::goChild(std::size_t child) {
  if (depthBelow_ == 0) {
    // We're at a node, drop down into the immediate child
    nodeRefBelow_ = Node{alloc_,nodeRefAtAbove_}.getChild(child);
    if (nodeRefBelow_ != Allocator::nullRef) {
      edgeToBelow_ = Node{alloc_,nodeRefBelow_}.edge();
    }
  } else if (nodeRefBelow_ != Allocator::nullRef) {
    // Check to see if we match the next step in the edge down to
    // the node below. If so, trim it off. If not, then reset the edge
    // as well as the node below ref - we've gone outside the edge.
    if (child == edgeToBelow_.at(0)) {
      edgeToBelow_.trim_front(1);
    } else {
      edgeToBelow_.clear();
      nodeRefBelow_ = Allocator::nullRef;
    }
  }
  ++depthBelow_;
  // See if we've arrived at the node below (if there is one).
  if ((nodeRefBelow_ != Allocator::nullRef) && edgeToBelow_.empty()) {
    nodeRefAtAbove_ = nodeRefBelow_;
    nodeRefBelow_ = Allocator::nullRef;
    depthBelow_ = 0;
    Node nodeAtAbove{alloc_,nodeRefAtAbove_};
    if (nodeAtAbove.hasValue()) { coveringValue_ = NodeValue{nodeAtAbove}; }
  }
  curPath_.push_back(child);
  return true;
}

template <std::size_t R,std::size_t MD,typename AllocatorT,typename NodeT,typename PathT>
bool LookupCursorWO<R,MD,AllocatorT,NodeT,PathT>::goChild(std::size_t child) {
  if (!canGoChild(child)) { return false; }

  // categorize our current position in the tree, have to
  // do different things depending on what our local topology is
  bool atNode = (depthBelow_ == 0);
  bool inEdge = (nodeRefBelow_ != Allocator::nullRef);
  bool runningFree = (!inEdge && !atNode);
  
  // filling our tracking edge or breaking out of an edge -
  // create new node and update our current local topology
  if ((runningFree && edgeFromAbove_.full()) ||
      (inEdge && (child != edgeToBelow_.at(0))))
  {
    addNode();
    atNode = true;
    inEdge = false;
    runningFree = false;
  }

  // the previous check ensures that if we're in an
  // edge or running free we aren't going to break
  // out of the current edge or go off of the end
  if (inEdge) {
    edgeFromAbove_.push_back(child);
    edgeToBelow_.trim_front(1);
    if (edgeToBelow_.empty()) {
      // exiting edge, now in node below
      nodeRefAtAbove_ = nodeRefBelow_;
      nodeRefBelow_ = Allocator::nullRef;
      edgeFromAbove_.clear();
      depthBelow_ = 0;
      childFromAbove_ = NoChild;
    } else {
      ++depthBelow_;
    }
  } else if (runningFree) {
    edgeFromAbove_.push_back(child);
    ++depthBelow_;
  } else if (atNode) {
    Node parentNode{alloc_,nodeRefAtAbove_};
    childFromAbove_ = child;
    NodeImplRef childRef = parentNode.getChild(child);
    if (childRef != Allocator::nullRef) {
      Node childNode{alloc_,childRef};
      nodeRefBelow_ = childRef;
      edgeToBelow_ = Node{alloc_,childRef}.edge();
      if (edgeToBelow_.empty()) {
        // heading directly into node below - no edge
        nodeRefAtAbove_ = childRef;
        edgeFromAbove_.clear();
        nodeRefBelow_ = Allocator::nullRef;
        depthBelow_ = 0;
        childFromAbove_ = NoChild;
      } else {
        // now in edge betewen nodes
        depthBelow_ = 1;
      }
    } else {
      // now running free
      depthBelow_ = 1;
    }
  } else {
    throw std::runtime_error("LookupCursorWO: not at edge, at node, or running free - state corrupted");
  }
  curPath_.push_back(child);
  return true;
}

template <std::size_t R,std::size_t MD,typename AllocatorT,typename NodeT,typename PathT>
typename LookupCursorWO<R,MD,AllocatorT,NodeT,PathT>::NodeValue
LookupCursorWO<R,MD,AllocatorT,NodeT,PathT>::addNode() {
  if (depthBelow_ == 0) { return NodeValue{coveringNode()}; }
  Node nodeAbove{alloc_,nodeRefAtAbove_};
  NodeImplRef newNodeRef = alloc_->newRef();
  Node newNode{alloc_,newNodeRef};
  newNode.edge() = edgeFromAbove_;
  nodeAbove.setChild(childFromAbove_,newNodeRef);
  // If we're on a path down to a node beneath us then
  // we need to split the edge to create the new node.
  if (nodeRefBelow_ != Allocator::nullRef) {
    Node nodeBelow{alloc_,nodeRefBelow_};
    // Absorb the first step of the edge below into the child of
    // the node we're about to create, trim the child's edge
    // to account for how far down the edge we are.
    newNode.setChild(edgeToBelow_.at(0),nodeRefBelow_);
    nodeBelow.edge().trim_front(depthBelow_);
  }
  nodeRefAtAbove_ = newNodeRef;
  nodeRefBelow_ = Allocator::nullRef;
  depthBelow_ = 0;
  childFromAbove_ = NoChild;
  edgeFromAbove_.clear();
  edgeToBelow_.clear();
  return NodeValue{newNode};
}


} //  namespace RadixTree
} //  namespace Mapper
} //  namespace Akamai

#endif
