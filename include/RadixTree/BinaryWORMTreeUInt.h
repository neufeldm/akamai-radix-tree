#ifndef AKAMAI_MAPPER_RADIX_TREE_BINARY_WORM_TREE_UINT_H_
#define AKAMAI_MAPPER_RADIX_TREE_BINARY_WORM_TREE_UINT_H_

#include <stdint.h>
#include <cstddef>
#include <string>
#include <array>

#include "BinaryWORMTree.h"
#include "BinaryWORMTreeBuilder.h"

namespace Akamai {
namespace Mapper {
namespace RadixTree {

template <bool LITTLEENDIAN,std::size_t OFFSETSIZE,std::size_t VALUESIZE>
using BinaryWORMNodeUIntWO = BinaryWORMNodeWO<OFFSETSIZE,LITTLEENDIAN,BinaryWORMReadWriteUInt<VALUESIZE,LITTLEENDIAN>>;

template <bool LITTLEENDIAN,std::size_t OFFSETSIZE,std::size_t VALUESIZE>
using BinaryWORMNodeUIntRO = BinaryWORMNodeRO<OFFSETSIZE,LITTLEENDIAN,BinaryWORMReadWriteUInt<VALUESIZE,LITTLEENDIAN>>;

template <typename BufferT,typename PathT,bool LITTLEENDIAN,std::size_t OFFSETSIZE,std::size_t VALUESIZE>
using BinaryWORMTreeUIntBuilder = BinaryWORMTreeBuilder<BufferT,PathT,BinaryWORMNodeUIntWO<LITTLEENDIAN,OFFSETSIZE,VALUESIZE>>;

template <typename BufferT,typename PathT,bool LITTLEENDIAN,std::size_t OFFSETSIZE,std::size_t VALUESIZE>
using BinaryWORMTreeUInt = BinaryWORMTree<BufferT,PathT,BinaryWORMNodeUIntRO<LITTLEENDIAN,OFFSETSIZE,VALUESIZE>>;

template <typename TreeNodeIter>
void findMinimumUIntWORMParameters(TreeNodeIter&& treeNodeIter) {
  
}

template <typename TreeNodeIter,typename BufferType>
BufferType buildUIntWormTree(TreeNodeIter&& treeNodeIter, BufferType&& buffer,
                             bool littleEndian,std::size_t offsetSize,std::size_t uintValueSize) {

}

} // namespace RadixTree
} // namespace Mapper
} // namespace Akamai

#endif