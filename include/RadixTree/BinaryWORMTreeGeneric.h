#ifndef AKAMAI_MAPPER_RADIX_TREE_BINARY_WORM_TREE_GENERIC_H_
#define AKAMAI_MAPPER_RADIX_TREE_BINARY_WORM_TREE_GENERIC_H_

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
#include <memory>

/**
 * \file BinaryWORMTreeGeneric.h
 * 
 *  Generic wrappers to use with binary WORM trees. An intended use case for
 *  WORM trees is as a serializable/memory-mappable generic byte buffer.
 *  However, the WORM tree implementation in this library is heavily templated
 *  based on the size and endian of the underlying node offsets, making it awkward
 *  to seamlessly work with WORM tree buffers that have different parameters.
 *  This file contains some wrapper interfaces that can be used to hide the
 *  underlying details within polymorphism.
 */

namespace Akamai {
namespace Mapper {
namespace RadixTree {

/**
 * \brief Abstract interface for a WORM cursor.
 * 
 * This is largely the same as the baseline cursor interface except
 * for the "valueCopy" and "copy" methods. The valueCopy method assumes
 * that the underlying value stored in a WORM tree is small and cheap
 * to copy. Instead of providing a further polymorphic interface for
 * NodeValue a generic cursor can simply use the standard binary WORM
 * "copy" value.
 */
template <typename PathT,typename ValueT>
class BinaryWORMCursorROGenericImpl {
public:
  using PathType = PathT;
  using ValueType = ValueT;

  BinaryWORMCursorROGenericImpl() = default;
  virtual ~BinaryWORMCursorROGenericImpl() = default;

  virtual bool atNode() const = 0;
  virtual bool atLeafNode() const = 0;
  virtual bool atValue() const = 0;
  virtual bool goChild(std::size_t child) = 0;
  virtual bool canGoChild(std::size_t child) const = 0;
  virtual bool canGoChildNode(std::size_t child) const = 0;
  virtual bool goParent() = 0;
  virtual bool canGoParent() const = 0;
  virtual PathType getPath() const = 0;

  virtual ValueType valueCopy() const = 0;

  virtual BinaryWORMCursorROGenericImpl<PathType,ValueType>* copy() const = 0;
};

/**
 * \brief Holds a unique pointer to an underlying generic cursor implementation.
 * 
 * Having a class on the stack that wraps the generic cursor implementation allows
 * code using the generic wrappers to look the same as code using the specific
 * implementations.
 */
template <typename PathT,typename ValueT>
class BinaryWORMCursorROGeneric {
public:
  static constexpr std::size_t Radix = 2;
  static constexpr std::size_t MaxDepth = PathT::MaxDepth;
  using MyType = BinaryWORMCursorROGeneric<PathT,ValueT>;
  using PathType = PathT;
  using ValueType = ValueT;
  using NodeValueRO = BinaryWORMValueCopyRO<uint64_t>;
  using NodeValue = NodeValueRO;
  using NodeHeader = BinaryWORMNodeHeaderRO<sizeof(uint64_t),false>;
  using OffsetType = typename NodeHeader::OffsetType;
  using EdgeWordType = typename NodeHeader::EdgeWordType;
  using CursorImplType = BinaryWORMCursorROGenericImpl<PathT,ValueT>;
  
  BinaryWORMCursorROGeneric() = default;
  BinaryWORMCursorROGeneric(BinaryWORMCursorROGeneric&& o) = default;
  BinaryWORMCursorROGeneric(std::unique_ptr<CursorImplType>&& o) : cursorImpl_(std::move(o)) {}
  BinaryWORMCursorROGeneric(const BinaryWORMCursorROGeneric& o) : cursorImpl_(o.cursorImpl_->copy()) {}
  MyType& operator=(const MyType& o) = default;
  MyType& operator=(MyType&& o) = default;
  virtual ~BinaryWORMCursorROGeneric() = default;

  bool atNode() const { return cursorImpl_->atNode(); }
  bool atLeafNode() const { return cursorImpl_->atLeafNode(); }
  bool atValue() const { return cursorImpl_->atValue(); }
  bool goChild(std::size_t child) { return cursorImpl_->goChild(child); }
  bool canGoChild(std::size_t child) const { return cursorImpl_->canGoChild(child); }
  bool canGoChildNode(std::size_t child) const { return cursorImpl_->canGoChildNode(child); }
  PathType getPath() const { return cursorImpl_->getPath(); }
 
  bool goParent() { return cursorImpl_->goParent(); }
  bool canGoParent() const { return cursorImpl_->canGoParent(); }

  NodeValueRO nodeValueRO() const {
    if (cursorImpl_->atValue()) { return NodeValueRO(cursorImpl_->valueCopy()); }
    else { return NodeValueRO{}; }
  }
  NodeValue nodeValue() const { return nodeValueRO(); }

  ValueType valueCopy() const { return cursorImpl_->valueCopy(); }

  template <typename NewValueType>
  NewValueType valueCopyAs() const { return static_cast<NewValueType>(valueCopy()); }

private:
  std::unique_ptr<CursorImplType> cursorImpl_{};
};


/**
 * \brief Generic implementation interface for a binary WORM tree.
 */
template <typename PathT,typename ValueT>
class BinaryWORMTreeGenericImpl
{
public:
  using PathType = PathT;
  using CursorROType = BinaryWORMCursorROGeneric<PathT,ValueT>;
  using CursorType = CursorROType;
  using LookupCursorROType = CursorROType;
  using ValueTypeRO = typename CursorROType::ValueType;
  using ValueType = ValueTypeRO;
  
  BinaryWORMTreeGenericImpl() = default;
  virtual ~BinaryWORMTreeGenericImpl() = default;

  virtual CursorROType walkCursorRO() const = 0;
  virtual LookupCursorROType lookupCursorRO() const = 0;
};

template <typename PathT,typename ValueT>
class BinaryWORMTreeGeneric
{
public:
  using PathType = PathT;
  using CursorROType = BinaryWORMCursorROGeneric<PathT,ValueT>;
  using CursorType = CursorROType;
  using LookupCursorROType = CursorROType;
  using ValueTypeRO = typename CursorROType::ValueType;
  using ValueType = ValueTypeRO;
  using TreeImplType = BinaryWORMTreeGenericImpl<PathT,ValueT>;

  BinaryWORMTreeGeneric() = default;
  BinaryWORMTreeGeneric(std::unique_ptr<TreeImplType>&& t) : treeImpl_(std::move(t)) {}
  virtual ~BinaryWORMTreeGeneric() = default;

  CursorType cursor() const { return walkCursorRO(); }
  CursorROType cursorRO() const { return walkCursorRO(); }
  CursorROType walkCursorRO() const { return treeImpl_->walkCursorRO(); }
  LookupCursorROType lookupCursorRO() const { return treeImpl_->lookupCursorRO(); }

private:
  std::shared_ptr<TreeImplType> treeImpl_{};
};

} // namespace RadixTree
} // namespace Mapper
} // namespace Akamai

#endif