#ifndef AKAMAI_MAPPER_RADIX_TREE_SIMPLE_NODE_IMPL_H_
#define AKAMAI_MAPPER_RADIX_TREE_SIMPLE_NODE_IMPL_H_

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

#include <cstddef>
#include <array>

namespace Akamai {
namespace Mapper {
namespace RadixTree {

/**
 * \brief Basic information for a node in a simple "tree-like" RadixTree implementation.
 *
 * Each node keeps references to its children, an edge path, and some value.
 * Note that we've templated on reference type here instead of using regular
 * pointers. This bit of indirection lets us swap in custom allocators of all sorts,
 * e.g. a reference could just be an integer offset into a flat slab of nodes instead of
 * a regular pointer.
 */
template <std::size_t R,typename EdgeT,typename NodeRef,NodeRef nullRef>
class SimpleNodeImplBase
{
public:
  static_assert(R >= 2,"Node radix must be >= 2");
  static constexpr std::size_t NoChild = std::numeric_limits<std::size_t>::max();
  SimpleNodeImplBase() = default;
  ~SimpleNodeImplBase() = default;

  const EdgeT& edge() const { return edge_; }
  EdgeT& edge() { return edge_; }
  
private:
  EdgeT edge_{};
};

  // Add routines for handling an embedded value
template <std::size_t R,typename EdgeT,typename ValueT,typename NodeRef,NodeRef nullRef>
class SimpleNodeImplBaseValue
  : public SimpleNodeImplBase<R,EdgeT,NodeRef,nullRef>
{
public:
  SimpleNodeImplBaseValue() = default;
  ~SimpleNodeImplBaseValue() = default;

  bool hasValue() const { return hasValue_; }

  const ValueT& value() const { return value_; }
  ValueT& value() { return value_; }
  
  void setValue(const ValueT& v) {
    this->hasValue_ = true;
    value_ = v;
  }

  void setValue(ValueT&& v) {
    this->hasValue_ = true;
    value_ = std::move(v);
  }

  void clearValue() {
    this->hasValue_ = false;
    value_ = ValueT{};
  }

private:
  bool hasValue_{false};
  ValueT value_;
};

/**
 * \brief Specialize for bool - no point in storing full extra value if we don't need to.
 */
template <std::size_t R,typename EdgeT,typename NodeRef,NodeRef nullRef>
class SimpleNodeImplBaseValue<R,EdgeT,bool,NodeRef,nullRef>
  : public SimpleNodeImplBase<R,EdgeT,NodeRef,nullRef>
{
public:
  SimpleNodeImplBaseValue() = default;
  ~SimpleNodeImplBaseValue() = default;

  const bool& value() const { return this->hasValue_; }
  bool& value() { return this->hasValue_; }
  void setValue(bool v) { this->hasValue_ = v; }
  void clearValue() { this->hasValue_ = false; }
  bool hasValue() const { return hasValue_; }

private:
  bool hasValue_{false};
};

  // finally add routines for handling children
template <std::size_t R,typename EdgeT,typename ValueT,typename NodeRef,NodeRef nullRef>
class SimpleNodeImpl
  : public SimpleNodeImplBaseValue<R,EdgeT,ValueT,NodeRef,nullRef>
{
public:
  using EdgeType = EdgeT;
  using NodeImplRefType = NodeRef;
  static constexpr NodeImplRefType nodeNullRef = nullRef;
  static constexpr std::size_t Radix = R;
  static constexpr bool ValueIsCopy = false;
  using ValueType = ValueT;

  SimpleNodeImpl() = default;
  ~SimpleNodeImpl() = default;

  NodeRef getChild(std::size_t c) const { return children_.at(c); }
  NodeRef setChild(std::size_t c,NodeRef newChild) {
    NodeRef prevChild = children_.at(c);
    children_[c] = newChild;
    return prevChild;
  }
  NodeRef detachChild(std::size_t c) {
    auto prevChild = children_.at(c);
    children_[c] = nullRef;
    return prevChild;
  }
  bool hasChild(std::size_t c) const { return (children_.at(c) != nullRef); }
  bool isLeaf() const {
    for (std::size_t c=0;c<R;++c) { if (hasChild(c)) { return false; } }
    return true; 
  }

private:
  // Children - store as simple array
  std::array<NodeRef,R> children_;
};

/**
 * \brief Node implementation that stores children in a map object.
 * More efficient than an array if you have a large possible child set but
 * a relatively sparse actual child count.
 */
template <std::size_t R,typename EdgeT,typename ValueT,typename NodeRef,NodeRef nullRef,template <typename,typename> class ChildMapT>
class SimpleNodeImplMap
  : public SimpleNodeImplBaseValue<R,EdgeT,ValueT,NodeRef,nullRef>
{
public:
  using EdgeType = EdgeT;
  using NodeImplRefType = NodeRef;
  static constexpr NodeImplRefType nodeNullRef = nullRef;
  static constexpr std::size_t Radix = R;
  static constexpr bool ValueIsCopy = false;
  using ValueType = ValueT;

  SimpleNodeImplMap() = default;
  ~SimpleNodeImplMap() = default;

  NodeRef getChild(std::size_t c) const {
    if (c >= R) { throw std::range_error("getChild() - child out of bounds"); }
    auto it = children_.find(c);
    if (it == children_.end()) { return nullRef; }
    return it->second;
  }
  NodeRef setChild(std::size_t c,NodeRef newChild) {
    if (newChild == nullRef) { return detachChild(c); }
    if (c >= R) { throw std::range_error("setChild() - child out of bounds"); }
    NodeRef prevChild{nullRef};
    auto it = children_.find(c);
    if (it != children_.end()) {
      prevChild = it->second;
      it->second = newChild;
    }
    else { children_[c] = newChild; }    
    return prevChild;
  }
  NodeRef detachChild(std::size_t c) {
    if (c >= R) { throw std::range_error("detachChild() - child out of bounds"); }
    NodeRef prevChild{nullRef};
    auto it = children_.find(c);
    if (it != children_.end()) {
      prevChild = it->second;
      children_.erase(it);
    }
    return prevChild;
  }
  bool hasChild(std::size_t c) const {
    auto it = children_.find(c);
    if (it == children_.end()) { return false; }
    return (it->second != nullRef);
  }

  bool isLeaf() const { return children_.empty(); }

private:
  ChildMapT<std::size_t,NodeRef> children_{};
};


}
}
}

#endif
