#ifndef AKAMAI_MAPPER_RADIX_TREE_ALLOCATOR_H_
#define AKAMAI_MAPPER_RADIX_TREE_ALLOCATOR_H_

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

#include <stdexcept>
#include <vector>
#include <cstring>
#include <limits>

/**
 * \file Custom allocator types for radix tree nodes.
 */

namespace Akamai {
namespace Mapper {
namespace RadixTree {

/**
 * \brief Allocator type traits class specifying the reference type and "null" value.
 *
 * We need a type traits class here instead of just using the types/constants defined
 * within each allocator to break a type recursion. In order for the node class to work
 * it needs to know the allocator in use as well as the reference type and null value.
 * However, for the allocator class to work it needs to know the type of the node that
 * it is allocating, which in turn depends on the type of allocator used. To break
 * the recursion we put the essential information (node reference type and null value)
 * into a separate traits class.
 */
template <template <typename> class Alloc>
struct AllocatorTraits {};

/**
 * \brief Type traits for tying allocator and node reference types together.
 *
 * For allocators that use simple pointers to nodes, we'd like to be able to have the
 * actual pointer type used for node children and tree roots. However, we're stuck
 * using void* (or some other type that doesn't depend on the node type) in our allocators.
 * To allow this we'll have a separate traits class that specifies the reference/pointer type
 * that a node implementation should use internally for its children. By default we use
 * whatever reference type is specified by the allocator, other allocators can specialize.
 * Note that we don't specialize explicitly on the allocator type, just the reference type.
 */
template <typename NodeImplType,typename NodeRef,NodeRef nullRefVal>
struct AllocatorNodeRefTraits {
  using Type = NodeRef;
  static constexpr NodeRef nullRef = nullRefVal;
};

/**
 * \brief Node reference specialization for simple pointer allocators.
 *
 * For allocators that just hand back simple pointers to the allocated object, e.g. "new" and "delete",
 * we can use the actual node pointer type. If you've got an allocator that does this, then
 * you can use void* and nullptr for your object type and null reference.
 */
template <typename NodeImplType>
struct AllocatorNodeRefTraits<NodeImplType,void*,nullptr> {
  using Type = NodeImplType*;
  static constexpr NodeImplType* nullRef = nullptr;
};


/**
 * \brief Baseline allocator that wraps new/delete.
 */
template <typename ObjType>
class AllocatorNew
{
public:
  using RefType = ObjType*;
  static constexpr RefType nullRef = nullptr;

  template <typename... Args>
  RefType newRef(Args&&... a) { return new ObjType{std::forward<Args>(a)...}; }
  void deleteRef(RefType ref) { delete static_cast<ObjType*>(ref); }
  static ObjType* getPtr(RefType ref) { return static_cast<ObjType*>(ref); }

};

template <>
struct AllocatorTraits<AllocatorNew> {
  using RefType = void*;
  static constexpr RefType nullRef = nullptr;
  static constexpr bool IsDirectPtr = true;
};

}
}
}

#endif
