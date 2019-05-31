#ifndef AKAMAI_MAPPER_RADIX_TREE_BINARY_WORM_TREE_UINT_H_
#define AKAMAI_MAPPER_RADIX_TREE_BINARY_WORM_TREE_UINT_H_

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

#include "BinaryWORMTree.h"
#include "BinaryWORMTreeGeneric.h"

/**
 * \file BinaryWORMTreeUInt.h
 * 
 * An expected extremely common use case for the binary WORM tree
 * is to store unsigned integers. The generic interface eases the use
 * of the WORM tree format for compact serialization. It becomes much
 * easier to deal with WORM trees that use different numbers of bytes
 * to represent offsets and values.
 */

namespace Akamai {
namespace Mapper {
namespace RadixTree {

/**
 * \brief Contains the size/value/endian parameters for a Binary WORM tree.
 */
struct BinaryWORMTreeUIntParams {
  std::size_t offsetSize{0}; ///< Size in bytes of offset unsigned integer.
  std::size_t valueSize{0}; ///< Size in bytes of value unsigned integer.
  bool isLittleEndian{false}; ///< Whether the offset and value are little endian.
};

/**
 * \brief Tree wrapper specialized for UInt value WORM trees.
 * 
 * Stores underlying properties of the tree buffer as metadata.
 */
template <typename PathT>
class BinaryWORMTreeUIntGeneric
  : public BinaryWORMTreeGeneric<PathT,uint64_t>
{
public:
  using GenericTreeBase = BinaryWORMTreeGeneric<PathT,uint64_t>;
  using TreeImplType = typename GenericTreeBase::TreeImplType;
  BinaryWORMTreeUIntGeneric() = default;
  BinaryWORMTreeUIntGeneric(const BinaryWORMTreeUIntParams& tp,std::unique_ptr<TreeImplType>&& t)
    : GenericTreeBase(std::move(t))
    , treeParams_(tp)
  {}
  virtual ~BinaryWORMTreeUIntGeneric() = default;
  const BinaryWORMTreeUIntParams& treeParams() const { return treeParams_; }

private:
  BinaryWORMTreeUIntParams treeParams_{};
};

template <bool LITTLEENDIAN,std::size_t OFFSETSIZE,std::size_t VALUESIZE>
using BinaryWORMNodeUIntWO = BinaryWORMNodeWO<OFFSETSIZE,LITTLEENDIAN,BinaryWORMReadWriteUInt<VALUESIZE,LITTLEENDIAN>>;

template <bool LITTLEENDIAN,std::size_t OFFSETSIZE,std::size_t VALUESIZE>
using BinaryWORMNodeUIntRO = BinaryWORMNodeRO<OFFSETSIZE,LITTLEENDIAN,BinaryWORMReadWriteUInt<VALUESIZE,LITTLEENDIAN>>;

template <typename BufferT,typename PathT,bool LITTLEENDIAN,std::size_t OFFSETSIZE,std::size_t VALUESIZE>
using BinaryWORMTreeUIntBuilder = BinaryWORMTreeBuilder<BufferT,PathT,BinaryWORMNodeUIntWO<LITTLEENDIAN,OFFSETSIZE,VALUESIZE>>;

template <typename BufferT,typename PathT,bool LITTLEENDIAN,std::size_t OFFSETSIZE,std::size_t VALUESIZE>
using BinaryWORMTreeUInt = BinaryWORMTree<BufferT,PathT,BinaryWORMNodeUIntRO<LITTLEENDIAN,OFFSETSIZE,VALUESIZE>>;

/**
 * \brief Generic UINT cursor type, uses uint64_t as value so that up to 8 bytes per value integer may be used.
 */
template <typename PathT>
using BinaryWORMCursorUIntGeneric = BinaryWORMCursorROGeneric<PathT,uint64_t>;

/**
 * \brief Wraps an actual WORM UINT cursor instance inside of the generic cursor interface.
 */
template <typename ActualImplT>
class BinaryWORMCursorUIntGenericImpl
  : public BinaryWORMCursorROGenericImpl<typename ActualImplT::PathType,uint64_t>
{
public:
  using ValueType = uint64_t;  
  using PathType = typename ActualImplT::PathType;
  using GenericImpl = BinaryWORMCursorROGenericImpl<PathType,uint64_t>;
  BinaryWORMCursorUIntGenericImpl() = default;
  template <typename... ConstructorArgs>
  BinaryWORMCursorUIntGenericImpl(ConstructorArgs&&... cargs) : actualCursor_(std::forward<ConstructorArgs>(cargs)...) {}
  virtual ~BinaryWORMCursorUIntGenericImpl() = default;

  virtual bool atNode() const override { return actualCursor_.atNode(); }
  virtual bool atLeafNode() const override { return actualCursor_.atLeafNode(); }
  virtual bool atValue() const override { return actualCursor_.atValue(); }
  virtual bool goChild(std::size_t child) override { return actualCursor_.goChild(child); }
  virtual bool canGoChild(std::size_t child) const override { return actualCursor_.canGoChild(child); }
  virtual bool canGoChildNode(std::size_t child) const override { return actualCursor_.canGoChildNode(child); }
  virtual bool goParent() override { return actualCursor_.goParent(); }
  virtual bool canGoParent() const override { return actualCursor_.canGoParent(); }
  virtual PathType getPath() const override { return actualCursor_.getPath(); }
  virtual ValueType valueCopy() const override { return static_cast<uint64_t>(*(actualCursor_.nodeValueRO().getPtrRO())); }
  virtual GenericImpl* copy() const override { return new BinaryWORMCursorUIntGenericImpl<ActualImplT>(actualCursor_); }

  const ActualImplT& actualCursor() const { return actualCursor_; }
  ActualImplT& actualCursor() { return actualCursor_; }

private:
  ActualImplT actualCursor_{};
};

template <typename PathT,bool LITTLEENDIAN,std::size_t OFFSETSIZE,std::size_t VALUESIZE>
using BinaryWORMCursorUInt = BinaryWORMCursorRO<PathT,BinaryWORMNodeUIntRO<LITTLEENDIAN,OFFSETSIZE,VALUESIZE>,SimpleFixedDepthStack>;

template <typename PathT,bool LITTLEENDIAN,std::size_t OFFSETSIZE,std::size_t VALUESIZE>
using BinaryWORMLookupCursorUInt = BinaryWORMLookupCursorRO<PathT,BinaryWORMNodeUIntRO<LITTLEENDIAN,OFFSETSIZE,VALUESIZE>>;

template <typename PathT,bool LITTLEENDIAN,std::size_t OFFSETSIZE,std::size_t VALUESIZE>
using BinaryWORMCursorUIntImpl = BinaryWORMCursorUIntGenericImpl<BinaryWORMCursorUInt<PathT,LITTLEENDIAN,OFFSETSIZE,VALUESIZE>>;

template <typename PathT,bool LITTLEENDIAN,std::size_t OFFSETSIZE,std::size_t VALUESIZE>
using BinaryWORMLookupCursorUIntImpl = BinaryWORMCursorUIntGenericImpl<BinaryWORMLookupCursorUInt<PathT,LITTLEENDIAN,OFFSETSIZE,VALUESIZE>>;

/**
 * \brief Wraps WORM UINT tree implementation in generic wrapper.
 */
template <typename BufferT,typename PathT,bool LITTLEENDIAN,std::size_t OFFSETSIZE,std::size_t VALUESIZE>
class BinaryWORMTreeUIntGenericImpl
  : public BinaryWORMTreeGenericImpl<PathT,uint64_t>
{
public:
  using ActualImpl = BinaryWORMTreeUInt<BufferT,PathT,LITTLEENDIAN,OFFSETSIZE,VALUESIZE>;
  using GenericImpl = BinaryWORMTreeGenericImpl<PathT,uint64_t>;
  using GenericCursor = BinaryWORMCursorUIntGeneric<PathT>;
  using ActualWalkCursorImpl = BinaryWORMCursorUIntImpl<PathT,LITTLEENDIAN,OFFSETSIZE,VALUESIZE>;
  using ActualLookupCursorImpl = BinaryWORMLookupCursorUIntImpl<PathT,LITTLEENDIAN,OFFSETSIZE,VALUESIZE>;
  using CursorROType = typename GenericImpl::CursorROType;
  using LookupCursorROType = typename GenericImpl::LookupCursorROType;

  template <typename... ConstructorArgs>
  BinaryWORMTreeUIntGenericImpl(ConstructorArgs&&... cargs) : actualTree_(std::forward<ConstructorArgs>(cargs)...) {}
  
  virtual CursorROType walkCursorRO() const override {
    std::unique_ptr<ActualWalkCursorImpl> cursorImpl(new ActualWalkCursorImpl(actualTree_.cursorRO()));
    return CursorROType(std::move(cursorImpl));
  }
  virtual LookupCursorROType lookupCursorRO() const override {
    std::unique_ptr<ActualLookupCursorImpl> lookupCursorImpl(new ActualLookupCursorImpl(actualTree_.lookupCursorRO()));
    return LookupCursorROType(std::move(lookupCursorImpl));
  }

  virtual const uint8_t* bytes() const override {
    return actualTree_.buffer().data();
  }

  virtual std::size_t bytesSize() const override {
    return actualTree_.buffer().size();
  }
private:
  ActualImpl actualTree_{};
};

} // namespace RadixTree
} // namespace Mapper
} // namespace Akamai

#endif