#ifndef AKAMAI_MAPPER_RADIX_TREE_BINARY_WORD_NODE_H_
#define AKAMAI_MAPPER_RADIX_TREE_BINARY_WORD_NODE_H_

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

#include <type_traits>
#include <limits>
#include <stdint.h>

#include "BinaryWordEdge.h"
#include "WordBlockAllocator.h"

namespace Akamai {
namespace Mapper {
namespace RadixTree {

/**
 * \brief Utility base class for multi-word binary tree nodes.
 * 
 * Implements the node interface abstraction except for the parts about
 * manipulating node values - those are implemented by subclasses.
 */
template <typename WordType,std::size_t WordCount,std::size_t ExtLead,std::size_t ExtTrail,template <typename,std::size_t> class WordAlloc>
class BinaryWordNodeBase
{
public:
  using AllocatorType = WordAlloc<WordType,WordCount>;
  static constexpr std::size_t InfoBitsReserved = (ExtLead + ExtTrail);
  using ExtTraits = WordEdgeBitTraits<WordType,InfoBitsReserved>;
  // Ref edge, but we steal some bits for metadata
  using Edge = BinaryWordEdgeRef<AllocatorType,WordType,ExtTraits::sizeBits,ExtTraits::pathBits,ExtLead,ExtTrail>;
  using NodeImplRefType = WordType;
  using NodeImplType = WordType*;
  // Order of binary tree is always 2
  static constexpr std::size_t Radix = 2;
  // we need to at least have metadata, left child, right child
  static constexpr std::size_t InfoWord = 0;
  static constexpr std::size_t LeftChildWord = 1;
  static constexpr std::size_t RightChildWord = 2;
  static constexpr NodeImplRefType nullRef = AllocatorType::nullRef;
  static constexpr std::size_t NoChild = std::numeric_limits<std::size_t>::max();

  static_assert(std::is_integral<WordType>::value && !std::numeric_limits<WordType>::is_signed,
                "node word base unit must be an unsigned integer");

  BinaryWordNodeBase() = default;
  virtual ~BinaryWordNodeBase() = default;
  explicit BinaryWordNodeBase(const AllocatorType* a,NodeImplRefType n) : alloc_(a), nodeRef_(n), ext_(a,n) {}
  BinaryWordNodeBase(const BinaryWordNodeBase& other) : alloc_(other.alloc_), nodeRef_(other.nodeRef_), ext_(other.alloc_,other.nodeRef_) {}
  BinaryWordNodeBase(BinaryWordNodeBase&& other)
    : alloc_(other.alloc_)
    , nodeRef_(other.nodeRef_)
    , ext_(std::move(other.ext_))
  {
    other.alloc_ = nullptr;
    other.nodeRef_ = nullRef;
    other.ext_ = Edge{};
  }

  BinaryWordNodeBase& operator=(const BinaryWordNodeBase& other) {
    if (this == &other) { return *this; }
    alloc_ = other.alloc_;
    nodeRef_ = other.nodeRef_;
    ext_ = Edge{other.alloc_,other.nodeRef_};
    return *this;
  }

  BinaryWordNodeBase& operator=(BinaryWordNodeBase&& other) {
    if (this == &other) { return *this; }
    alloc_ = other.alloc_;
    nodeRef_ = other.nodeRef_;
    ext_ = std::move(other.ext_);

    other.alloc_ = nullptr;
    other.nodeRef_ = nullRef;
    other.ext_ = Edge{};
    return *this;
  }
  
  bool exists() const { return (nodeRef_ != nullRef); }
  bool hasChild(unsigned c) const { return (getChild(c) != nullRef); }
  WordType getChild(unsigned c) const {
    if (c == 0) return *(chunk() + LeftChildWord);
    if (c == 1) return *(chunk() + RightChildWord);
    throw std::range_error("getChild() - child out of bounds");
    return nullRef;
  }
  WordType detachChild(unsigned c) { return setChild(c,nullRef); }
  WordType setChild(unsigned c,WordType childRef) {
    WordType prevChild = nullRef;
    if (c == 0) {
      prevChild = *(chunk() + LeftChildWord);
      *(chunk() + LeftChildWord) = childRef;
    } else if (c == 1) {
      prevChild = *(chunk() + RightChildWord);
      *(chunk() + RightChildWord) = childRef;
    } else {
      throw std::range_error("setChild() - child out of bounds");
    }
    return prevChild;
  }

  Edge& edge() { return ext_; }
  const Edge& edge() const { return ext_; }

