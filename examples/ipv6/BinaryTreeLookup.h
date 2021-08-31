#ifndef AKAMAI_MAPPER_RADIXTREE_BINARY_TREE_LOOKUP_H_
#define AKAMAI_MAPPER_RADIXTREE_BINARY_TREE_LOOKUP_H_

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
#include <stdexcept>
#include <cstddef>
#include <tuple>
#include <utility>
#include <unordered_map>

#include "BitPacking.h"
#include "CursorOps.h"
#include "RadixTreeUtils.h"

// This file implements a simple binary-tree based lookup object.
// 


// When returning a lookup value we'll often want to know both the value present
// and its actual depth in the tree. This is often done with a std::pair<>, but having
// explicit names for the value and depth help code readability. Conversions to/from std::pair<>
// are provided if std::pair<> functionality is desired.
// N.B. we return a copy of the contained value - probably not suitable for expensive objects.
template <typename ValueT>
struct ValueDepth {
  using Value = ValueT;
  Value value{};
  std::size_t depth{0};
  bool foundValue{false};
  ValueDepth() = default;
  ValueDepth(const Value& v,std::size_t d) : value(v), depth(d), foundValue(true) {}
  ValueDepth(Value&& v,std::size_t d) : value(std::move<Value>(v)), depth(d), foundValue(true) {}
  ValueDepth(const std::pair<Value,std::size_t>& vdp) : value(vdp.first), depth(vdp.second),foundValue(true) {}
  std::pair<Value,std::size_t> operator()() const { return std::make_pair(value,depth); }
};

// Our lookup object takes raw byte pointers to binary paths. We assume
// that these paths are laid out like IPv6 addresses in memory, big endian
// order bytes if we look at the entire address as a single number.
// Our radix tree cursor utilities require a "path" object presenting
// an array-like interface. This is a very small wrapper class to provide
// just enough path functionality on top of a raw byte buffer containing
// a bit path for us to use our cursor utility routines.
template <std::size_t MAXDEPTH>
class WrapBytesAsPathRO
{
public:
  static constexpr std::size_t MaxDepth = MAXDEPTH;

  WrapBytesAsPathRO() = default;
  WrapBytesAsPathRO(const uint8_t* pb,std::size_t len) : pathBits_(pb), length_(len) {
    if (length_ > MaxDepth) {
      throw std::range_error("WrapBytesAsPathRO: length " + std::to_string(len) + " > MaxDepth" + std::to_string(MaxDepth));
    }
  }

  std::size_t size() const { return length_; }
  std::size_t at(std::size_t p) const {
    if (p >= length_) {
      throw std::range_error("WrapBytesAsPathRO::at(" + std::to_string(p) + ") out of bounds");
    }
    return Akamai::Mapper::RadixTree::BitPacking::atBit(pathBits_,p);
  }
  std::size_t operator[](std::size_t p) const { return at(p); }
  
private:
  const uint8_t* pathBits_{nullptr};
  std::size_t length_{0};
};

// This class implements a binary key/value map using a simple binary tree
// and basic cursor operations. Depth and tree implementation are templatized.
template <typename ValueT,std::size_t MD,template <typename,std::size_t> class TreeT>
class BinaryTreeLookup {
public:
  using Value = ValueT;
  static constexpr std::size_t MaxDepth = MD;

  void addValue(const uint8_t* addrBits,std::size_t prefixLength,const ValueT& v) {
    WrapBytesAsPathRO<MaxDepth> p(addrBits,prefixLength);
    cursorAddValueAt(tree_.lookupCursorWO(),p,v);
  }
  
  bool removeValue(const uint8_t* addrBits,std::size_t prefixLength) {
    WrapBytesAsPathRO<MaxDepth> p(addrBits,prefixLength);
    return cursorRemoveValueAt(tree_.cursor(),p);
  }

  ValueDepth<ValueT> lookupValueDepth(const uint8_t* addrBits,std::size_t prefixLength) const {
    WrapBytesAsPathRO<MaxDepth> p(addrBits,prefixLength);
    auto c = tree_.lookupCursorRO();
    cursorGoto(c,p);
    auto v = c.coveringNodeValueRO();
    std::size_t d = c.coveringNodeValueDepth();
    return ValueDepth<ValueT>(*(v.getPtrRO()),d);
  }

  // This is just like lookupValueDepth but ignores the depth - basically a convenience function
  // if you don't care about the depth.
  ValueT lookupValue(const uint8_t* addrBits,std::size_t prefixLength) const {
    auto v = cursorLookupCoveringValueRO(tree_.lookupCursorRO(),addrBits,prefixLength);
    return *(v.getPtrRO());
  }

private:
  TreeT<ValueT,MaxDepth> tree_{};
};

#endif