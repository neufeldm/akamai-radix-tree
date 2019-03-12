#ifndef AKAMAI_MAPPER_RADIX_TREE_SIMPLE_RADIX_TREE_H_
#define AKAMAI_MAPPER_RADIX_TREE_SIMPLE_RADIX_TREE_H_

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

#include <unordered_map>

#include "RadixTree.h"
#include "NodeInterface.h"
#include "SimpleEdge.h"
#include "SimpleNodeImpl.h"
#include "SimplePath.h"
#include "SimpleStack.h"
#include "NodeAllocator.h"

/**
 * \file SimpleRadixTree.h
 * Type aliases for some expected common tree types.
 */

namespace Akamai {
namespace Mapper {
namespace RadixTree {

/**
 * \brief Tree node implementation based on our basic allocator and simple edge.
 */
template <std::size_t R,typename ValueT,std::size_t EdgeLen>
using SimpleTreeNodeImpl = SimpleNodeImpl<R,SimpleEdge<R,EdgeLen>,ValueT,
                                          typename AllocatorTraits<AllocatorNew>::RefType,
                                          AllocatorTraits<AllocatorNew>::nullRef>;

template <std::size_t R,typename ValueT,std::size_t EdgeLen>
using SimpleTreeNode = NodeInterface<AllocatorNew<SimpleTreeNodeImpl<R,ValueT,EdgeLen>>,SimpleTreeNodeImpl<R,ValueT,EdgeLen>>;


template <typename ValueT,std::size_t R,std::size_t MaxDepth,std::size_t EdgeLen>
using SimpleRadixTree = RadixTree<SimplePath<R,MaxDepth>,SimpleTreeNode<R,ValueT,EdgeLen>,SimpleFixedDepthStack>;

// Version of tree that uses a map to store the children instead of std::array

template <typename KeyT,typename ValT>
using DefaultNodeChildMap = std::unordered_map<KeyT,ValT>;

template <std::size_t R,typename ValueT,std::size_t EdgeLen,template <typename,typename> class ChildMap = DefaultNodeChildMap>
using SimpleTreeNodeImplMap = SimpleNodeImplMap<R,SimpleEdge<R,EdgeLen>,ValueT,
                                          typename AllocatorTraits<AllocatorNew>::RefType,
                                          AllocatorTraits<AllocatorNew>::nullRef,
                                          ChildMap>;

template <std::size_t R,typename ValueT,std::size_t EdgeLen>
using SimpleTreeNodeMap = NodeInterface<AllocatorNew<SimpleTreeNodeImplMap<R,ValueT,EdgeLen>>,SimpleTreeNodeImplMap<R,ValueT,EdgeLen>>;


template <typename ValueT,std::size_t R,std::size_t MaxDepth,std::size_t EdgeLen>
using SimpleRadixTreeMap = RadixTree<SimplePath<R,MaxDepth>,SimpleTreeNodeMap<R,ValueT,EdgeLen>,SimpleFixedDepthStack>;


} // namespace RadixTree
} // namespace Mapper
} // namespace Akamai

#endif