  bool isLeaf() const { return !(hasChild(0) || hasChild(1)); }

protected:
  WordType* chunk() const { return alloc_->getPtr(nodeRef_); }

private:
  const AllocatorType* alloc_{nullptr};
  NodeImplRefType nodeRef_{nullRef};
  Edge ext_{};
};


/**
 * \brief Binary radix tree node/edge implemented on top of 4 integer words.
 *
 * Four word node has a layout like this:
 * \verbatim
 * Word 0: metadata
 *   bit 0 (MSB) - has value
 *   bits 1 - N: edge size/bits, 0 means no edge
 *
 * Word 1: left child ref
 * Word 2: right child ref
 * Word 3: value
 * \endverbatim
*/
template <typename WordType,template <typename,std::size_t> class WordAlloc>
class BinaryWordNode
  : public BinaryWordNodeBase<WordType,4,1,0,WordAlloc>
{
public:
  using Base = BinaryWordNodeBase<WordType,4,1,0,WordAlloc>;
  using AllocatorType = typename Base::AllocatorType;
  using NodeImplRefType = typename Base::NodeImplRefType;
  using ValueType = WordType;
  static constexpr bool ValueIsCopy = false;

  BinaryWordNode() = default;
  virtual ~BinaryWordNode() = default;
  explicit BinaryWordNode(const AllocatorType* a,NodeImplRefType n) : Base(a,n) {}
  BinaryWordNode(const BinaryWordNode& other) : Base(other) {}
  BinaryWordNode(BinaryWordNode&& other) : Base(std::move(other)) {}
  BinaryWordNode& operator=(const BinaryWordNode& other) {
    static_cast<Base&>(*this) = static_cast<const Base&>(other);
    return *this;
  }
  BinaryWordNode& operator=(BinaryWordNode&& other) {
    static_cast<Base&>(*this) = std::move(static_cast<const Base&>(other));
    return *this;
  }
  
  bool hasValue() const { return (this->exists() && ((this->chunk()[Base::InfoWord] & HasValueSet) != 0)); }
  void clearValue() { if (this->exists()) { this->chunk()[Base::InfoWord] &= ~HasValueSet; } }
  void setValue(WordType v) {
    this->chunk()[ValueWord] = v;
    this->chunk()[Base::InfoWord] |= HasValueSet;
  }

  const ValueType& value() const { return this->chunk()[ValueWord]; }
  ValueType& value() { return this->chunk()[ValueWord]; }

private:
  static constexpr std::size_t ValueWord = 3;
  static constexpr WordType HasValueSet = (static_cast<WordType>(0x1) << (8*sizeof(WordType) - 1));
};

/**
 * \brief Binary radix tree node/edge implemented on top of 3 + (data word count) integer words.
 *
 * Array word node has a layout like this:
 * \verbatim
 * Word 0: metadata
 *   bit 0 (MSB) - has value
 *   bits 1 - N: edge size/bits, 0 means no edge
 *
 * Word 1: left child ref
 * Word 2: right child ref
 * Words 3 - (3 + DataWordCount): value array
 * \endverbatim
*/
template <typename WordType,std::size_t DataWordCount,template <typename,std::size_t> class WordAlloc>
class BinaryWordArrayNode
  : public BinaryWordNodeBase<WordType,3 + DataWordCount,1,0,WordAlloc>
{
public:
  using Base = BinaryWordNodeBase<WordType,4,1,0,WordAlloc>;
  using AllocatorType = typename Base::AllocatorType;
  using NodeImplRefType = typename Base::NodeImplRefType;
  using ValueType = WordType*;
  static constexpr bool ValueIsCopy = false;

  BinaryWordArrayNode() = default;
  virtual ~BinaryWordArrayNode() = default;
  explicit BinaryWordArrayNode(const AllocatorType* a,NodeImplRefType n) : Base(a,n) {}
  BinaryWordArrayNode(const BinaryWordArrayNode& other) : Base(other) {}
  BinaryWordArrayNode(BinaryWordArrayNode&& other) : Base(std::move(other)) {}
  BinaryWordArrayNode& operator=(const BinaryWordArrayNode& other) {
    static_cast<Base&>(*this) = static_cast<const Base&>(other);
    return *this;
  }
  BinaryWordArrayNode& operator=(BinaryWordArrayNode&& other) {
    static_cast<Base&>(*this) = std::move(static_cast<const Base&>(other));
    return *this;
  }

  bool hasValue() const { return (this->exists() && ((this->chunk()[Base::InfoWord] & HasValueSet) != 0)); }
  void clearValue() { if (this->exists()) { this->chunk()[Base::InfoWord] &= ~HasValueSet; } }
  void setValue(const ValueType v) {
    for (std::size_t i = 0; i < DataWordCount; ++i) { this->chunk()[ValueWord + i] = *(v + i); }
    this->chunk()[Base::InfoWord] |= HasValueSet;
  }

  const ValueType value() const { return this->chunk() + ValueWord; }
  ValueType value() { return this->chunk() + ValueWord; }

private:
  static constexpr std::size_t ValueWord = 3;
  static constexpr WordType HasValueSet = (static_cast<WordType>(0x1) << (8*sizeof(WordType) - 1));
};


/**
 * \brief Compact binary word node - only uses 3 words per node instead of 4.
 *
 * \verbatim
 * Word 0: metadata and data
 *   bit 0 (MSB) - has value
 *   remaining bits are split between edge and value as desired
 * Word 1: left child ref
 * Word 2: right child ref
 * \endverbatim
 * 
 * This lightweight implementation of the node interface stores a copy of
 * the actual value since the internal bits of word 0 containing the value
 * aren't directly byte addressable. This copy doesn't inflate the entire
 * tree. It is used on-demand to provide a place to reference a node value
 * when we're at a specific node in the tree.
 */
template <typename DataWordType,typename WordType,template <typename,std::size_t> class WordAlloc>
class CompactBinaryWordNode
  : public BinaryWordNodeBase<WordType,3,0,8*sizeof(DataWordType) + 1,WordAlloc>
{
public:
  using Base = BinaryWordNodeBase<WordType,3,0,8*sizeof(DataWordType) + 1,WordAlloc>;
  using AllocatorType = typename Base::AllocatorType;
  using NodeImplRefType = typename Base::NodeImplRefType;
  using ValueType = DataWordType;
  static constexpr bool ValueIsCopy = true;

  static_assert(std::is_integral<DataWordType>::value,"data word must be an integer");
  static_assert(sizeof(DataWordType) <= (sizeof(WordType)/2),"data word size too large");

  CompactBinaryWordNode() = default;
  virtual ~CompactBinaryWordNode() = default;
  explicit CompactBinaryWordNode(const AllocatorType* a,NodeImplRefType n) : Base(a,n) {}
  CompactBinaryWordNode(const CompactBinaryWordNode& other) : Base(other), value_(other.value_) {}
  CompactBinaryWordNode(CompactBinaryWordNode&& other) : Base(std::move(other)), value_(std::move(other.value_)) {}
  CompactBinaryWordNode& operator=(const CompactBinaryWordNode& other) {
    if (this == &other) { return *this; }
    static_cast<Base&>(*this) = static_cast<const Base&>(other);
    value_ = other.value_;
    return *this;
  }
  CompactBinaryWordNode& operator=(CompactBinaryWordNode&& other) {
    if (this == &other) { return *this; }
    static_cast<Base&>(*this) = std::move(static_cast<const Base&>(other));
    value_ = std::move(other.value_);
    return *this;
  }

  bool hasValue() const { return ((this->chunk()[Base::InfoWord] & HasValueSet) != 0); }
  void clearValue() { this->chunk()[Base::InfoWord] &= ~HasValueSet; }
  DataWordType valueCopy() const { return static_cast<DataWordType>(this->chunk()[Base::InfoWord] & DataWordBitMask); }
  void setValue(DataWordType v) {
    WordType newiw = this->chunk()[Base::InfoWord];
    newiw |= HasValueSet;
    newiw |= ((newiw & ~DataWordBitMask) | static_cast<WordType>(v));
    this->chunk()[Base::InfoWord] = newiw;
    value_ = valueCopy();
  }
  const ValueType& value() const {
    value_ = valueCopy();
    return value_;
  }
  ValueType& value() {
    value_ = valueCopy();
    return value_;
  }

private:
  static constexpr std::size_t DataBits = 8*sizeof(DataWordType);
  static constexpr WordType HasValueSet = (static_cast<WordType>(0x1) << DataBits);
  static constexpr WordType DataWordBitMask = ((static_cast<WordType>(0x1) << DataBits) - 1);
  /**
   * \brief Since the value isn't directly addressable we store a copy here.
   * Needs to be mutable so we can update a const object with an RO pointer.
   */
  mutable ValueType value_{};
};

/**
 * \brief Special case for an boolean value - 1 bit integer is all we need.
 */
template <typename WordType,template <typename,std::size_t> class WordAlloc>
class CompactBinaryWordNode<bool,WordType,WordAlloc>
  : public BinaryWordNodeBase<WordType,3,0,2,WordAlloc>
{
public:
  using Base = BinaryWordNodeBase<WordType,3,0,2,WordAlloc>;
  using AllocatorType = typename Base::AllocatorType;
  using NodeImplRefType = typename Base::NodeImplRefType;
  using ValueType = bool;
  static constexpr bool ValueIsCopy = true;

  CompactBinaryWordNode() = default;
  virtual ~CompactBinaryWordNode() = default;
  explicit CompactBinaryWordNode(const AllocatorType* a,NodeImplRefType n) : Base(a,n) {}
  CompactBinaryWordNode(const CompactBinaryWordNode& other) : Base(other) {}
  CompactBinaryWordNode(CompactBinaryWordNode&& other) : Base(std::move(other)) {}
  CompactBinaryWordNode& operator=(const CompactBinaryWordNode& other) {
    if (this == &other) { return *this; }
    static_cast<Base&>(*this) = static_cast<const Base&>(other);
    value_ = other.value_;
    return *this;
  }
  CompactBinaryWordNode& operator=(CompactBinaryWordNode&& other) {
    if (this == &other) { return *this; }
    static_cast<Base&>(*this) = std::move(static_cast<const Base&>(other));
    value_ = std::move(other.value_);
    return *this;
  }

  bool hasValue() const { return (this->exists() && ((this->chunk()[Base::InfoWord] & HasValueSet) != 0)); }
  void clearValue() { if (this->exists()) { this->chunk()[Base::InfoWord] &= ~HasValueSet; } }
  bool valueCopy() const { return ((this->chunk()[Base::InfoWord] & DataWordBitMask) != 0); }
  void setValue(bool v) {
    WordType newiw = this->chunk()[Base::InfoWord];
    newiw |= HasValueSet;
    newiw |= ((newiw & ~DataWordBitMask) | (v ? DataWordBitMask : 0));
    this->chunk()[Base::InfoWord] = newiw;
    value_ = valueCopy();
  }

  const bool& value() const {
    value_ = valueCopy();
    return value_;
  }
  bool& value() {
    value_ = valueCopy();
    return value_;
  }

private:
  static constexpr std::size_t DataBits = 1;
  static constexpr WordType HasValueSet = (static_cast<WordType>(0x1) << DataBits);
  static constexpr WordType DataWordBitMask = ((static_cast<WordType>(0x1) << DataBits) - 1);
  // Need to have this mutable so we can update it if we get a read-only pointer
  mutable bool value_{false};
};

/**
 * \brief Special case for a nothing value - presence/absence of value is all we care about.
 */
template <typename WordType,template <typename,std::size_t> class WordAlloc>
class CompactBinaryWordNode<void,WordType,WordAlloc>
  : public BinaryWordNodeBase<WordType,3,0,1,WordAlloc>
{
public:
  using Base = BinaryWordNodeBase<WordType,3,0,1,WordAlloc>;
  using AllocatorType = typename Base::AllocatorType;
  using NodeImplRefType = typename Base::NodeImplRefType;
  using ValueType = bool;
  static constexpr bool ValueIsCopy = true;

  CompactBinaryWordNode() = default;
  virtual ~CompactBinaryWordNode() = default;
  explicit CompactBinaryWordNode(const AllocatorType* a,NodeImplRefType n) : Base(a,n) {}
  CompactBinaryWordNode(const CompactBinaryWordNode& other) : Base(other) {}
  CompactBinaryWordNode(CompactBinaryWordNode&& other) : Base(std::move(other)) {}
  CompactBinaryWordNode& operator=(const CompactBinaryWordNode& other) {
    if (this == &other) { return *this; }
    static_cast<Base&>(*this) = static_cast<const Base&>(other);
    value_ = other.value_;
    return *this;
  }
  CompactBinaryWordNode& operator=(CompactBinaryWordNode&& other) {
    if (this == &other) { return *this; }
    static_cast<Base&>(*this) = std::move(static_cast<const Base&>(other));
    value_ = std::move(other.value_);
    return *this;
  }

  bool hasValue() const { return (this->exists() && ((this->chunk()[Base::InfoWord] & HasValueSet) != 0)); }
  void clearValue() { if (this->exists()) { this->chunk()[Base::InfoWord] &= ~HasValueSet; } }
  bool valueCopy() const { return hasValue(); }
  void setValue(bool v) {
    if (this->exists()) {
      if (v) { this->chunk()[Base::InfoWord] |= HasValueSet; } 
      else { this->chunk()[Base::InfoWord] &= ~HasValueSet; }
    }
    value_ = hasValue();
  }
  const bool& value() const {
    value_ = hasValue();
    return value_;
  }
  bool& value() {
    value_ = hasValue();
    return value_;
  }

private:
  static constexpr WordType HasValueSet = static_cast<WordType>(0x1);
  // Need to have this mutable so we can update it if we get a read-only pointer
  mutable bool value_{false};
};

}
}
}

#endif
