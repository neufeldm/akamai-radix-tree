#ifndef AKAMAI_MAPPER_RADIX_TREE_RADIX_TREE_H_
#define AKAMAI_MAPPER_RADIX_TREE_RADIX_TREE_H_

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

#include "Cursor.h"
#include "WalkCursorRO.h"
#include "LookupCursor.h"

namespace Akamai {
namespace Mapper {
namespace RadixTree {

/**
 * \file RadixTree.h
 * Template for assembling an actual radix tree definition.
 * Includes copy/move stuff, getting cursors.
 */


/**
 * \brief Combine a path encapsulator and node type to make a tree.
 */
template <typename PathType,typename NodeType,template<typename,std::size_t> class NodeStackType>
class RadixTree {
public:
  static_assert(NodeType::Radix == PathType::Radix,"Node radix and path radix mismatch!");
  static constexpr std::size_t Radix = NodeType::Radix;
  static constexpr std::size_t MaxDepth = PathType::MaxDepth;
  using NodeAllocatorType = typename NodeType::AllocatorType;
  using MyType = RadixTree<PathType,NodeType,NodeStackType>;

  using CursorROType = Mapper::RadixTree::CursorRO<Radix,MaxDepth,NodeAllocatorType,NodeType,PathType,NodeStackType>;
  using CursorType = Mapper::RadixTree::Cursor<Radix,MaxDepth,NodeAllocatorType,NodeType,PathType,NodeStackType>;
 
  using WalkCursorROType = Mapper::RadixTree::WalkCursorRO<Radix,MaxDepth,NodeAllocatorType,NodeType,PathType,NodeStackType>;
  using LookupCursorROType = Mapper::RadixTree::LookupCursorRO<Radix,MaxDepth,NodeAllocatorType,NodeType,PathType>;
  using LookupCursorWOType = Mapper::RadixTree::LookupCursorWO<Radix,MaxDepth,NodeAllocatorType,NodeType,PathType>;
  
  using Value = typename CursorROType::ValueType;

  template <typename... AllocatorArgs>
  RadixTree(AllocatorArgs&&... aa) : alloc_(std::forward<AllocatorArgs>(aa)...), root_(static_cast<NodeRefType>(alloc_.newRef())) {}

  // No auto copying allowed - any tree copying must be done manually
  RadixTree(const MyType& o) = delete;
  MyType& operator=(const MyType& o) = delete;

  // Need to support moves
  inline RadixTree(MyType&& o);
  inline MyType& operator=(MyType&& o);

  inline virtual ~RadixTree();

  /**
   * \brief Destroy any existing tree, start with a new root.
   */
  inline void clear();

  /**
   * \brief Return an RO cursor at the root of the tree.
   */
  CursorROType cursorRO() const { return CursorROType(alloc_,root_); }
  /**
   * \brief Return an RW cursor at the root of the tree.
   */
  CursorType cursor() { return CursorType(alloc_,root_); }

  /**
   * \brief Return a "walking" cursor at the root of the tree.
   * A "walking" cursor allows full read-only navigation of a tree, but is less
   * robust against simultaneous reads/writes than the standard cursor.
   * This cursor is often somewhat faster than the standard cursor.
   */
  WalkCursorROType walkCursorRO() const { return WalkCursorROType(alloc_,root_); }

  /**
   * \brief Return a read-only lookup cursor at the root of the tree.
   * A unidirectional cursor, only able to visit children, never return back
   * up the tree. Significantly faster than the regular cursor, completely
   * adequate for simple longest prefix lookup operations.
   */
  LookupCursorROType lookupCursorRO() const { return LookupCursorROType(alloc_,root_); }

  /**
   * \brief Return a write-only lookup cursor at the root of the tree.
   * Unidirectional, like the read-only cursor. In addition, it creates nodes
   * as required on its way down, i.e. the path you trace with the cursor
   * is created in the tree as you go. As with the read-only, significantly
   * faster than the regular cursor, completely adequate for making a single
   * addition to the tree.
   */
  LookupCursorWOType lookupCursorWO() { return LookupCursorWOType(alloc_,root_); }

  /**
   * \brief Return a const ref to the allocator in use.
   */
  const NodeAllocatorType& nodeAllocator() const { return alloc_; }

private:
  using NodeRefType = typename NodeType::NodeImplRefType;
  NodeAllocatorType alloc_{};
  NodeRefType root_{NodeType::nodeNullRef};
};


/////////////////////
// IMPLEMENTATIONS //
/////////////////////

template <typename PathType,typename NodeType,template<typename,std::size_t> class NodeStackType>
RadixTree<PathType,NodeType,NodeStackType>::RadixTree(RadixTree<PathType,NodeType,NodeStackType>&& o)
  : alloc_(std::move(o.alloc_))
  , root_(std::move(o.root_))
{
  o.root_ = NodeType::nodeNullRef;
}

template <typename PathType,typename NodeType,template<typename,std::size_t> class NodeStackType>
RadixTree<PathType,NodeType,NodeStackType>&
RadixTree<PathType,NodeType,NodeStackType>::operator=(RadixTree<PathType,NodeType,NodeStackType>&& o) {
  if (this == &o) { return *this; }
  alloc_.deleteRef(root_);
  root_ = std::move(o.root_);
  o.root_ = NodeAllocatorType::nullRef;
  alloc_ = std::move(o.alloc_);
  return *this;
}

template <typename CursorType>
inline void postOrderRemoveNodes(CursorType&& c) {
  static constexpr std::size_t radix = std::remove_reference<CursorType>::type::Radix;
  for (std::size_t i=0; i<radix; ++i) {
    if (c.canGoChildNode(i)) {
      c.goChild(i);
      postOrderRemoveNodes(std::forward<CursorType>(c));
      c.goParent();
    }
  }

  if (c.atNode()) {
    c.nodeValue().clear();
    c.removeNode();
  }

}

template <typename PathType,typename NodeType,template<typename,std::size_t> class NodeStackType>
RadixTree<PathType,NodeType,NodeStackType>::~RadixTree() {
  // walk tree and remove all the nodes post-order
  if (root_ != NodeAllocatorType::nullRef) {
    postOrderRemoveNodes(cursor());
    alloc_.deleteRef(root_);
    root_ = NodeAllocatorType::nullRef;
  }
}


template <typename PathType,typename NodeType,template<typename,std::size_t> class NodeStackType>
void RadixTree<PathType,NodeType,NodeStackType>::clear() {
  if (root_ != NodeAllocatorType::nullRef) {
    postOrderRemoveNodes(cursor());
    alloc_.deleteRef(root_);
  }
  root_ = alloc_.newRef();
}

} // namespace RadixTree
} // namespace Mapper
} // namespace Akamai

#endif
