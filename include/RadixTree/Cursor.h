#ifndef AKAMAI_MAPPER_RADIX_TREE_CURSOR_H_
#define AKAMAI_MAPPER_RADIX_TREE_CURSOR_H_

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
 * \brief Baseline read-only cursor implementation.
 */
template <std::size_t R,std::size_t MD,typename AllocatorT,typename NodeT,typename PathT,template<typename,std::size_t> class NodeStackT>
class CursorRO
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

  CursorRO(const Allocator& a, NodeImplRef root)
    : alloc_(&a)
    , curPathNodeChild_(std::numeric_limits<std::size_t>::max())
  {
    NodePos rootPos(root,0);
    nodeStack_.push_back(rootPos);
  }
  CursorRO(const CursorRO& other) = default;
  CursorRO(CursorRO&& other) = default;
  CursorRO() = delete;
  CursorRO& operator=(const CursorRO& other) = default;
  CursorRO& operator=(CursorRO&& other) = default;

  // Interface methods start here
  
	/** \brief Return current Path on which the cursor is located */
	PathType getPath() const { return curPath_; }
  
  /**
   * \brief Check to see if the cursor is currently at a node in the tree.
   *
   * \return whether or not the cursor is at a node
   */
  bool atNode() const { return (curPath_.size() == nodeStack_.back().depth); }

  /**
   * \brief Check to see if the cursor is currently at a value (and implicitly a node) in the tree.
   *
   * \return whether or not the cursor is at a value
   */
  bool atValue() const { return (atNode() && backNode().hasValue()); }

  /**
   * \brief Move the cursor to a child position in the tracked tree.
   *
   * The cursor is always able to move to a child position (even if there
   * isn't any actual node/value there) as long as it isn't at the maximum
   * depth of the tree.
   *
   * \return whether or not the cursor actually went to the child
   * \param[in] child child number (0 or 1) to go to
   */
  inline bool goChild(std::size_t child);

  /**
   * \brief Check to see if the cursor could move towards a child node in the tracked tree.
   *
   * Since radix tree cursors can track non-existent positions in a tree, a critical piece
   * of information is whether or not there's actually a child node contained in the subtree
   * with a root at a particular child. Note that the presence of a node is not equivalent
   * to that node having a value: the cursor must be at a node to determine if there's a
   * value present.
   * @see atNode
   * @see atValue
   *
   * \return whether or not the cursor could go to the child - fails when at maximum depth
   * \param[in] child child number (0 or 1) to check
   */
  bool canGoChild(std::size_t /*child*/) const { return (curPath_.suffixLength() > 0); }
 
  /**
   * \brief Check to see if the cursor could move towards a child node in the tracked tree.
   *
   * Since radix tree cursors can track non-existent positions in a tree, a critical piece
   * of information is whether or not there's actually a child node contained in the subtree
   * with a root at a particular child. Note that the presence of a node is not equivalent
   * to that node having a value: the cursor must be at a node to determine if there's a
   * value present.
   * @see atNode
   * @see atValue
   *
   * \return whether or not the cursor could go to the child - fails when at maximum depth
   * \param[in] child child number (0..(R-1)) to check
   */
  bool canGoChildNode(std::size_t child) const { return (getChildNode(child).exists()); }
  bool hasChildNode(std::size_t child) const { return canGoChildNode(child); }
 
  /**
   * \brief Go directly to child node, passing over any edge, if possible.
   *
   * \return path object for the full path traversed down to the child (empty if no traversal possible)
   * \param[in] child child number to start at
   */
  inline PathType goChildNode(std::size_t child);
  /**
   * \brief Return the path that would be traversed by goChildNode(child) if it were called.
   *
   * \return path object for the full path that would be traversed down to the child (empty if no traversal possible)
   * \param[in] child child number to start at
   */
  inline PathType childNodePath(std::size_t child) const;
  /**
   * \brief Go to the parent of the current node if not at root.
   *
   * \return whether or not the cursor was able to move to the parent
   */
  inline bool goParent();
  
  /**
   * \brief Check to see if the cursor can move to the parent of the current node.
   *
   * \return whether or not the cursor was able to move to the parent
   */
  bool canGoParent() const { return (curPath_.size() > 0); }
  /**
   * \brief Return distance (in path branches) to parent node above the current.
   *
   * Returns the number of branches that would be traversed by goParentNode().
   * When at the root this value is 0, i.e. no motion to parent is possible so none would occur. 
   *
   * \return number of branches required to get from a node above down to the current position
   */
  inline std::size_t parentNodeDistance() const;
  /**
   * \brief Go to the parent node above the current position.
   *
   * When at root of tree no cursor motion occurs, 0 is returned.
   *
   * \return number of branches traversed to get to parent node
   */
  inline std::size_t goParentNode();


  /**
   * \brief Return read-only node value of covering node at current position in the path.
   *  "Covering node" is defined to be the nearest node immediately above current cursor position.
   */
  NodeValue coveringNodeValueRO() const { return NodeValue{coveringValueNode()}; }

  /**
   * \brief Return node value if cursor is at a node - empty if cursor is not at a node.
   *  
   * Non-Read-Only allows for node value to be updated, cleared, etc.
   */
  NodeValue nodeValue() const { return (atNode() ? NodeValue{backNode()} : NodeValue{}); }

  /**
   * \brief Return read-only node value if cursor is at node - empty if cursor is not at a node.
   */
  NodeValue nodeValueRO() const { return nodeValue(); }

