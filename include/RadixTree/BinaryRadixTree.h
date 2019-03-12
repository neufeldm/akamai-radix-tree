#ifndef AKAMAI_MAPPER_RADIX_TREE_BINARY_RADIX_TREE_H_
#define AKAMAI_MAPPER_RADIX_TREE_BINARY_RADIX_TREE_H_

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

#include "NodeInterface.h"
#include "BinaryWordEdge.h"
#include "BinaryWordNode.h"
#include "SimpleNodeImpl.h"
#include "SimpleStack.h"
#include "BinaryPath.h"
#include "WordBlockAllocator.h"
#include "NodeAllocator.h"
#include "RadixTree.h"

namespace Akamai {
namespace Mapper {
namespace RadixTree {

/**
 * \brief A helper for declaring a simple (node/edge/pointer) binary tree node implementation.
 */
template <typename ValueT,typename EdgeT = SimpleBinaryWordEdge<uint32_t>,template <typename> class AllocatorT = AllocatorNew>
using BinaryTreeSimpleNodeImpl = SimpleNodeImpl<2,EdgeT,ValueT,
                                                typename AllocatorTraits<AllocatorT>::RefType,
                                                AllocatorTraits<AllocatorT>::nullRef>;


template <typename ValueT,typename EdgeT,template <typename> class AllocatorT = AllocatorNew>
using BinaryTreeNode = NodeInterface<AllocatorT<BinaryTreeSimpleNodeImpl<ValueT,EdgeT,AllocatorT>>,
                                     BinaryTreeSimpleNodeImpl<ValueT,EdgeT,AllocatorT>>;


/**
 * \brief A generic node/pointer/edge binary radix tree node, 32 bit integer for edge.
 */
template <typename ValueT,template <typename> class AllocatorT = AllocatorNew>
using BinaryTreeNode32 = BinaryTreeNode<ValueT,SimpleBinaryWordEdge<uint32_t>,AllocatorT>;

/**
 * \brief A generic node/pointer/edge binary radix tree node, 64 bit integer for edge.
 */
template <typename ValueT,template <typename> class AllocatorT = AllocatorNew>
using BinaryTreeNode64 = BinaryTreeNode<ValueT,SimpleBinaryWordEdge<uint64_t>,AllocatorT>;

template <typename ValueT,std::size_t MaxDepth,template <typename> class AllocatorT = AllocatorNew>
using BinaryRadixTree32 = RadixTree<BinaryPath<MaxDepth>,BinaryTreeNode32<ValueT,AllocatorT>,SimpleFixedDepthStack>;

template <typename ValueT,std::size_t MaxDepth,template <typename> class AllocatorT = AllocatorNew>
using BinaryRadixTree64 = RadixTree<BinaryPath<MaxDepth>,BinaryTreeNode64<ValueT,AllocatorT>,SimpleFixedDepthStack>;

/**
 * \brief Vector "word" based tree, each node is stored in 4 uint32_t or uint64_t values.
 * 
 * All nodes are stored contiguously in a vector, each node stores a single uint32_t or uint64_t
 * as its value depending on the underlying word type.
 */
template <typename WordType,std::size_t MaxDepth>
using BinaryWordTree = RadixTree<BinaryPath<MaxDepth>,BinaryWordNode<WordType,WordBlockVectorAllocator>,SimpleFixedDepthStack>;

template <std::size_t MaxDepth>
using BinaryWordTree32 = BinaryWordTree<uint32_t,MaxDepth>;

template <std::size_t MaxDepth>
using BinaryWordTree64 = BinaryWordTree<uint64_t,MaxDepth>;

/**
 * \brief Vector "word" based tree, each node is stored in 3 uint32_t or uint64_t values.
 * 
 * The stored value can be an integer type smaller than the underlying word type, e.g.
 * uint8_t or uint16_t for the uint32_t nodes, uint8_t,uint16_t, or uint32_t for the uint64_t nodes.
 * The bits used to store the values consume bits that would otherwise be used to store
 * edge paths. Storing a "bool" is specialized to only use a single bit, storing void
 * is specialized to store no extra bits. With the "void" tree the true/false value is
 * equivalent to the presence/absence of a value completely.
 */
template <typename ValueT,typename WordType,std::size_t MaxDepth>
using CompactBinaryWordTree = RadixTree<BinaryPath<MaxDepth>,CompactBinaryWordNode<ValueT,WordType,WordBlockVectorAllocator>,SimpleFixedDepthStack>;

template <std::size_t MaxDepth>
using CompactBinaryBoolTree32 = CompactBinaryWordTree<bool,uint32_t,MaxDepth>;

template <std::size_t MaxDepth>
using CompactBinaryBoolTree64 = CompactBinaryWordTree<bool,uint64_t,MaxDepth>;

template <std::size_t MaxDepth>
using CompactBinaryVoidTree32 = CompactBinaryWordTree<void,uint32_t,MaxDepth>;

template <std::size_t MaxDepth>
using CompactBinaryVoidTree64 = CompactBinaryWordTree<void,uint64_t,MaxDepth>;



} // namespace RadixTree
} // namespace Mapper
}

#endif
