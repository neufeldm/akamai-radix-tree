#ifndef AKAMAI_MAPPER_RADIX_TREE_TEST_PATHSORT_H_
#define AKAMAI_MAPPER_RADIX_TREE_TEST_PATHSORT_H_

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

/**
 * \file PathSort.h
 * 
 */

#include <stdint.h>
#include <type_traits>

/**
 * \brief Sort two Path objects in pre-order.
 * 
 * Note that the "ReverseChildern" parameter is useful if you want to perform a
 * "mirror image" traversal, still pre-order but starting at child 1 and then going
 * to child 0.
 */
template <typename PathType,bool ReverseChildren=false>
struct PathSortPreOrder {
  bool operator()(const PathType& a,const PathType& b) const {
    // if they're the same length we can just compare them directly
    if (a.size() == b.size()) { return lt(a,b); }
    // not the same length - truncate copies of both paths to the shortest of the two
    std::size_t minLength(std::min(a.size(),b.size()));
    PathType shortA(a);
    shortA.trim_back(a.size() - minLength);
    PathType shortB(b);
    shortB.trim_back(b.size() - minLength);
    // compare the actual bits in the path, break a tie by comparing the original lengths
    if (shortA.path() == shortB.path()) { return (a.size() < b.size()); }
    return lt(shortA,shortB);
  }
  static bool lt(const PathType& a,const PathType& b) { return (ReverseChildren ? a.path() > b.path() : a.path() < b.path()); }
};

/**
 * \brief Sort two Path objects in post-order.
 * 
 * This is a mirror image of PathSortPreOrder - using greater
 * than instead of less than to perform comparisons.
 */
template <typename PathType,bool ReverseChildren=false>
struct PathSortPostOrder {
  bool operator()(const PathType& a,const PathType& b) const {
    if (a.size() == b.size()) { return lt(a,b); }
    std::size_t minLength(std::min(a.size(),b.size()));
    PathType shortA(a);
    shortA.trim_back(a.size() - minLength);
    PathType shortB(b);
    shortB.trim_back(b.size() - minLength);
    if (shortA.path() == shortB.path()) { return (a.size() > b.size()); }
    return lt(shortA,shortB);
  }
  static bool lt(const PathType& a,const PathType& b) { return (ReverseChildren ? a.path() > b.path() : a.path() < b.path()); }
};

/**
 * \brief Sort two Path objects in in-order.
 * 
 * Similar to pre/post-order sorts but keeps track when it is at the
 * "middle" in-order node.
 */
template <typename PathType,bool ReverseChildren=false>
struct PathSortInOrder {
  static_assert((PathType::Radix % 2) == 0,"attempt to use in-order sort for odd radix");
  bool operator()(const PathType& a,const PathType& b) const {
    if (a.size() == b.size()) { return lt(a,b); }
    std::size_t minLength(std::min(a.size(),b.size()));
    PathType shortA(a);
    shortA.trim_back(a.size() - minLength);
    PathType shortB(b);
    shortB.trim_back(b.size() - minLength);

    if (shortA.path() == shortB.path()) {
      constexpr std::size_t ltInOrder = ((PathType::Radix/2) - 1);
      constexpr std::size_t gtInOrder = (PathType::Radix/2);
      if (a.size() < b.size()) {
        if (ReverseChildren) {
          return (b.at(minLength) < gtInOrder);
        } else {
          return (b.at(minLength) > ltInOrder);
        }
        return (b.at(minLength) == (ReverseChildren ? ltInOrder : gtInOrder));
      } else {
        if (ReverseChildren) {
          return (a.at(minLength) >= gtInOrder);
        } else {
          return (a.at(minLength) <= ltInOrder);
        }
      }
    }
    return lt(shortA,shortB);
  }
  static bool lt(const PathType& a,const PathType& b) { return (ReverseChildren ? a.path() > b.path() : a.path() < b.path()); }
};

/**
 * \brief Sort two Path objects in reverse depth order.
 */
template <typename PathType,bool ReverseChildren=false>
struct PathSortDeepestFirst {
  bool operator()(const PathType& a,const PathType& b) const { return (a.size() > b.size()); }
};


#endif