protected:
  const Allocator* alloc_{nullptr};
  // We keep a stack of internal nodes and their depth in the path
  struct NodePos {
    // Node reference
    NodeImplRef nodeRef;
    // Depth of "node" from the root - root node is at 0
    std::size_t depth;

    NodePos(NodeImplRef nref = Allocator::nullRef,size_t d = 0)
      : nodeRef(nref)
      , depth(d)
    {}
  };
  using NodeStack =  NodeStackT<NodePos,MaxDepth+1>;
  // Keep a stack of nodes around
  NodeStack nodeStack_;
  Node backNode() const { return Node{alloc_,nodeStack_.back().nodeRef}; }
  Node coveringValueNode() const {
    std::size_t atStack = nodeStack_.size() - 1;
    while (atStack != 0) {
      Node n{alloc_,nodeStack_[atStack--].nodeRef};
      if (n.hasValue()) { return n; }      
    }
    return Node{alloc_,nodeStack_[0].nodeRef};
  }    
  
  // Keep our current position in the tree
 PathType curPath_;

  // If our current path is below a node then curPathChild_
  // is the direction it takes when leaving that node. If
  // our current path is more than 1 step below a node then
  // curPathEdge_ is an edge representing the remainder
  // of our path below that covering node.
  std::size_t curPathNodeChild_;
  NodeEdge curPathNodeEdge_;

  inline Node getChildNode(std::size_t child) const;
  inline NodeEdge edgeMatch() const;
};

/**
 * \brief Baseline read-write cursor implementation.
 */
