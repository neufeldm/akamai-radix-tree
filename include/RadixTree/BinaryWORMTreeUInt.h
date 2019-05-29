#ifndef AKAMAI_MAPPER_RADIX_TREE_BINARY_WORM_TREE_UINT_H_
#define AKAMAI_MAPPER_RADIX_TREE_BINARY_WORM_TREE_UINT_H_

#include <stdint.h>
#include <cstddef>
#include <string>
#include <array>
#include <type_traits>

#include "CursorIterator.h"

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
using BinaryWORMTreeUIntCursorGeneric = BinaryWORMCursorROGeneric<PathT,uint64_t>;

template <typename PathT>
class BinaryWORMTreeUIntGeneric
  : public BinaryWORMTreeGeneric<PathT,uint64_t>
{
public:
  using GenericTreeBase = BinaryWORMTreeGeneric<PathT,uint64_t>;
  BinaryWORMTreeUInt() = default;
  BinaryWORMTreeUInt(std::unique_ptr<TreeImplType>&& t,const BinaryWORMTreeUIntParams& tp)
    : GenericTreeBase(std::move(t))
    , treeParams_(tp)
  {}
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
  auto treeIter = make_preorder_iterator<false,true>(std::forward<CursorType>(c));
  dryRunBuilder.start(true);
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
    wormBuilder.addNode(treeIter->getPath(),atValue,atValue ? &value : nullptr,{hasLeftChild,hasRightChild});
  }
  dryRunBuilder.finish();
  std::size_t valueBitsRequired = 0;
  while (maxVal > 0) { ++valueBitsRequired; maxVal = (maxVal >> 1); }
  std::size_t valueBytesRequired = (valueBitsRequired + 7)/8;
  if (valueBytesRequired == 0) { valueBytesRequired = 1; }
  treeParams.valueSize = valueBytesRequired;
  auto treeStats = wormBuilder.treeStats();
  treeParams.offsetSize = treeStats.minBytesForOffset();
  return treeParams;
}

template <typename CursorType,typename BufferType>
BufferType
buildWormTreeUIntBuffer(const BinaryWORMTreeUIntParams& treeParams,const CursorType& treeCursor, BufferType&& buffer)
{
  static_assert(std::is_unsigned<typename CursorT::ValueType>::value,
                "buildWORMTreeUIntBuffer: source tree must be of unsigned integer type");
  static_assert(sizeof(typename CursorT::ValueType) <= 8,
                "buildWORMTreeUIntBuffer: source tree value too large for uint64_t");

}

template <typename CursorT,typename BufferT = std::vector<uint8_t>>
BinaryWORMTreeUIntGeneric<typename std::decay<CursorT>::type::PathType>
buildWORMTreeUIntGeneric(const BinaryWORMTreeUIntParams& treeParams,const CursorT& sourceCursor)
{
  static_assert(std::is_unsigned<typename CursorT::ValueType>::value,
                "buildWORMTreeUIntGeneric: source tree value must be unsigned integer");
  static_assert(sizeof(typename CursorT::ValueType) <= 8,
                "buildWORMTreeUIntGeneric: source tree value too large for uint64_t");
}


} // namespace RadixTree
} // namespace Mapper
} // namespace Akamai

#endif