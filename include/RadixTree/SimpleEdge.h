#ifndef AKAMAI_MAPPER_RADIX_TREE_SIMPLE_EDGE_H_
#define AKAMAI_MAPPER_RADIX_TREE_SIMPLE_EDGE_H_

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

#include <array>
#include <algorithm>
#include <utility>
#include <stdint.h>

#include "RadixTreeUtils.h"

namespace Akamai {
namespace Mapper {
namespace RadixTree {

/**
 * \brief Fixed length node edge, can handle any degree tree up to max(uint64_t).
 *
 * Uses a std::array of integers, one per step in the edge, each of the smallest
 * standard size that will hold the requested tree degree. More straightforward
 * than a bit-packed edge of arbitrary radix, but not as space efficient. For
 * binary trees you should probably be using the BinaryEdge class instead. 
 */
template <std::size_t R,std::size_t MD>
struct SimpleEdge  {
  static_assert(R >= 2,"Edge radix must be >= 2");
  using PathUInt = Utils::SmallestUIntFor<R-1>;
  
  /** \brief The degree of the radix tree */
  static constexpr std::size_t Radix = R;

  /** \brief maximum possible depth of the edge */
  static constexpr std::size_t MaxDepth = MD;

  SimpleEdge() = default;
  ~SimpleEdge() = default;

  /** \brief Return number of entries stored in the edge */
  std::size_t size() const { return size_; }

  /** 
   * \brief Return true if the current size of the edge is equal to the maximum depth.
   */
  bool full() const { return (size_ >= MaxDepth); }

  /**
   * \brief Return true if size of the edge is 0.
   */
  bool empty() const { return size_ == 0; }
 
  /**
   * \brief Return the maximum possible depth of the edge.
   */ 
  std::size_t capacity() const { return MaxDepth; }

  /**
   * \brief "Clear" the edge: set size to 0 and populate edge with zeros. 
   */
  inline void clear();

  /**
   * \brief Add step c to the end of the edge
   */
  inline void push_back(PathUInt c);
  
  /**
   * \brief Pop the last step off the edge.
   */  
  inline void pop_back();

  /**
   * \brief Return the element at index p in the edge.
   * \param p the step index within the edge to return
   */
  inline PathUInt at(std::size_t p) const;
  PathUInt operator[](std::size_t p) const { return at(p); }
  /**
   * \brief Two edges are equal if they've got the same size and sequence of steps.
   */
  inline bool operator ==(const SimpleEdge<R,MaxDepth>& other) const;
  bool operator !=(const SimpleEdge<R,MaxDepth>& other) const { return !(*this == other); }
 
  /**
   * \brief Trim n steps of the back of the edge - equivalent to calling pop_back() n times.
   * 
   * \param n the length to trim
   */
  inline void trim_back(std::size_t n);

  /**
   * \brief Trim n steps off the front of the edge.
   * \param number of steps to trim
   */
  inline void trim_front(std::size_t n);
  
  /**
   * \brief Returns true if edge is "covered by" the other edge.
   * 
   * For an edge to be "covered" means that the covering edge has at
   * least as many steps as the covered edge and that truncating the
   * covering edge to the length of the covered edge would result in
   * two equal edges.
   */
  inline bool coveredby(const SimpleEdge<R,MaxDepth>& other) const;

  /**
   * \brief Returns number of steps starting at the beginning that match between the two edges.
   */  
  inline size_t matching(const SimpleEdge<R, MaxDepth>& other) const;

private:
  std::array<PathUInt,MaxDepth> ext_{};
  std::size_t size_{0};
};



/////////////////////
// IMPLEMENTATIONS //
/////////////////////

template <std::size_t R,std::size_t MaxDepth>
void SimpleEdge<R,MaxDepth>::clear() {
  size_ = 0;
  ext_.fill(0);
}

template <std::size_t R,std::size_t MaxDepth>
void SimpleEdge<R,MaxDepth>::push_back(PathUInt c) {
  if (full()) { throw std::length_error("[SimpleEdge] push_back: edge full"); }
  ext_[size_++] = c;
}

template <std::size_t R,std::size_t MaxDepth>
void SimpleEdge<R,MaxDepth>::pop_back() {
  if (empty()) { throw std::length_error("[SimpleEdge] pop_back: edge empty"); }
  ext_[--size_] = 0;
}

template <std::size_t R,std::size_t MaxDepth>
typename SimpleEdge<R, MaxDepth>::PathUInt SimpleEdge<R,MaxDepth>::at(std::size_t p) const {
  if (empty() || (p >= size_)) { throw std::length_error("[SimpleEdge] at: offset out of range"); }
  return ext_[p];
}

template <std::size_t R,std::size_t MaxDepth>
void SimpleEdge<R,MaxDepth>::trim_back(std::size_t n) {
  if (size_ < n) { throw std::length_error("[SimpleEdge] trim_back: element count larger than size"); }
  if (size_ == n) {
    clear();
  } else {
    size_ -= n;
    std::fill(ext_.begin() + size_,ext_.end(),0);
  }
}

template <std::size_t R,std::size_t MaxDepth>
void SimpleEdge<R,MaxDepth>::trim_front(std::size_t n) {
  if (size_ < n) { throw std::length_error("[SimpleEdge] trim_front: element count larger than size"); }
  if (size_ == n) {
    clear();
  } else {
    size_ -= n;
    for (std::size_t i = 0; i < size_; ++i) { ext_[i] = ext_[i+n]; }
  }
}

template <std::size_t R,std::size_t MaxDepth>
bool SimpleEdge<R,MaxDepth>::operator==(const SimpleEdge<R,MaxDepth>& other) const {
  return ((size_ == other.size_) && ((size_ == 0) || coveredby(other)));
}

template <std::size_t R,std::size_t MaxDepth>
bool SimpleEdge<R,MaxDepth>::coveredby(const SimpleEdge<R,MaxDepth>& other) const {
  if (size_ == 0) { return true; }
  if (size_ <= other.size_) { return (matching(other) == size_); }
  return false;
}

template<std::size_t R, std::size_t MaxDepth>
std::size_t SimpleEdge<R, MaxDepth>::matching(const SimpleEdge<R, MaxDepth>& other) const {
  size_t max_len = std::min(size_, other.size_);
  for (size_t i = 0; i < max_len; i++) {
    if (ext_[i] != other.ext_[i]) { return i; }
  } 
  return max_len;
}

} // namespace RadixTree
} // namespace Mapper
} // namespace Akamai

#endif