template <std::size_t R,std::size_t MaxDepth,typename Alloc,typename NodeT,typename PathT,template<typename,std::size_t> class NodeStackT>
class Cursor
  : public CursorRO<R,MaxDepth,Alloc,NodeT,PathT,NodeStackT>
{
public:
  static constexpr std::size_t Radix = R;
  using CursorROType = CursorRO<R,MaxDepth,Alloc,NodeT,PathT,NodeStackT>;
  using CursorType = Cursor<R,MaxDepth,Alloc,NodeT,PathT,NodeStackT>;

  using Node = NodeT;
  using NodeImpl = typename Node::NodeImplType;
  using NodeImplRef = typename Node::NodeImplRefType;
  using ValueType = typename Node::ValueType;
  using Allocator = Alloc;
  using NodeEdge = typename Node::Edge;
  using NodeValueRO = typename CursorROType::NodeValueRO;
  using NodeValue = Mapper::RadixTree::NodeValue<Node>;
  using PathType = PathT;

  Cursor(Allocator& a,NodeImplRef root) : CursorROType(a,root) {}
  Cursor(const Cursor& other) = default;
  Cursor(Cursor&& other) = default;
  Cursor() = delete;
  Cursor& operator=(const Cursor& other) = default;
  Cursor& operator=(Cursor&& other) = default;
 
  /**
   * \brief Create a node at current position if it doesn't already exist.
   *
   * \return node value
   */
  inline NodeValue addNode();
  /**
   * \brief Remove the node from the current position, if it exists and is possible to remove.
   *
   * A node may only be removed if it has no value set and has no children.
   */
  inline bool removeNode();
  /**
   * \brief Return true if a call to removeNode() would succeed.
   */
  inline bool canRemoveNode() const;

  /**
   * \brief Get a read/write copy of the value at the current position.
   */
  NodeValue nodeValue() { return (this->atNode() ? NodeValue{this->backNode()} : NodeValue{}); }

  /**
   * \brief Set value at this node, throws exception if not at a node, copies value.
   */
  void setValue(const ValueType& v) { nodeValue().set(v); }

  /**
   * \brief Set value at this node, throws exception if not at a node, moves value.
   */
  void setValue(ValueType&& v) { nodeValue().set(std::move(v)); }

   /**
   * \brief Remove the value (if any) from the current position.
   */
  inline bool clearValue();

private:
  Allocator* alloc() { return const_cast<Allocator*>(this->alloc_); }
  typedef typename CursorROType::NodePos NodePos;
};




///////////////////////////////////////////////////////////////////////
/////////// IMPLEMENTATION - RadixTree CursorRO //////////////////////
///////////////////////////////////////////////////////////////////////

template <std::size_t R,std::size_t MaxDepth,typename Alloc,typename NodeT,typename PathT,template<typename,std::size_t> class NodeStackT>
bool CursorRO<R,MaxDepth,Alloc,NodeT,PathT,NodeStackT>::goChild(std::size_t child) {
  // A bit more complicated than you might think because we have
  // to track various nonexistent positions in the tree.

  if (!canGoChild(child)) return false;

  NodePos& pos(nodeStack_.back());
  std::size_t curDepthBelow = (curPath_.size() - pos.depth);

  if (curDepthBelow == 0) {
    // Popping just below the current node, set the node child
    // we'll be in as well as reset our edge.
    curPathNodeChild_ = child;
    curPathNodeEdge_.clear();
  } else if (!curPathNodeEdge_.full()) {
    // Our edge isn't full yet, so we need to update it.
    curPathNodeEdge_.push_back(child);
  }
  curPath_.push_back(child);

  // Next check to see if we've just descended into a child node, update our
  // stack and edge information accordingly.
  Node backNode = this->backNode();
  NodeImplRef childNodeRef = backNode.getChild(curPathNodeChild_);
  Node childNode{alloc_,childNodeRef};
  if (childNode.exists()) {
    const NodeEdge& childExt(childNode.edge());
    // Only check if we're at a point where we *might* jump down into the next node
    if ((curDepthBelow == childExt.size()) && (childExt == curPathNodeEdge_)) {
      NodePos newPos(childNodeRef,pos.depth + curDepthBelow + 1);
      nodeStack_.push_back(newPos);
      curPathNodeEdge_.clear();
      curPathNodeChild_ = std::numeric_limits<std::size_t>::max();
    }
  }

  return true;
}

