#ifndef AKAMAI_MAPPER_RADIX_TREE_TEST_PATH_H_
#define AKAMAI_MAPPER_RADIX_TREE_TEST_PATH_H_

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
 * \file TestPath.h
 * 
 * This file contains simple implementations of a path/edge object compatible
 * with the RadixTree library. The primary goal here is to provide a basic foundation
 * for testing actual implementations of cursor, node, edge, and path classes.
 */

#include <stdint.h>
#include <type_traits>
#include <vector>
#include <stdexcept>

#include "CursorTestUtils.h"
#include "PathEdgeTestUtils.h"

// Simple path based on a vector, can move cursors along it
// as well as perform a shortest path move between it and a destination path.
template <std::size_t R,std::size_t MD>
class TestPath {
public:
  static constexpr std::size_t MaxDepth = MD;
  static constexpr std::size_t Radix = R;

  static_assert(R >= 2,"path radix must be at least 2");
  static_assert(MaxDepth > 0,"desired path depth == 0");

  using MyType = TestPath<R,MD>;
  
  TestPath() = default;
  TestPath(const std::vector<std::size_t>& steps) {
    if (steps.size() > MaxDepth) { throw std::out_of_range("desired size exceeds maximum"); }
    for (std::size_t step : steps) { if (step >= Radix) { throw std::out_of_range("step value exceeds radix"); } }
    path_ = steps;
  }
  TestPath(std::initializer_list<std::size_t> steps) {
    if (steps.size() > MaxDepth) { throw std::out_of_range("initializer path exeeds maximum depth"); }
    path_.reserve(steps.size());
    for (std::size_t step : steps) {
      if (step >= Radix) { throw std::out_of_range("step value exceeds radix"); }
      push_back(step);
    }
  }
  virtual ~TestPath() = default;

  bool operator==(const MyType& o) const { return (path_ == o.path_); }
  std::size_t size() const { return path_.size(); }
  bool empty() const { return (size() == 0); }
  std::size_t capacity() const { return MaxDepth; }
  std::size_t at(std::size_t l) const { return path_.at(l); }
  std::size_t operator[](std::size_t l) const { return at(l); }
  const std::vector<std::size_t>& path() const { return path_; }

  void trim_front(std::size_t c) { vectorTrimFront(path_,c); }
  void trim_back(std::size_t c) { vectorTrimBack(path_,c); }
  void push_back(std::size_t v) {
    if (v >= Radix) { throw std::out_of_range("push_back: invalid path step value"); }
    if (path_.size() == MaxDepth) { throw std::out_of_range("push_back: path full"); }
    path_.push_back(v);
  }

  void pop_back() { path_.pop_back(); }

  void clear() { path_.clear(); }
 
  // Move the cursor along the current path/length.
  template <typename CursorType>
  void moveCursor(CursorType&& c) const { for (std::size_t s : path_) { c.goChild(s); } }

  // Move the cursor to the root, then move it along the current
  // path/length.
  template <typename CursorType>
  void setCursor(CursorType&& c) const {
    cursorGotoRoot(std::forward<CursorType>(c));
    moveCursor(std::forward<CursorType>(c));
  }

  // Assuming the cursor is at the current path/length move it to the
  // destination path (d) along the shortest path.
  template <typename CursorType>
  void moveCursorTo(CursorType&& c,const MyType& d) const {
    std::size_t splitAt = commonPrefixSize(d);
    for (std::size_t i = splitAt; i < size(); ++i) { c.goParent(); }
    std::vector<std::size_t> newPath(d.path_.begin() + splitAt,d.path_.end());
    MyType destination(std::move(newPath));
    destination.moveCursor(std::forward<CursorType>(c));
  }
  template <typename CursorType>
  void moveCursorFrom(CursorType&& c,const MyType& s) const { s.moveCursorTo(std::forward<CursorType>(c),*this); }

  std::size_t commonPrefixSize(const MyType& o) const {
    if ((size() == 0) || (o.size() == 0)) { return 0; }
    // Normalize both path lengths to the smallest of the two
    std::size_t minLength(std::min(size(),o.size()));
    std::size_t commonLength(0);
    while ((commonLength < minLength) && at(commonLength) == o.at(commonLength)) { ++commonLength; }
    return commonLength;
  }

private:
  std::vector<std::size_t> path_;
};

#endif