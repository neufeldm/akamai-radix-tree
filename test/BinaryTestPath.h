#ifndef AKAMAI_MAPPER_RADIX_TREE_TEST_BINARY_TEST_PATH_H_
#define AKAMAI_MAPPER_RADIX_TREE_TEST_BINARY_TEST_PATH_H_

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
 * \file BinaryTestPath.h
 * 
 * This file contains simple implementations of a path/edge object compatible
 * with the RadixTree library. The primary goal here is to provide a basic foundation
 * for testing actual implementations of cursor, node, edge, and path classes.
 */

#include <stdint.h>
#include <type_traits>
#include <stdexcept>

#include "CursorTestUtils.h"

// Simple path based on a single unsigned integer, can move cursors along it
// as well as perform a shortest path move between it and a destination path.
template <std::size_t MD=64,typename IntType=uint64_t>
class BinaryTestPath {
public:
  static_assert(!std::numeric_limits<IntType>::is_signed,"path integer type must be unsigned");
  static constexpr std::size_t MaxPossibleDepth = 8*sizeof(IntType);
  static constexpr std::size_t MaxDepth = MD;
  static constexpr std::size_t Radix = 2;

  static_assert(MaxDepth <= MaxPossibleDepth,"desired path depth too large");
  static_assert(MaxDepth > 0,"desired path depth == 0");

  using PathIntType = IntType;
  using MyType = BinaryTestPath<MD,IntType>;
  
  BinaryTestPath() = default;
  template <typename InitType>
  BinaryTestPath(InitType p) {
    if (p.size() > MaxDepth) { throw std::out_of_range{"length exceeds maximum"}; }
    for (std::size_t i = 0;i < p.size(); ++i) { push_back(p.at(i)); }
  }
  BinaryTestPath(std::initializer_list<std::size_t> bits) {
    if (bits.size() > MaxDepth) { throw std::out_of_range("initializer path exeeds maximum depth"); }
    size_ = bits.size();
    for (std::size_t curBit : bits) {
      if (curBit > 1) { throw std::out_of_range("initializer item > 1"); }
      path_ = (path_ << 1);
      if (curBit == 1) { path_ |= static_cast<IntType>(0x1); }
    }
  }
  BinaryTestPath(const std::vector<std::size_t>& bits) {
    if (bits.size() > MaxDepth) { throw std::out_of_range("initializer path exeeds maximum depth"); }
    size_ = bits.size();
    for (std::size_t curBit : bits) {
      if (curBit > 1) { throw std::out_of_range("initializer item > 1"); }
      path_ = (path_ << 1);
      if (curBit == 1) { path_ |= static_cast<IntType>(0x1); }
    }    
  }
  static MyType FromInt(IntType p,std::size_t d) { return MyType(p,d); }
  
  virtual ~BinaryTestPath() = default;

  bool operator==(const MyType& o) const { return ((path_ == o.path_) && (size_ == o.size_)); }

  IntType path() const { return path_; }
  std::size_t size() const { return size_; }
  std::size_t capacity() const { return MaxDepth; }
  std::size_t at(std::size_t l) const {
    if (l >= size_) { throw std::out_of_range{"invalid position"}; }
    return ((((0x1 << (size_ - l - 1)) & path_) == 0) ? 0 : 1);
  }
  std::size_t operator[](std::size_t l) const { return at(l); }

  void trim_front(std::size_t d) {
    if (d > size_) { throw std::out_of_range("trim_front: trim size too large"); }
    size_ -= d;
    // Want to remove the top "d" bits of our path - 
    // since we're unsigned we can shift them out of
    // the big end and then shift the remainder back down.
    std::size_t shiftSize = (MaxDepth - size_);
    path_ = ((path_ << shiftSize) >> shiftSize);
  }

  void trim_back(std::size_t d) {
    if (d > size_) { throw std::out_of_range("trim_back: trim size too large"); }
    path_ = (path_ >> d);
    size_ -= d;
  }

  void push_back(std::size_t v) {
    if (v > 1) { throw std::out_of_range("push_back: invalid path step value"); }
    if (size_ == MaxDepth) { throw std::out_of_range("push_back: path full"); }
    path_ = (path_ << 1);
    ++size_;
    if (v == 1) { path_ |= static_cast<IntType>(0x1); }
  }

  void pop_back() {
    if (size_ == 0) { throw std::out_of_range("pop_back: empty path"); }
    path_ = (path_ >> 1);
    --size_;
  }

  void clear() { size_ = 0; path_ = 0; }
 
  // Move the cursor along the current path/length.
  template <typename CursorType>
  void moveCursor(CursorType&& c) const {
    if (size_ == 0) { return; }
    IntType mask = (0x1 << (size_ - 1));
    for (std::size_t i=0; i < size_; ++i) {
      c.goChild(((path_ & mask) == 0) ? 0 : 1);
      mask = (mask >> 1);
    }
  }

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
    uint8_t splitAt = commonPrefixSize(d);
    for (uint8_t i = splitAt; i < size_; ++i) { c.goParent(); }
    MyType destination(d.path_, d.size_ - splitAt);
    destination.moveCursor(std::forward<CursorType>(c));
  }
  template <typename CursorType>
  void moveCursorFrom(CursorType&& c,const MyType& s) const {
    s.moveCursorTo(std::forward<CursorType>(c),*this);
  }

  std::size_t commonPrefixSize(const MyType& o) const {
    if ((size_ == 0) || (o.size_ == 0)) { return 0; }
    // Normalize both path lengths to the smallest of the two
    std::size_t minLength(std::min(size_,o.size_));
    IntType x((path_ >> (size_ - minLength)) ^ (o.path_ >> (o.size_ - minLength)));
    const IntType checkBit(static_cast<IntType>(0x1) << (minLength - 1));
    std::size_t commonLength(0);
    while (((checkBit & x) == 0) && (commonLength < minLength)) { ++commonLength; x = (x << 1); }
    return commonLength;
  }

  static constexpr IntType maskLower(uint8_t l) { return ((static_cast<IntType>(0x1) << l) - 1); }

  MyType shiftRight(std::size_t s) const { return MyType(path_ >> s,(s <= size_) ? (size_ - s) : 0); }
private:
  BinaryTestPath(IntType p,std::size_t s) : path_(p), size_(s) {}
  IntType path_{0};
  uint8_t size_{0};
};

using BinaryTestPath8 = BinaryTestPath<8,uint8_t>;
using BinaryTestPath16 = BinaryTestPath<16,uint16_t>;
using BinaryTestPath32 = BinaryTestPath<32,uint32_t>;
using BinaryTestPath64 = BinaryTestPath<64,uint64_t>;

#endif