template <std::size_t R,std::size_t MaxDepth,typename Alloc,typename NodeT,typename PathT,template<typename,std::size_t> class NodeStackT>
bool CursorRO<R,MaxDepth,Alloc,NodeT,PathT,NodeStackT>::goParent() {
  if (curPath_.size() == 0) return false;
  const NodePos& pos(nodeStack_.back());
  std::size_t depthBelow = (curPath_.size() - pos.depth);
  if (depthBelow == 0) {
    // We're popping up from a node, set our current edge
    // to get us down to one above the one we're moving away from
    // (may be empty) and curPathNodeChild_ to match where our
    // current overall path matches the new covering node.
    NodeEdge curNodeEdge = backNode().edge();
    nodeStack_.pop_back();
    const NodePos& newPos(nodeStack_.back());
    std::size_t newDepthBelow = ((curPath_.size() - 1) - newPos.depth);
    if (newDepthBelow == 0) {
      // We just popped up into another node - clear our edge
      // and the node that owns the edge along the node path
      curPathNodeEdge_.clear();
      curPathNodeChild_ = std::numeric_limits<std::size_t>::max();
    } else {
      // We're still below the node above so update our current
      // edge to match - one back from the end. 
      curNodeEdge.pop_back();
      curPathNodeEdge_ = curNodeEdge;
      curPathNodeChild_ = curPath_[newPos.depth];
    }
  } else if (depthBelow == 1) {
    // We're popping up into a node - no longer have a node responsible for our edge
    curPathNodeChild_ = std::numeric_limits<std::size_t>::max();
    curPathNodeEdge_.clear();
  } else if (depthBelow <= (curPathNodeEdge_.capacity()+1)) {
    // We're within range of our edge tracking so update it.
    curPathNodeEdge_.pop_back();
  }
  curPath_.pop_back();

  return true;
}

template <std::size_t R,std::size_t MaxDepth,typename Alloc,typename NodeT,typename PathT,template<typename,std::size_t> class NodeStackT>
std::size_t
CursorRO<R,MaxDepth,Alloc,NodeT,PathT,NodeStackT>::parentNodeDistance() const {
  if (!canGoParent()) { return 0; }
  const NodePos& pos(nodeStack_.back());
  std::size_t depthBelow = (curPath_.size() - pos.depth);
  if (depthBelow == 0) {
    // At a node so have to jump back up one position in the stack.
    const NodePos& prevPos(nodeStack_.at(nodeStack_.size() - 2));
    depthBelow = (curPath_.size() - prevPos.depth);
  }
  return depthBelow;
}

template <std::size_t R,std::size_t MaxDepth,typename Alloc,typename NodeT,typename PathT,template<typename,std::size_t> class NodeStackT>
std::size_t
CursorRO<R,MaxDepth,Alloc,NodeT,PathT,NodeStackT>::goParentNode() {
  if (!canGoParent()) return 0;
  const NodePos& pos(nodeStack_.back());
  std::size_t depthBelow = (curPath_.size() - pos.depth);
  if (depthBelow == 0) {
    nodeStack_.pop_back();
    const NodePos& newPos(nodeStack_.back());
    depthBelow = (curPath_.size() - newPos.depth);
  }
  for (std::size_t i = 0;i < depthBelow;++i) { curPath_.pop_back(); }
  curPathNodeChild_ = std::numeric_limits<std::size_t>::max();
  curPathNodeEdge_.clear();

  return depthBelow;
}

template <std::size_t R,std::size_t MaxDepth,typename Alloc,typename NodeT,typename PathT,template<typename,std::size_t> class NodeStackT>
PathT CursorRO<R,MaxDepth,Alloc,NodeT,PathT,NodeStackT>::goChildNode(std::size_t child) {
  Node childNode{getChildNode(child)};
  if (!childNode.exists()) { return PathType{}; }

  const NodeEdge& ext(childNode.edge());
  const NodePos& pos(nodeStack_.back());
  std::size_t depthBelow = (curPath_.size() - pos.depth);

  // Setup our new node stack position
  NodePos newPos{childNode.nodeImplRef(),pos.depth + ext.size() + 1};
  nodeStack_.push_back(newPos);
  curPathNodeEdge_.clear();
  curPathNodeChild_ = std::numeric_limits<std::size_t>::max();
  PathType childPath{};
  // If we're at a node then first go to the passed in child
  // to get us to the first child in the edge.
  if (depthBelow == 0) {
    curPath_.push_back(child);
    childPath.push_back(child);
  }
  // Walk as much of the edge as we need.
  for (std::size_t i=0;i<ext.size();++i) {
    curPath_.push_back(ext[i]);
    childPath.push_back(ext[i]);
  }
  return childPath;
}

