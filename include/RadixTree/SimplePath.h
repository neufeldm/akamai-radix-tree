#ifndef AKAMAI_MAPPER_RADIX_TREE_SIMPLE_PATH_H_
#define AKAMAI_MAPPER_RADIX_TREE_SIMPLE_PATH_H_

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

#include <vector>
#include <stdexcept>

#include "RadixTreeUtils.h"

namespace Akamai {
namespace Mapper {
namespace RadixTree {

template <std::size_t R,std::size_t MD>
class SimplePath  {
public:
  static_assert(R >= 2,"Path radix must be >= 2");
  using PathUInt = Utils::SmallestUIntFor<R-1>;
  static constexpr std::size_t Radix = R;
  static constexpr std::size_t MaxDepth = MD;

  inline SimplePath(const std::vector<std::size_t>& steps);
  inline SimplePath(std::initializer_list<std::size_t> steps);
  SimplePath() { path_.reserve(MaxDepth); }
  virtual ~SimplePath() = default;

  /**
   * \brief Return length of path (not to be confused with the maximum depth of the path). 
   */ 
  std::size_t size() const { return path_.size(); }

  /**
   * \brief Return true if path length is equal to maximum possible depth.
   */
  bool full() const { return (MaxDepth == size()); }

  /**
   * \brief Returns true if path is empty.
   */
  bool empty() const { return path_.empty(); }

  /**
   * \brief Return the maximum possible path length. 
   */
  static constexpr std::size_t capacity() { return MaxDepth; }

  /**
   * \brief Clear the path.
   */ 
  void clear() { path_.clear(); }

  /**
   * \brief Add c to the path. Throws an error if path is full.
   * \param c step to add to path
   */ 
  inline void push_back(PathUInt c);

  /**
   * \brief Pop last step from the path. Throws an error if the path is empty.
   */
  inline void pop_back();
  inline void trim_front(std::size_t c);
  inline void trim_back(std::size_t c);

  /**
   * \brief Return the step at position p in the path.
   * \param p the position in the path to return (size_t)
   */
  PathUInt at(std::size_t p) const { return path_.at(p); }
  std::size_t operator[](std::size_t p) const { return at(p); }

  bool operator==(const SimplePath<R,MD>& other) const { return (path_ == other.path_); }
  bool operator!=(const SimplePath<R,MD>& other) const { return !(*this == other); }
  const std::vector<PathUInt>& path() const { return path_; }

  /**
   * \brief Expand the capacity of the path.
   * \param newsize the new capacity for the path
   */
  void resize(std::size_t newsize) { path_.resize(newsize); }

  /** 
   * \brief Return the difference between maximum depth and the current size of the path.
   */
  std::size_t suffixLength() const { return (MD - path_.size()); }
  

private:
  std::vector<PathUInt> path_;
};

/////////////////////
// IMPLEMENTATIONS //
/////////////////////

template <std::size_t R,std::size_t MD>
SimplePath<R,MD>::SimplePath(const std::vector<std::size_t>& steps) {
  if (steps.size() > MaxDepth) { throw std::out_of_range("desired size exceeds maximum"); }
  for (std::size_t step : steps) { if (step >= Radix) { throw std::out_of_range("step value exceeds radix"); } }
  path_ = steps;
}

template <std::size_t R,std::size_t MD>
SimplePath<R,MD>::SimplePath(std::initializer_list<std::size_t> steps) {
  if (steps.size() > MaxDepth) { throw std::out_of_range("initializer path exeeds maximum depth"); }
  path_.reserve(steps.size());
  for (std::size_t step : steps) {
    if (step >= Radix) { throw std::out_of_range("step value exceeds radix"); }
    push_back(step);
  }
}

template <std::size_t R,std::size_t MD>
void SimplePath<R,MD>::push_back(PathUInt c)  {
  if (full()) { throw std::length_error("SimplePath: push_back: path full"); }
  if (c >= Radix) { throw std::range_error("SimplePath: illegal child value pushed"); }
  path_.push_back(c);
}

template <std::size_t R,std::size_t MD>
void SimplePath<R,MD>::pop_back() {
  if (empty()) { throw std::length_error("SimplePath::pop_back: path empty"); }
  path_.pop_back();
}

template <std::size_t R,std::size_t MD>
void SimplePath<R,MD>::trim_front(std::size_t c) {
  if (c == 0) { return; }
  if (c > path_.size()) { throw std::range_error("SimplePath::trim_front: trim size too large"); }
  path_.erase(path_.begin(),path_.begin() + c);
}

template <std::size_t R,std::size_t MD>
void SimplePath<R,MD>::trim_back(std::size_t c) {
  if (c == 0) { return; }
  if (c > path_.size()) { throw std::range_error("SimplePath::trim_back: trim size too large"); }
  path_.resize(path_.size() - c);
}



} // namespace RadixTree
} // namespace Mapper
} // namespace Akamai

#endif
