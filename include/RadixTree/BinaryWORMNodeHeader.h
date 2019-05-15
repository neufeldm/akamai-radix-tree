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
 * \brief Wrapper class for the read/write binary WORM node header.
 * 
 * Keeps writing state in an internal buffer.
 */
template <std::size_t OFFSETSIZE,bool LITTLEENDIAN>
class BinaryWORMNodeHeaderRW
{
public:
  using HeaderBytes = BinaryWORMNodeHeaderBytes<OFFSETSIZE,LITTLEENDIAN>;
  using MyType = BinaryWORMNodeHeaderRW<OFFSETSIZE,LITTLEENDIAN>;
  using OffsetType = typename HeaderBytes::OffsetType;
  static constexpr std::size_t Radix = 2;
  static constexpr std::size_t OffsetSize = OFFSETSIZE;
  static constexpr bool LittleEndian = LITTLEENDIAN;
  static constexpr bool BigEndian = !LITTLEENDIAN;
  static constexpr std::size_t MaxEdgeSteps = HeaderBytes::MaxEdgeSteps;
  using EdgeWordType = typename HeaderBytes::EdgeWordType;
  static constexpr std::size_t NoChild = std::numeric_limits<std::size_t>::max();

  static std::string headerTypeID() { return HeaderBytes::headerTypeID(); }

  bool hasChild(std::size_t c) const {
    if (c >= Radix) { throw std::runtime_error("BinaryWORMNodeHeaderRW: child out of range"); }
    return HeaderBytes::hasChild(headerBytes_.data(),c);
  }
  bool hasValue() const { return HeaderBytes::hasValue(headerBytes_.data()); }

  std::size_t edgeStepCount() const { return HeaderBytes::edgeStepCount(headerBytes_.data()); }
  EdgeWordType edgeBitsAsWord() const { return HeaderBytes::getEdgeBitsAsWord(headerBytes_.data()); }
  std::size_t edgeStepAt(std::size_t es) const {
    if (es >= HeaderBytes::edgeStepCount(headerBytes_.data())) { throw std::runtime_error("BinaryWORMNodeHeaderRW: edge step out of range"); }
    return HeaderBytes::edgeStepAt(headerBytes_.data(),es);
  }
  std::size_t headerSize() const { return HeaderBytes::headerSize(headerBytes_.data()); }
  // Without knowing the size of the value we can't
  // actually compute the left node offset since it
  // directly follows our node header + value.
  OffsetType rightChildOffset() const {
    if (!(hasChild(0) && hasChild(1))) { throw std::runtime_error("BinaryWORMNodeHeaderRW: right child offset doesn't exist"); }
    return HeaderBytes::getRightChildOffset(headerBytes_.data());
  }

  void setHasValue(bool hv) { HeaderBytes::setHasValue(headerBytes_.data(),hv); }

  void setHasChild(std::size_t c,bool hc) {
    if (c > 1) { throw std::runtime_error("BinaryWORMNodeHeaderRW::setHasChild: child out of range"); }
    HeaderBytes::setHasChild(headerBytes_.data(),c,hc);
  }
  void setHasChild(const std::array<bool,2>& hc) { 
    HeaderBytes::setHasChild(headerBytes_.data(),0,hc[0]);
    HeaderBytes::setHasChild(headerBytes_.data(),1,hc[1]);
  }
  void setRightChildOffset(OffsetType rco) {
    if (!(this->hasChild(0) && this->hasChild(1))) { throw std::runtime_error("BinaryWORMNodeHeaderRW: cannot set right child offset without right child present"); }
    HeaderBytes::setRightChildOffset(headerBytes_.data(),rco);
  }

  bool edgeFull() const { return HeaderBytes::edgeStepCount(headerBytes_.data()) == HeaderBytes::MaxEdgeSteps; }
  void edgePushBack(std::size_t step) {
    if (edgeFull()) {
      throw std::runtime_error("BinaryWORMNodeHeaderRW::edgePushBack: edge full");
    }
    std::size_t curStepCount = HeaderBytes::edgeStepCount(headerBytes_.data());
    HeaderBytes::setEdgeStepAt(headerBytes_.data(),curStepCount,step);
    HeaderBytes::setEdgeStepCount(headerBytes_.data(),++curStepCount);
  }
  void copyEdgeFrom(const MyType& o) {
    headerBytes_[0] = (headerBytes_[0] & HeaderBytes::MASK_ALL_EDGE_OUT) | (o.headerBytes_[0] & HeaderBytes::MASK_ALL_EDGE_IN);
  }
  std::size_t writeHeader(uint8_t* b) const {
    if (b == nullptr) { throw std::runtime_error("BinaryWORMNodeHeaderRW: attempt to write to nullptr"); }
    std::memcpy(b,headerBytes_.data(),HeaderBytes::headerSize(headerBytes_.data()));
    return HeaderBytes::headerSize(b);
  }
  std::size_t readHeader(const uint8_t* b) {
    if (b == nullptr) { throw std::runtime_error("BinaryWORMNodeHeaderRW: attempt to read from nullptr"); }
    std::memcpy(headerBytes_.data(),b,HeaderBytes::headerSize(b));
    return HeaderBytes::headerSize(b);
  }

private:
  std::array<uint8_t,HeaderBytes::MaxHeaderSize> headerBytes_{};
};

/////////////////////
// IMPLEMENTATIONS //
/////////////////////

} // namespace RadixTree
} // namespace Mapper
} // namespace Akamai

#endif