template <std::size_t R,std::size_t MaxDepth,typename Alloc,typename NodeT,typename PathT,template<typename,std::size_t> class NodeStackT>
PathT
CursorRO<R,MaxDepth,Alloc,NodeT,PathT,NodeStackT>::childNodePath(std::size_t child) const {
  Node childNode{getChildNode(child)};
  if (!childNode.exists()) { return PathType{}; }

  const NodePos& pos(nodeStack_.back());
  std::size_t depthBelow = (curPath_.size() - pos.depth);
  const NodeEdge& ext(childNode.edge());
  PathType childPath{};
  // We have to go to the child first before we hit the edge
  // if we're already at a node.
  if (depthBelow == 0) { childPath.pushBack(child); }
  // Go down the remainder of the edge.
  std::size_t extStart = ((depthBelow == 0) ? 0 : (depthBelow - 1));
  for (std::size_t i = extStart; i < ext.size(); ++i) { childPath.pushBack(ext[i]); }
  return childPath;
}


template <std::size_t R,std::size_t MaxDepth,typename Alloc,typename NodeT,typename PathT,template<typename,std::size_t> class NodeStackT>
typename CursorRO<R,MaxDepth,Alloc,NodeT,PathT,NodeStackT>::Node
CursorRO<R,MaxDepth,Alloc,NodeT,PathT,NodeStackT>::getChildNode(std::size_t child) const {
  const NodePos& pos(nodeStack_.back());
  Node back = backNode();
  std::size_t depthBelow = (curPath_.size() - pos.depth);
  // Check immediate child if we're at a node
  if (depthBelow == 0) { return Node{alloc_,back.getChild(child)}; }
  // Check the child at which we jump off the current covering node
  NodeImplRef childNodeRef = Allocator::nullRef;
  if (curPathNodeChild_ != std::numeric_limits<std::size_t>::max()) {
    childNodeRef = back.getChild(curPathNodeChild_);
  }
  if (childNodeRef == Allocator::nullRef) {
    return Node{};
  }
  // If we get here then we need to check the edge. First make sure
  // that we haven't gone deeper than the edge allows.
  Node childNode{alloc_,childNodeRef};
  const NodeEdge& nodeExt(childNode.edge());
  if (depthBelow > nodeExt.size()) {
    return Node{};
  }
  // Finally check to see if the node edge covers the child position
  // we want to check.
  NodeEdge newExt(curPathNodeEdge_);
  newExt.push_back(child);
  if (newExt.coveredby(nodeExt)) {
    return childNode;
  }
  return Node{};
}

template <std::size_t R,std::size_t MaxDepth,typename Alloc,typename NodeT,typename PathT,template<typename,std::size_t> class NodeStackT>
typename CursorRO<R,MaxDepth,Alloc,NodeT,PathT,NodeStackT>::NodeEdge
CursorRO<R,MaxDepth,Alloc,NodeT,PathT,NodeStackT>::edgeMatch() const {
  const NodePos& pos(nodeStack_.back());
  std::size_t depthBelow = (curPath_.size() - pos.depth);
  // If we're at or one below then we can't be inside an edge
  if (depthBelow < 2) { return NodeEdge(); }
  NodeImplRef edgeNodeRef = Allocator::nullRef;
  if (curPathNodeChild_ != std::numeric_limits<std::size_t>::max()) {
    edgeNodeRef = backNode().getChild(curPathNodeChild_);
  }
  // If we've got no edge handling node then we also can't be inside an edge
  if (edgeNodeRef == Allocator::nullRef) { return NodeEdge{}; }
  Node edgeNode{alloc_,edgeNodeRef};
  NodeEdge extMatch = curPathNodeEdge_;
  auto matchBits = extMatch.matching(edgeNode.edge());
  extMatch.trim_back(extMatch.size() - matchBits);
  return extMatch;
}



///////////////////////////////////////////////////////////////////////
/////////// IMPLEMENTATION - RadixTree Cursor /////////////////////////
///////////////////////////////////////////////////////////////////////

