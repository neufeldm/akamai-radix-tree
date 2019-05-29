#ifndef AKAMAI_MAPPER_RADIX_TREE_BINARY_WORM_TREE_UINT_H_
#define AKAMAI_MAPPER_RADIX_TREE_BINARY_WORM_TREE_UINT_H_

#include <stdint.h>
#include <cstddef>
#include <string>
#include <array>
#include <type_traits>

#include "CursorIterator.h"
#include "MetaUtils.h"

#include "BinaryWORMTree.h"
#include "BinaryWORMTreeBuilder.h"
#include "BinaryWORMTreeGeneric.h"

namespace Akamai {
namespace Mapper {
namespace RadixTree {

struct BinaryWORMTreeUIntParams {
  std::size_t offsetSize{0};
  std::size_t valueSize{0};
  bool isLittleEndian{false};
};

template <typename PathT>
class BinaryWORMTreeUIntGeneric
  : public BinaryWORMTreeGeneric<PathT,uint64_t>
{
public:
  using GenericTreeBase = BinaryWORMTreeGeneric<PathT,uint64_t>;
  using TreeImplType = typename GenericTreeBase::TreeImplType;
  BinaryWORMTreeUIntGeneric() = default;
  BinaryWORMTreeUIntGeneric(const BinaryWORMTreeUIntParams& tp,std::unique_ptr<TreeImplType>&& t)
    : GenericTreeBase(t)
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


template <typename PathT>
using BinaryWORMCursorUIntGeneric = BinaryWORMCursorROGeneric<PathT,uint64_t>;

template <typename ActualImplT>
class BinaryWORMCursorUIntGenericImpl
  : public BinaryWORMCursorROGenericImpl<typename ActualImplT::PathType,uint64_t>
  , public ActualImplT
{
public:
  using ActualImpl = ActualImplT;
  using ValueType = uint64_t;  
  using PathType = typename ActualImpl::PathType;
  using GenericImpl = BinaryWORMCursorROGenericImpl<PathType,uint64_t>;
  BinaryWORMCursorUIntGenericImpl() = default;
  template <typename... ConstructorArgs>
  BinaryWORMCursorUIntGenericImpl(ConstructorArgs&&... cargs) : ActualImpl(std::forward<ConstructorArgs>(cargs)...) {}
  virtual ~BinaryWORMCursorUIntGenericImpl() = default;

  virtual bool atNode() const override { return ActualImpl::atNode(); }
  virtual bool atLeafNode() const override { return ActualImpl::atLeafNode(); }
  virtual bool atValue() const override { return ActualImpl::atValue(); }
  virtual bool goChild(std::size_t child) override { return ActualImpl::goChild(child); }
  virtual bool canGoChild(std::size_t child) const override { return ActualImpl::canGoChild(child); }
  virtual bool canGoChildNode(std::size_t child) const override { return ActualImpl::canGoChildNMode(child); }
  virtual bool goParent() override { return ActualImpl::goParent(); }
  virtual bool canGoParent() const override { return ActualImpl::canGoParent(); }
  virtual ValueType valueCopy() const override { return static_cast<uint64_t>(*(ActualImpl::nodeValueRO().getPtr())); }
};

template <typename PathT,bool LITTLEENDIAN,std::size_t OFFSETSIZE,std::size_t VALUESIZE>
using BinaryWORMCursorUInt = BinaryWORMCursorRO<PathT,BinaryWORMNodeUIntRO<LITTLEENDIAN,OFFSETSIZE,VALUESIZE>,SimpleFixedDepthStack>;

template <typename PathT,bool LITTLEENDIAN,std::size_t OFFSETSIZE,std::size_t VALUESIZE>
using BinaryWORMLookupCursorUInt = BinaryWORMLookupCursorRO<PathT,BinaryWORMNodeUIntRO<LITTLEENDIAN,OFFSETSIZE,VALUESIZE>>;

template <typename PathT,bool LITTLEENDIAN,std::size_t OFFSETSIZE,std::size_t VALUESIZE>
using BinaryWORMCursorUIntImpl = BinaryWORMCursorUIntGenericImpl<BinaryWORMCursorUInt<PathT,LITTLEENDIAN,OFFSETSIZE,VALUESIZE>>;

template <typename PathT,bool LITTLEENDIAN,std::size_t OFFSETSIZE,std::size_t VALUESIZE>
using BinaryWORMLookupCursorUIntImpl = BinaryWORMCursorUIntGenericImpl<BinaryWORMLookupCursorUInt<PathT,LITTLEENDIAN,OFFSETSIZE,VALUESIZE>>;

template <typename BufferT,typename PathT,bool LITTLEENDIAN,std::size_t OFFSETSIZE,std::size_t VALUESIZE>
class BinaryWORMTreeUIntGenericImpl
  : public BinaryWORMTreeGenericImpl<PathT,uint64_t>
  , public BinaryWORMTreeUInt<BufferT,PathT,LITTLEENDIAN,OFFSETSIZE,VALUESIZE>
{
public:
  using ActualImpl = BinaryWORMTreeUInt<BufferT,PathT,LITTLEENDIAN,OFFSETSIZE,VALUESIZE>;
  using GenericImpl = BinaryWORMTreeGenericImpl<PathT,uint64_t>;
  using GenericCursor = BinaryWORMCursorUIntGeneric<PathT>;
  using ActualWalkCursorImpl = BinaryWORMCursorUInt<PathT,LITTLEENDIAN,OFFSETSIZE,VALUESIZE>;
  using ActualLookupCursorImpl = BinaryWORMLookupCursorUInt<PathT,LITTLEENDIAN,OFFSETSIZE,VALUESIZE>;
  using CursorROType = typename GenericImpl::CursorROType;
  using LookupCursorROType = typename GenericImpl::LookupCursorROType;

  template <typename... ConstructorArgs>
  BinaryWORMTreeUIntGenericImpl(ConstructorArgs&&... cargs) : ActualImpl(std::forward<ConstructorArgs>(cargs)...) {}
  
  virtual CursorROType walkCursorRO() const override {
    std::unique_ptr<GenericCursor> genericCursor(new ActualWalkCursorImpl(ActualImpl::cursorRO()));
    return CursorROType(std::move(genericCursor));
  }
  virtual LookupCursorROType lookupCursorRO() const override {
    std::unique_ptr<GenericCursor> genericLookupCursor(new ActualLookupCursorImpl(ActualImpl::lookupCursorRO()));
    return CursorROType(std::move(genericLookupCursor));
  }
};

template <typename CursorT>
BinaryWORMTreeUIntParams findMinimumWORMTreeUIntParameters(const CursorT& c) {
  static_assert(std::is_unsigned<typename CursorT::ValueType>::value,
                "findMinimumWORMTreeUIntParameters: source tree must be of unsigned integer type");
  static_assert(sizeof(typename CursorT::ValueType) <= 8,
                "findMinimumWORMTreeUIntParameters: source tree value too large for uint64_t");

  using PathType = typename CursorT::PathType;
  BinaryWORMTreeUIntParams treeParams{};
  // do a dry run with 8 and 8 for the byte count, lttle/big endian doesn't matter
  BinaryWORMTreeUIntBuilder<std::vector<uint8_t>,PathType,false,sizeof(uint64_t),sizeof(uint64_t)> dryRunBuilder;
  auto treeIter = make_preorder_iterator<false,true>(std::forward<CursorT>(c));
  if (!dryRunBuilder.start(true)) {
    throw std::runtime_error("Unable to start dry-run build of WORM tree!");
  }
  uint64_t maxVal = 0;
  while (!treeIter.finished()) {
    bool hasLeftChild = treeIter->canGoChildNode(0);
    bool hasRightChild = treeIter->canGoChildNode(1);
    uint64_t value{};
    bool atValue = treeIter->atValue();
    if (atValue) {
      value = *(treeIter->nodeValueRO().getPtrRO());
      if (value > maxVal) { maxVal = value; }
    }
    if (!dryRunBuilder.addNode(treeIter->getPath(),atValue,atValue ? &value : nullptr,{hasLeftChild,hasRightChild})) {
      throw std::runtime_error("Unable to add node to dry-run WORM tree!");
    }
  }
  if (!dryRunBuilder.finish()) {
    throw std::runtime_error("Unable to finish dry-run WORM tree!");
  }
  std::size_t valueBitsRequired = 0;
  while (maxVal > 0) { ++valueBitsRequired; maxVal = (maxVal >> 1); }
  std::size_t valueBytesRequired = (valueBitsRequired + 7)/8;
  if (valueBytesRequired == 0) { valueBytesRequired = 1; }
  treeParams.valueSize = valueBytesRequired;
  auto treeStats = dryRunBuilder.treeStats();
  treeParams.offsetSize = treeStats.minBytesForOffset();
  return treeParams;
}


template <bool LITTLEENDIAN,std::size_t OFFSETSIZE,std::size_t VALUESIZE,typename CursorT, typename BufferT>
typename std::decay<BufferT>::type
buildBinaryWORMTreeUIntBuffer(const CursorT& cursor,BufferT&& buffer) {
  using CursorType =  CursorT;
  using PathType = typename CursorType::PathType;
  using BufferType = typename std::decay<BufferT>::type;
  BinaryWORMTreeUIntBuilder<BufferType,PathType,LITTLEENDIAN,OFFSETSIZE,VALUESIZE> wormBuilder;
  auto treeIter = make_preorder_iterator<false,true>(std::forward<CursorType>(cursor));
  if (!wormBuilder.start(true)) {
    throw std::runtime_error("Unable to start building WORM tree!");
  }
  uint64_t maxVal = 0;
  while (!treeIter.finished()) {
    bool hasLeftChild = treeIter->canGoChildNode(0);
    bool hasRightChild = treeIter->canGoChildNode(1);
    uint64_t value{};
    bool atValue = treeIter->atValue();
    if (atValue) {
      value = *(treeIter->nodeValueRO().getPtrRO());
      if (value > maxVal) { maxVal = value; }
    }
    if (!wormBuilder.addNode(treeIter->getPath(),atValue,atValue ? &value : nullptr,{hasLeftChild,hasRightChild})) {
      throw std::runtime_error("Unable to add node!");
    }
  }
  wormBuilder.finish();
  if (!wormBuilder.start(true)) {
    throw std::runtime_error("Unable to finish building WORM tree!");
  }

  return wormBuilder.extractBuffer();
}

template <typename PathT,bool LITTLEENDIAN,std::size_t OFFSETSIZE = sizeof(uint64_t),std::size_t VALUESIZE = sizeof(uint64_t)>
struct MakeBinaryWORMTreeUInt
  : public MakeBinaryWORMTreeUInt<PathT,LITTLEENDIAN,OFFSETSIZE,VALUESIZE - 1>
{
  static_assert(OFFSETSIZE <= sizeof(uint64_t),"OFFSETSIZE > sizeof(uint64_t)");
  static_assert(VALUESIZE <= sizeof(uint64_t),"VALUESIZE > sizeof(uint64_t)");

  // basic recursion case that does the work
  using ParentClass = MakeBinaryWORMTreeUInt<PathT,LITTLEENDIAN,OFFSETSIZE,VALUESIZE - 1>;
  template <typename BufferT>
  using TreeImpl = BinaryWORMTreeUIntGenericImpl<BufferT,PathT,LITTLEENDIAN,OFFSETSIZE,VALUESIZE>;

  template <typename CursorType, typename BufferT>
  BinaryWORMTreeUIntGeneric<PathT>
  from(const BinaryWORMTreeUIntParams& treeParams,const CursorType& cursor,BufferT&& buffer) {
    using BufferType = typename std::decay<BufferT>::type;
    if ((OFFSETSIZE == treeParams.offsetSize) &&
        (VALUESIZE == treeParams.valueSize) &&
        (LITTLEENDIAN == treeParams.isLittleEndian))
    {
      BufferType newBuffer = buildBinaryWORMTreeUIntBuffer<LITTLEENDIAN,OFFSETSIZE,VALUESIZE>(cursor,std::move(buffer));
      return TreeImpl<BufferType>(std::move(newBuffer));
    } else {
      return ParentClass::from(treeParams,cursor,std::move(buffer));
    }
  }
};

template <typename PathT,bool LITTLEENDIAN>
struct MakeBinaryWORMTreeUInt<PathT,LITTLEENDIAN,0,sizeof(uint64_t)> {
  // this terminates the template recursion - no need to run through 0 offset size
  template <typename BufferT>
  using TreeImpl = BinaryWORMTreeUIntGenericImpl<BufferT,PathT,LITTLEENDIAN,0,sizeof(uint64_t)>;
  template <typename CursorType, typename BufferT>
  BinaryWORMTreeUIntGeneric<PathT>
  from(const BinaryWORMTreeUIntParams& treeParams,const CursorType& cursor,BufferT&& buffer) {
    throw std::runtime_error("Invalid UInt binary WORM tree params: offsetsize " + std::to_string(treeParams.offsetSize) +
                             " valuesize " + std::to_string(treeParams.valueSize));
    return BinaryWORMTreeUIntGeneric<PathT>();
  }  
};

template <typename PathT,bool LITTLEENDIAN,std::size_t OFFSETSIZE>
struct MakeBinaryWORMTreeUInt<PathT,LITTLEENDIAN,OFFSETSIZE,0>
  : public MakeBinaryWORMTreeUInt<PathT,LITTLEENDIAN,OFFSETSIZE - 1,sizeof(uint64_t)>
{
  using ParentClass = MakeBinaryWORMTreeUInt<PathT,LITTLEENDIAN,OFFSETSIZE - 1,sizeof(uint64_t)>;
  template <typename BufferT>
  using TreeImpl = BinaryWORMTreeUIntGenericImpl<BufferT,PathT,LITTLEENDIAN,OFFSETSIZE,VALUESIZE>;
  // this restarts the inheritance chain at OFFSETSIZE - 1
  template <typename CursorType, typename BufferType>
  static TreeImpl<typename std::decay<BufferType>::type>
  from(const BinaryWORMTreeUIntParams& treeParams,const CursorType& cursor,BufferType&& buffer) {
    return ParentClass::from(treeParams,cursor,std::move(buffer));
  }
};


template <typename CursorT,typename BufferType = std::vector<uint8_t>>
BinaryWORMTreeUIntGeneric<typename std::decay<CursorT>::type::PathType>
buildWormTreeUIntGeneric(const BinaryWORMTreeUIntParams& treeParams,const CursorT& treeCursor,BufferType&& buffer)
{
  static_assert(std::is_unsigned<typename CursorT::ValueType>::value,
                "buildWORMTreeUIntBuffer: source tree must be of unsigned integer type");
  static_assert(sizeof(typename CursorT::ValueType) <= 8,
                "buildWORMTreeUIntBuffer: source tree value too large for uint64_t");
  if (treeParams.isLittleEndian) {
    return MakeBinaryWORMTreeUIntBuffer<true>::from(treeParams,treeCursor,std::move(buffer));
  } else {
    return MakeBinaryWORMTreeUIntBuffer<false>::from(treeParams,treeCursor,std::move(buffer));
  }
}

template <typename CursorT,typename BufferT = std::vector<uint8_t>>
BinaryWORMTreeUIntGeneric<typename std::decay<CursorT>::type::PathType>
buildWORMTreeUIntGeneric(const BinaryWORMTreeUIntParams& treeParams,const CursorT& sourceCursor)
{
  using TreeType = BinaryWORMTreeUIntGeneric<typename std::decay<CursorT>::type::PathType>;
  BufferT newBuffer{};
  newBuffer = buildWORMTreeUIntBuffer(treeParams,sourceCursor,std::move(newBuffer));
  TreeType();
}


} // namespace RadixTree
} // namespace Mapper
} // namespace Akamai

#endif