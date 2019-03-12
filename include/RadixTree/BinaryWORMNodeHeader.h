#ifndef AKAMAI_MAPPER_RADIX_TREE_BINARY_WORM_NODE_HEADER_H_
#define AKAMAI_MAPPER_RADIX_TREE_BINARY_WORM_NODE_HEADER_H_

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
#include <array>
#include <stdexcept>
#include <limits>

#include "RadixTreeUtils.h"
#include "SimpleEdge.h"

#include "BinaryWORMNodeHeaderBytes.h"

namespace Akamai {
namespace Mapper {
namespace RadixTree {

/**
 * \brief Wrapper class for the read-only version of our binary WORM node.
 */
template <std::size_t OFFSETSIZE,bool LITTLEENDIAN>
class BinaryWORMNodeHeaderRO {
public:
  using HeaderBytes = BinaryWORMNodeHeaderBytes<OFFSETSIZE,LITTLEENDIAN>;
  using OffsetType = typename HeaderBytes::OffsetType;
  static constexpr std::size_t Radix = 2;
  static constexpr std::size_t OffsetSize = OFFSETSIZE;
  static constexpr bool LittleEndian = LITTLEENDIAN;
  static constexpr bool BigEndian = !LITTLEENDIAN;
  static constexpr std::size_t MaxEdgeSteps = HeaderBytes::MaxEdgeSteps;
  using EdgeWordType = typename HeaderBytes::EdgeWordType;
  static constexpr std::size_t NoChild = std::numeric_limits<std::size_t>::max();

  BinaryWORMNodeHeaderRO() = default;
  BinaryWORMNodeHeaderRO(const uint8_t* ptr) : ptr_(ptr) {}

  static std::string headerTypeID() { return HeaderBytes::headerTypeID(); }

  bool hasChild(std::size_t c) const {
    if (c >= Radix) { throw std::runtime_error("BinaryWORMNodeHeaderRO: child out of range"); }
    return HeaderBytes::hasChild(checkptr(),c);
  }
  bool hasValue() const { return HeaderBytes::hasValue(checkptr()); }

  std::size_t edgeStepCount() const { return HeaderBytes::edgeStepCount(checkptr()); }
  EdgeWordType edgeBitsAsWord() const { return HeaderBytes::getEdgeBitsAsWord(checkptr()); }
  std::size_t edgeStepAt(std::size_t es) const {
    if (es >= HeaderBytes::edgeStepCount(checkptr())) { throw std::runtime_error("BinaryWORMNodeHeaderRO: edge step out of range"); }
    return HeaderBytes::edgeStepAt(ptr(),es);
  }
  std::size_t headerSize() const { return HeaderBytes::headerSize(checkptr()); }
  const uint8_t* valuePtr() const { return (ptr() + headerSize()); }
  // Without knowing the size of the value we can't
  // actually compute the left node offset since it
  // directly follows our node header + value.
  OffsetType rightChildOffset() const {
    if (!(hasChild(0) && hasChild(1))) { throw std::runtime_error("BinaryWORMNodeHeaderRO: right child offset doesn't exist"); }
    return HeaderBytes::getRightChildOffset(ptr_);
  }
  const uint8_t* ptr() const { return ptr_; }
  void setPtr(const uint8_t* p) { ptr_ = p; }

private:
  const uint8_t* ptr_{nullptr};
  const uint8_t* checkptr() const {
    if (ptr_ == nullptr) { throw std::runtime_error("BinaryWORMNodeHeaderRO: null data pointer"); }
    return ptr_;
  }
};

/**
 * \brief Wrapper class for the read/write (mostly write) binary WORM node header.
 */
template <std::size_t OFFSETSIZE,bool LITTLEENDIAN>
class BinaryWORMNodeHeaderRW
{
public:
  using MyType = BinaryWORMNodeHeaderRW<OFFSETSIZE,LITTLEENDIAN>;
  using HeaderRO = BinaryWORMNodeHeaderRO<OFFSETSIZE,LITTLEENDIAN>;
  using HeaderBytes = BinaryWORMNodeHeaderBytes<OFFSETSIZE,LITTLEENDIAN>;
  using OffsetType = typename HeaderBytes::OffsetType;
  using EdgeType = SimpleEdge<2,HeaderBytes::MaxEdgeSteps>;
  static constexpr std::size_t Radix = 2;
  static constexpr std::size_t OffsetSize = OFFSETSIZE;

  BinaryWORMNodeHeaderRW() = default;

  static std::string headerTypeID() { return HeaderBytes::headerTypeID(); }
  
  bool hasValue() const { return hasValue_; }
  void setHasValue(bool hv) { hasValue_ = hv; }

  bool hasChild(std::size_t c) const { return hasChild_.at(c); }
  void setHasChild(std::size_t c,bool hc) { hasChild_.at(c) = hc; }
  void setHasChild(const std::array<bool,2>& hc) { hasChild_ = hc; }
  void setRightChildOffset(OffsetType rco) {
    if (!(hasChild(0) && hasChild(1))) { throw std::runtime_error("BinaryWORMNodeHeaderRW: cannot set right child offset without right child present"); }
    rightChildOffset_ = rco;
  }

  EdgeType& edge() { return edge_; }
  const EdgeType& edge() const { return edge_; }

  std::size_t headerSize() const { return HeaderBytes::headerSize(hasChild(0) && hasChild(1)); }
  std::size_t writeHeader(uint8_t* b) const {
    if (b == nullptr) { throw std::runtime_error("BinaryWORMNodeHeaderRW: attempt to write to nullptr"); }
    HeaderBytes::clear(b);
    HeaderBytes::setHasValue(b,hasValue());
    HeaderBytes::setHasChild(b,0,hasChild(0));
    HeaderBytes::setHasChild(b,1,hasChild(1));
    std::size_t edgeStepCount = edge_.size();
    HeaderBytes::setEdgeStepCount(b,edgeStepCount);
    for (std::size_t s = 0; s < edgeStepCount; ++s) { HeaderBytes::setEdgeStepAt(b,s,edge_.at(s)); }
    if (hasChild(0) && hasChild(1)) { HeaderBytes::setRightChildOffset(b,rightChildOffset_); }
    return HeaderBytes::headerSize(b);
  }
  std::size_t readHeader(const uint8_t* b) {
    if (b == nullptr) { throw std::runtime_error("BinaryWORMNodeHeaderRW: attempt to read from nullptr"); }
    *this = MyType{};
    HeaderRO h(b);
    setHasValue(h.hasValue());
    setHasChild(0,h.hasChild(0));
    setHasChild(1,h.hasChild(1));
    if (hasChild(0) && hasChild(1)) { setRightChildOffset(h.rightChildOffset()); }
    for (std::size_t es = 0; es < h.edgeStepCount(); ++es) { edge().push_back(h.edgeStepAt(es)); }
    return h.headerSize();
  }

private:
  bool hasValue_{false};
  std::array<bool,2> hasChild_{{false,false}};
  EdgeType edge_{};
  OffsetType rightChildOffset_{0};
};

/////////////////////
// IMPLEMENTATIONS //
/////////////////////

} // namespace RadixTree
} // namespace Mapper
} // namespace Akamai

#endif