template <std::size_t R,std::size_t MaxDepth,typename Alloc,typename NodeT,typename PathT,template<typename,std::size_t> class NodeStackT>
typename Cursor<R,MaxDepth,Alloc,NodeT,PathT,NodeStackT>::NodeValue
Cursor<R,MaxDepth,Alloc,NodeT,PathT,NodeStackT>::addNode()
{
  NodePos& pos(CursorROType::nodeStack_.back());
  std::size_t depthBelow = (CursorROType::curPath_.size() - pos.depth);
  // At node - nothing left to do.
  if (depthBelow == 0) { return nodeValue(); }

  // Possible step 0: check to see if we can extend our existing covering node
  // to make a node here. If the covering node isn't the root, has no children,
  // and no set value then we could.
  // We aren't going to do this for now because it changes the depth of an
  // existing node in the tree, which invalidates any cursors at that
  // node in a non-obvious fashion. It'd be nice to keep it so that a cursor
  // at a particular node isn't invalidated by changes below that node.
  // The price we pay is a more dense tree structure in some situations, but
  // my intuition is that the tradeoff is worthwhile to make. Time/use will tell.

  // Step 1: we need to add at least a "branch" node as a child of
  // our covering node to accomodate this position. Create it and
  // swap it in for any child node of the current cover that we
  // displace.
  NodeImplRef branchNodeRef = alloc()->newRef();
  Node branchNode{alloc(),branchNodeRef};
  NodeEdge extMatch = CursorROType::edgeMatch();

  // We'll be branching off of the cover node at pos.depth in our path.
  std::size_t prevChildIndex = CursorROType::curPath_[pos.depth];
  NodeImplRef prevChildRef = this->backNode().setChild(prevChildIndex,branchNodeRef);
  if (prevChildRef != Allocator::nullRef) {
    // If we got here then prevChild must have an edge - if it didn't then it
    // would be our covering node.
    // Trim its edge by however much of the edge we were matching at
    // this position (may be 0) plus one for the additional branchNode
    // inserted between prevChild and the parent. The first bit of the edge
    // gets "saved" as the child location, the rest is covered by our current
    // edge match.
    Node prevChild{alloc(),prevChildRef};
    branchNode.setChild(prevChild.edge()[extMatch.size()],prevChildRef);
    prevChild.edge().trim_front(extMatch.size() + 1);
    branchNode.edge() = extMatch;
  }

  // Step 2: update our node stack to include the branch node we just made
  // at or above our current path in the tree.
  NodePos branchNodePos(branchNodeRef,pos.depth + 1 + extMatch.size());
  CursorROType::nodeStack_.push_back(branchNodePos);
  PathType newNodePath(CursorROType::curPath_);
  newNodePath.resize(branchNodePos.depth);
  
  // Step 3: if we've got any difference bewteen curPath_ and
  // newNodePath then we'll reconcile it now. If branchNode has no children,
  // we can start absorbing the difference with it.
  // If it does then we need to add a new node in the appropriate
  // empty slot and start with that node.
  if (newNodePath.size() < CursorROType::curPath_.size()) {
    NodeImplRef childRef = branchNodeRef;
    bool hasChildren = false;
    for (std::size_t i=0;i<R;++i) {
      if (branchNode.getChild(i) != Allocator::nullRef) {
        hasChildren = true;
        break;
      }
    }
    if (hasChildren) {
      childRef = alloc()->newRef();
      std::size_t newChildPathStep = CursorROType::curPath_[branchNodePos.depth];
      branchNode.setChild(newChildPathStep,childRef);
      NodePos newNodePos(childRef,branchNodePos.depth+1);
      CursorROType::nodeStack_.push_back(newNodePos);
      newNodePath.push_back(newChildPathStep);
    }
    Node child{alloc(),childRef};

    // Keep extending until we match curPath_
    while (newNodePath.size() < CursorROType::curPath_.size()) {
      // If we've got a full edge then we'll need to make
      // a new node, otherwise we can just push the next path
      // bit into the existing node edge.
      std::size_t nextPathStep = CursorROType::curPath_[newNodePath.size()];
      if (child.edge().full()) {
        NodeImplRef newChildRef = alloc()->newRef();
        child.setChild(nextPathStep,newChildRef);
        newNodePath.push_back(nextPathStep);
        NodePos newPos(newChildRef,newNodePath.size());
        CursorROType::nodeStack_.push_back(newPos);
        childRef = newChildRef;
        child = Node{alloc(),newChildRef};
      } else {
        child.edge().push_back(nextPathStep);
        newNodePath.push_back(nextPathStep);
        ++(CursorROType::nodeStack_.back().depth);
      }
    }
  }
  // We're now at a node, reset our edge
  CursorROType::curPathNodeEdge_.clear();
  CursorROType::curPathNodeChild_ = std::numeric_limits<std::size_t>::max();
  return nodeValue();
}

