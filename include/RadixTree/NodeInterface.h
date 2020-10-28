#ifndef AKAMAI_MAPPER_RADIX_TREE_NODE_INTERFACE_H_
#define AKAMAI_MAPPER_RADIX_TREE_NODE_INTERFACE_H_

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
#include <limits>

namespace Akamai {
namespace Mapper {
namespace RadixTree {

/**
 * \brief Delegates the node "interface" to an underlying node object reference and allocator.
 * 
 * This is a "shim" class that provides an object wrapper around an underlying reference (pointer)
 * to a node object implementation and its associated allocator. This implementation simply delegates
 * operations to the referenced node object. A bit like a "flyweight" pattern.
 */
template <typename AllocT,typename NodeImplT>
class NodeInterface
{
public:
  using NodeImplType = NodeImplT;
  using Edge = typename NodeImplType::EdgeType;
  using ValueType = typename NodeImplType::ValueType ;
  using NodeImplRefType = typename NodeImplType::NodeImplRefType;
  static constexpr NodeImplRefType nodeNullRef = NodeImplType::nodeNullRef;
  using AllocatorType = AllocT;
  static constexpr std::size_t Radix = NodeImplType::Radix;
  static constexpr bool ValueIsCopy = NodeImplType::ValueIsCopy;
  static constexpr std::size_t NoChild = std::numeric_limits<std::size_t>::max();

  explicit NodeInterface(const AllocatorType* a,NodeImplRefType n) : alloc_(a), nodeImplRef_(n) {}
  
  NodeInterface() = default;

  NodeInterface(const NodeInterface& other) : alloc_(other.alloc_), nodeImplRef_(other.nodeImplRef_) {}

  NodeInterface(NodeInterface&& other)
    : alloc_(other.alloc_)
    , nodeImplRef_(other.nodeImplRef_)
  {
    other.alloc_ = nullptr;
    other.nodeImplRef_ = AllocatorType::nullRef;
  }

  NodeInterface& operator=(const NodeInterface& other) {
    if (this == &other) { return *this; }
    alloc_ = other.alloc_;
    nodeImplRef_ = other.nodeImplRef_;
    return *this;
  }

  NodeInterface& operator=(NodeInterface&& other) {
    if (this == &other) { return *this; }
    alloc_ = other.alloc_;
    nodeImplRef_ = other.nodeImplRef_;
    other.alloc_ = nullptr;
    other.nodeImplRef_ = AllocatorType::nullRef;
    return *this;
  }

  /** 
   * \brief Returns true if referenced node object exists, false otherwise.
  */
  bool exists() const { return ((alloc_ != nullptr) && (nodeImplRef_ != AllocatorType::nullRef)); }
  
  
  const Edge& edge() const { return nodeImplPtr()->edge(); }
  Edge& edge() { return nodeImplPtr()->edge(); }
  
  /**
   * \brief Returns true if referenced node stores a value, false otherwise.
   */
  bool hasValue() const { return (exists() && nodeImplPtr()->hasValue()); }

  /**
   * \brief Return the value stored at the referenced node, read-only.
   */ 
  const ValueType& value() const { return nodeImplPtr()->value(); } 
  
  /**
   * \brief Return value stored at the referenced node.
   */
  ValueType& value() { return nodeImplPtr()->value(); }

  
  void setValue(const ValueType& v) { nodeImplPtr()->setValue(v); }
  void setValue(ValueType&& v) { nodeImplPtr()->setValue(std::move(v)); }
  
  /**
   * \brief Remove value stored at referenced node. 
   */
  void clearValue() { nodeImplPtr()->clearValue(); }

  /**
   * \brief Return referenced node object.
   */
  NodeImplRefType nodeImplRef() const { return nodeImplRef_; }

  /**
   * \brief Return the child (c) of the referenced node.
   * \param c the child to return (size_t)
   */
  NodeImplRefType getChild(std::size_t c) const { return nodeImplPtr()->getChild(c); }
  
  /**
   * \brief Set a new child for the referenced node object.
   * \param c the child to add to the node (size_t)
   * \param newChild referenced node type (NodeImplRefType)
   */
  NodeImplRefType setChild(std::size_t c,NodeImplRefType newChild) { return nodeImplPtr()->setChild(c,newChild); }

  /**
   * \brief Separate the specified child from the referenced node.
   * \param c the child to detach (size_t)
   */
  NodeImplRefType detachChild(std::size_t c) { return nodeImplPtr()->detachChild(c); }

  /**
   * \brief Return where or not specified child exists for the referenced node.
   * \param c the child (size_t)
   */
  bool hasChild(std::size_t c) const { return nodeImplPtr()->hasChild(c); }

  /**
   * \brief Return true if the node is a leaf, i.e. has no children.
   */
  bool isLeaf() const { return nodeImplPtr()->isLeaf(); }

private:
  const NodeImplType* nodeImplPtr() const { return alloc_->getPtr(nodeImplRef_); }
  NodeImplType* nodeImplPtr() { return alloc_->getPtr(nodeImplRef_); }
  const AllocatorType* alloc_{nullptr};
  NodeImplRefType nodeImplRef_{AllocatorType::nullRef};
};

}
}
}

#endif
