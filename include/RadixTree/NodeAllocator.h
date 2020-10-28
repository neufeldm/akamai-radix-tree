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