template <std::size_t R,std::size_t MaxDepth,typename Alloc,typename NodeT,typename PathT,template<typename,std::size_t> class NodeStackT>
bool Cursor<R,MaxDepth,Alloc,NodeT,PathT,NodeStackT>::removeNode() {
  // No node at all here? Success.
  if (!CursorROType::atNode()) { return true; }
  // Value here? Fail - need to remove the value first.
  if (CursorROType::atValue()) { return false; }
  // Can't delete the root
  if (CursorROType::nodeStack_.size() <= 1) { return false; }
  // If we've got any children then we fail. Not going
  // to do more complicated stuff like try to figure out
  // if we can subsume this node into an edge.
  for (std::size_t i=0;i<R;++i) {
    if (CursorROType::canGoChildNode(i)) { return false; }
  }

  // If we get this far we can remove the node. This consists of
  // removing the position from our nodeStack_, its parent, and deleting the
  // node itself. Note that our position in the tree remains the same,
  // we just aren't at a node any longer.
  CursorROType::nodeStack_.pop_back();
  NodePos& pos(CursorROType::nodeStack_.back());
  std::size_t prevChildIndex = CursorROType::curPath_[pos.depth];
  Node backNode = this->backNode();
  NodeImplRef nodeRefToRemove = backNode.detachChild(prevChildIndex);
  // Replace our current path edge with the edge of the node
  // that we're removing from the tree before we delete the node.
  Node nodeToRemove{alloc(),nodeRefToRemove};
  std::swap(CursorROType::curPathNodeEdge_,nodeToRemove.edge());
  CursorROType::curPathNodeChild_ = prevChildIndex;
  alloc()->deleteRef(nodeRefToRemove);
  return true;
}


template <std::size_t R,std::size_t MaxDepth,typename Alloc,typename NodeT,typename PathT,template<typename,std::size_t> class NodeStackT>
bool Cursor<R,MaxDepth,Alloc,NodeT,PathT,NodeStackT>::canRemoveNode() const {
  // No node at all here? Success.
  if (!CursorROType::atNode()) { return true; }
  // Value here? Fail - need to remove the value first.
  if (CursorROType::atValue()) { return false; }
  // Can't delete the root
  if (CursorROType::nodeStack_.size() <= 1) { return false; }
  // If we've got any children then we fail. Not going
  // to do more complicated stuff like try to figure out
  // if we can subsume this node into an edge.
  for (std::size_t i=0;i<R;++i) {
    if (CursorROType::canGoChildNode(i)) { return false; }
  }
  return true;
}

template <std::size_t R,std::size_t MaxDepth,typename Alloc,typename NodeT,typename PathT,template<typename,std::size_t> class NodeStackT>
bool Cursor<R,MaxDepth,Alloc,NodeT,PathT,NodeStackT>::clearValue() {
  // No node at all here? Then it's impossible to clear a value.
  if (!CursorROType::atNode()) { return false; }
  // We cannot clear a value that does not exist.
  if (!CursorROType::atValue()) { return false; }
  // Else we want to clear the value at the node.
  nodeValue().clear();
  return true;
}

} //  namespace RadixTree
} //  namespace Mapper
} //  namespace Akamai

#endif
