#ifndef AKAMAI_MAPPER_RADIX_TREE_NODE_VALUE_H_
#define AKAMAI_MAPPER_RADIX_TREE_NODE_VALUE_H_

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

#include <utility>

namespace Akamai {
namespace Mapper {
namespace RadixTree {

/**
 * \brief Class for wrapping the value part of a Node - read/only.
 * @see NodeInterface
 */
template <typename NodeInterfaceType>
class NodeValueRO {
public:
  static constexpr bool ValueIsCopy = NodeInterfaceType::ValueIsCopy;
  typedef typename NodeInterfaceType::ValueType ValueType;
  NodeValueRO() = default;
  NodeValueRO(const NodeInterfaceType& n) : node_(n) {}
  NodeValueRO(NodeInterfaceType&& n) : node_(std::move(n)) {}
  NodeValueRO(const NodeValueRO& other) : node_(other.node_) {}
  NodeValueRO(NodeValueRO&& other) : node_(std::move(other.node_)) {}
  NodeValueRO& operator=(const NodeValueRO& other) {
    node_ = other.node_;
    return *this;
  }
  NodeValueRO& operator=(NodeValueRO&& other) {
    node_ = std::move(other.node_);
    return *this;
  }

  bool operator==(const NodeValueRO& other) const {
    auto myValuePtr = getPtrRO();
    auto otherValuePtr = other.getPtrRO();
    if (myValuePtr == otherValuePtr) { return true; }
    if ((myValuePtr == nullptr) || (otherValuePtr == nullptr)) { return false; }
    return (*myValuePtr == *otherValuePtr);
  }

  bool operator!=(const NodeValueRO& other) const { return !(*this == other); }
  
  /**
   * \brief Return true if the node wrapper actually references an underlying node.
   */  
  bool atNode() const { return node_.exists(); }
  /**
   * \brief Return true if the node wrapper references an underlying node that also has a value set.
   */  
  bool atValue() const { return node_.hasValue(); }
  /**
   * \brief Get a const pointer to the actual underlying node value.
   */  
  const ValueType* getPtrRO() const { return (atValue() ? &(node_.value()) : nullptr); }
  /**
   * \brief Whether or not the node pointer value is a copy of what's stored in the node.
   * For node implementations that don't have a directly addressable value stored in them
   * (e.g. the value is kept as a subset of bits inside an integer) the implementation
   * may have to store a copy of the actual value. This means that if you get a pointer
   * to a value it may drift from the actual value set, and if you've got a read/write
   * pointer your updates to the referenced value won't be reflected in the actual node.
   */
  bool ptrIsCopy() const { return NodeInterfaceType::ValueIsCopy; }; 

protected:
  NodeInterfaceType node_{};
};

/**
 * \brief Class for wrapping the value part of a Node.
 * @see Node
 */
template <typename NodeInterfaceType>
class NodeValue
  : public NodeValueRO<NodeInterfaceType>
{
public:
  using Base = NodeValueRO<NodeInterfaceType>;
  typedef typename NodeInterfaceType::ValueType ValueType;
  NodeValue(NodeInterfaceType& nv) : Base(nv) {}
  NodeValue(NodeInterfaceType&& nv = NodeInterfaceType{}) : Base(std::move(nv)) {}
  NodeValue(const NodeValue& other) : Base(other) {}
  NodeValue(NodeValue&& other) : Base(std::move(other)) {}
  NodeValue& operator=(const NodeValue& other) {
    static_cast<Base&>(*this) = static_cast<const Base&>(other.node_);
    return *this;
  }
  NodeValue& operator=(NodeValue&& other) {
    static_cast<Base&>(*this) = std::move(static_cast<const Base&>(other.node_));
    return *this;
  }

  /**
   * \brief Set value if possible, i.e. the node wrapper is backed by an underlying node.
   */
  void set(const ValueType& v) { const_cast<NodeInterfaceType*>(&(this->node_))->setValue(v); }

 /**
   * \brief Set value if possible, move version.
   */
  void set(ValueType&& v) { const_cast<NodeInterfaceType*>(&(this->node_))->setValue(std::move(v)); }
  ValueType move() {
    ValueType v{};
    std::swap(v,*getPtrRW());
    clear();
    return v;
  }
  void swap(ValueType& v) { std::swap(v,*getPtrRW()); }
  /**
   * \brief Remove any value set.
   */
  void clear() { const_cast<NodeInterfaceType*>(&(this->node_))->clearValue(); }

  /**
   * \brief Get a mutable pointer to the stored node value - may not actually mutate the stored value.
   * @see ptrIsCopy()
   */  
  ValueType* getPtrRW() { return const_cast<ValueType*>(this->getPtrRO()); }
};

} //  namespace RadixTree
} //  namespace Mapper
} //  namespace Akamai

#endif
