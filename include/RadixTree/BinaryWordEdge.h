#ifndef AKAMAI_MAPPER_RADIX_TREE_BINARY_WORD_EDGE_H_
#define AKAMAI_MAPPER_RADIX_TREE_BINARY_WORD_EDGE_H_

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

#include <cstddef>
#include <stdint.h>
#include <exception>
#include <stdexcept>

namespace Akamai {
namespace Mapper {
namespace RadixTree {

/**
 * \brief Base CRTP class with a bunch of static methods for manipulating a WordType as an edge.
 *
 * Putting this in a base class to facilitate using the
 * same code for different node allocation schemes. For example, our
 * raw mmap-compatible node implementations want to use this as a wrapper
 * on top of an external WordType, but we don't want to preclude building
 * a class that keeps a WordType internally.
 */
template <typename ImplClass,typename WordType,std::size_t SizeBits,std::size_t PathBits,std::size_t LeadingBits,std::size_t TrailingBits>
class BinaryWordEdgeBase
{
public:
  static_assert(sizeof(WordType)*8 == (LeadingBits + SizeBits + PathBits + TrailingBits),"Sum of bit allocations != to WordType total");
  static_assert(!std::is_signed<WordType>(),"WordType must be unsigned");
  
  static constexpr std::size_t Radix = 2;
  static constexpr std::size_t MaxDepth = PathBits;

  inline WordType size() const;
  inline bool full() const;
  inline bool empty() const;
  inline WordType capacity() const;
  inline void clear();
  inline void push_back(std::size_t c);
  inline void pop_back();
  inline std::size_t at(WordType p) const;
  inline std::size_t operator[](WordType p) const;
  inline bool operator==(const BinaryWordEdgeBase<ImplClass,WordType,SizeBits,PathBits,LeadingBits,TrailingBits>& other) const;
  inline bool operator!=(const BinaryWordEdgeBase<ImplClass,WordType,SizeBits,PathBits,LeadingBits,TrailingBits>& other) const;
  inline WordType matching(const BinaryWordEdgeBase<ImplClass,WordType,SizeBits,PathBits,LeadingBits,TrailingBits>& other) const;
  inline void trim_back(uint32_t n);
  inline void trim_front(uint32_t n);
  inline void printBits() const;
  inline bool coveredby(const BinaryWordEdgeBase<ImplClass,WordType,SizeBits,PathBits,LeadingBits,TrailingBits>& other) const;

  static constexpr WordType maskBits(uint32_t bitCount,uint32_t offset) { return (((static_cast<WordType>(0x1) << bitCount) - 1) << offset); }
  static constexpr WordType maskSize = maskBits(SizeBits,(TrailingBits + PathBits));
  static constexpr WordType maskSkip = ((LeadingBits == 0) ? 0 : maskBits(LeadingBits,(TrailingBits + PathBits + SizeBits)));
  static constexpr WordType maskTrailing = ((TrailingBits == 0) ? 0 : maskBits(TrailingBits,0));
  static constexpr WordType maskPathBits = maskBits(PathBits,TrailingBits);
  static constexpr WordType maskEdge = (maskSize | maskPathBits);

  inline void setSize(WordType s);
  inline static WordType pathBitOffset(uint32_t n);
  inline uint32_t pathBit(uint32_t n) const;
  inline WordType relevantBits() const;
  inline WordType allPathBits() const;
  inline WordType getPathBits(uint32_t bitCount) const;
};

/**
 * \brief Traits for path bit counts to use given a word type/reserved bits count.
 *
 * A bit on the brute force end of things, but straightforward, easy, and quick.
 */
template <typename WordType,std::size_t ReserveBits=0>
struct WordEdgeBitTraits {};

/**
 * \brief 8 bit integer, nothing reserved.
 */
template <>
struct WordEdgeBitTraits<uint8_t,0>
{
  static constexpr std::size_t sizeBits = 3;
  static constexpr std::size_t pathBits = 5;
};

/**
 * \brief 16 bit integers, nothing reserved.
 */
template <>
struct WordEdgeBitTraits<uint16_t,0>
{
  static constexpr std::size_t sizeBits = 4;
  static constexpr std::size_t pathBits = 12; 
};

/**
 * \brief 32 bit integers, nothing reserved.
 */
template <>
struct WordEdgeBitTraits<uint32_t,0>
{
  static constexpr std::size_t sizeBits = 5;
  static constexpr std::size_t pathBits = 27; 
};

/**
 * \brief 32 bit integers, 1 bit for presence of value.
 */
template <>
struct WordEdgeBitTraits<uint32_t,1>
{
  static constexpr std::size_t sizeBits = 5;
  static constexpr std::size_t pathBits = 26; 
};

/**
 * \brief 32 bit integers, 1 bit for presence of value and 1 bit for boolean.
 */
template <>
struct WordEdgeBitTraits<uint32_t,2>
{
  static constexpr std::size_t sizeBits = 5;
  static constexpr std::size_t pathBits = 25;
};

/**
 * \brief 32 bit integers, 1 bit for presence of value, 8 bit value.
 */
template <>
struct WordEdgeBitTraits<uint32_t,9>
{
  static constexpr std::size_t sizeBits = 5;
  static constexpr std::size_t pathBits = 18; 
};

/**
 * \brief 32 bit integers, 1 bit for presence of value, 16 bit value.
 */
template <>
struct WordEdgeBitTraits<uint32_t,17>
{
  static constexpr std::size_t sizeBits = 4;
  static constexpr std::size_t pathBits = 11; 
};

/**
 * \brief 64 bit integers, nothing reserved.
 */
template <>
struct WordEdgeBitTraits<uint64_t,0>
{
  static constexpr std::size_t sizeBits = 6;
  static constexpr std::size_t pathBits = 58; 
};

/**
 * \brief 64 bit integers, 1 bit for presence of value
 */
template <>
struct WordEdgeBitTraits<uint64_t,1>
{
  static constexpr std::size_t sizeBits = 6;
  static constexpr std::size_t pathBits = 57; 
};

/**
 * \brief 64 bit integers, 1 bit for presence of value, 1 bit boolean value.
 */
template <>
struct WordEdgeBitTraits<uint64_t,2>
{
  static constexpr std::size_t sizeBits = 6;
  static constexpr std::size_t pathBits = 56; 
};

/**
 * \brief 64 bit integers, 1 bit for presence of value, 8 bit value.
 */
template <>
struct WordEdgeBitTraits<uint64_t,9>
{
  static constexpr std::size_t sizeBits = 6;
  static constexpr std::size_t pathBits = 49; 
};

/**
 * \brief 64 bit integers, 1 bit for presence of value, 16 bit value.
 */
template <>
struct WordEdgeBitTraits<uint64_t,17>
{
  static constexpr std::size_t sizeBits = 6;
  static constexpr std::size_t pathBits = 41; 
};

/**
 * \brief 32 bit integers, 1 bit for presence of value, 32 bit value.
 */
template <>
struct WordEdgeBitTraits<uint64_t,33>
{
  static constexpr std::size_t sizeBits = 5;
  static constexpr std::size_t pathBits = 26; 
};



/**
 * \brief Simple in-place, word-sized edge.
 */
template <typename WordType,std::size_t SizeBits,std::size_t PathBits,std::size_t LeadingBits = 0,std::size_t TrailingBits = (8*sizeof(WordType) - (SizeBits + PathBits + LeadingBits))>
class BinaryWordEdge
  : public BinaryWordEdgeBase<BinaryWordEdge<WordType,SizeBits,PathBits,LeadingBits,TrailingBits>,WordType,SizeBits,PathBits,LeadingBits,TrailingBits>
{
public:
  BinaryWordEdge() = default;
  const WordType& extWord() const { return ext_; }
  WordType& extWord() { return ext_; }
private:
  WordType ext_{0};
};

template <typename WordType>
using SimpleBinaryWordEdge = BinaryWordEdge<WordType,WordEdgeBitTraits<WordType>::sizeBits,WordEdgeBitTraits<WordType>::pathBits>;


/**
 * \brief Indirect reference edge - needs to act like a reference and a separate object depending on context.
 *
 * When exposed from the node wrapper we want it to be a reference to the word inside the actual node
 * data, when created from the cursor we want it to be standalone.
 */
template <typename AllocatorType,typename WordType,std::size_t SizeBits,std::size_t PathBits,std::size_t LeadingBits = 0,std::size_t TrailingBits = (8*sizeof(WordType) - (SizeBits + PathBits + LeadingBits))>
class BinaryWordEdgeRef
  : public BinaryWordEdgeBase<BinaryWordEdgeRef<AllocatorType,WordType,SizeBits,PathBits,LeadingBits,TrailingBits>,WordType,SizeBits,PathBits,LeadingBits,TrailingBits>
{
public:
  typedef BinaryWordEdgeRef<AllocatorType,WordType,SizeBits,PathBits,LeadingBits,TrailingBits> MyType;
  typedef BinaryWordEdgeBase<MyType,WordType,SizeBits,PathBits,LeadingBits,TrailingBits> BaseType;
  typedef typename AllocatorType::RefType RefType;
  // Makes a simple empty edge
  BinaryWordEdgeRef() = default;
  // Makes a simple edge initialized with w
  BinaryWordEdgeRef(WordType w) : alloc_(nullptr), wordRef_(AllocatorType::nullRef), word_(w & this->maskEdge) {}

  // Makes a reference edge based on a and wr
  BinaryWordEdgeRef(const AllocatorType* a,RefType wr) : alloc_(a), wordRef_(wr), word_(0) {}

  /**
   * \brief Copy constructor for expected use within a Cursor class - note differences to copy assignment.
   * 
   * Copy construction takes a snapshot of the existing state and transforms
   * it into a standalone edge - that's how things will generally be
   * used inside of the cursor.
   */
  BinaryWordEdgeRef(const BinaryWordEdgeRef& other) : alloc_(nullptr), wordRef_(AllocatorType::nullRef), word_(other.extWord() & this->maskEdge) {}

  // Move construction just keeps things as they are.
  BinaryWordEdgeRef(BinaryWordEdgeRef&& other)
    : alloc_(other.alloc_)
    , wordRef_(other.wordRef_)
    , word_(other.word_ & this->maskEdge)
  {
    // Transform the source into a simple
    other.alloc_ = nullptr;
    other.wordRef_ = AllocatorType::nullRef;
  }

  /**
   * \brief Assignment operator - note differences with copy constructor.
   * 
   * Copying *into* an object keeps its existing ref/simple status, copies whatever
   * was pointed at by the other. Need to be careful to only copy edge bits so
   * we don't stomp on any metadata that might've already been there.
   */
  BinaryWordEdgeRef& operator=(const BinaryWordEdgeRef& other) {
    WordType myWord = extWord();
    extWord() = ((myWord & ~this->maskEdge) | (other.extWord() & this->maskEdge));
    return *this;
  }

  /**
   * \brief Similar to move construction but retains contents of non-edge bits.
   */
  BinaryWordEdgeRef& operator=(BinaryWordEdgeRef&& other) {
    alloc_ = other.alloc_;
    wordRef_ = other.wordRef_;
    word_ = ((word_ & ~this->maskEdge) | (other.word_ & this->maskEdge));

    other.alloc_ = nullptr;
    other.wordRef_ = AllocatorType::nullRef;

    return *this;
  }

  const WordType& extWord() const { return (isRef() ? *(alloc_->getPtr(wordRef_)) : word_); }
  WordType& extWord() { return (isRef() ? *(alloc_->getPtr(wordRef_)) : word_); }

  bool operator==(const MyType& other) const { return BaseType::operator==(other); }
  bool operator!=(const MyType& other) const { return !(*this == other); }

  bool isRef() const { return (wordRef_ != AllocatorType::nullRef); }

private:
  const AllocatorType* alloc_{nullptr};
  RefType wordRef_{AllocatorType::nullRef};
  WordType word_{0};
};

template <typename WordType,typename AllocatorType>
using SimpleBinaryWordEdgeRef = BinaryWordEdgeRef<AllocatorType,WordType,WordEdgeBitTraits<WordType>::sizeBits,WordEdgeBitTraits<WordType>::pathBits>;


/////////////////////
// IMPLEMENTATIONS //
/////////////////////

template <typename ImplClass,typename WordType,std::size_t SizeBits,std::size_t PathBits,std::size_t LeadingBits,std::size_t TrailingBits>
WordType BinaryWordEdgeBase<ImplClass,WordType,SizeBits,PathBits,LeadingBits,TrailingBits>::size() const {
  const WordType& ext = static_cast<const ImplClass*>(this)->extWord();
  return ((maskSize & ext) >> (PathBits + TrailingBits));
}

template <typename ImplClass,typename WordType,std::size_t SizeBits,std::size_t PathBits,std::size_t LeadingBits,std::size_t TrailingBits>
bool BinaryWordEdgeBase<ImplClass,WordType,SizeBits,PathBits,LeadingBits,TrailingBits>::full() const { return (size() == PathBits); }

template <typename ImplClass,typename WordType,std::size_t SizeBits,std::size_t PathBits,std::size_t LeadingBits,std::size_t TrailingBits>
bool BinaryWordEdgeBase<ImplClass,WordType,SizeBits,PathBits,LeadingBits,TrailingBits>::empty() const { return (size() == 0); }

template <typename ImplClass,typename WordType,std::size_t SizeBits,std::size_t PathBits,std::size_t LeadingBits,std::size_t TrailingBits>
WordType BinaryWordEdgeBase<ImplClass,WordType,SizeBits,PathBits,LeadingBits,TrailingBits>::capacity() const { return PathBits; }

template <typename ImplClass,typename WordType,std::size_t SizeBits,std::size_t PathBits,std::size_t LeadingBits,std::size_t TrailingBits>
void BinaryWordEdgeBase<ImplClass,WordType,SizeBits,PathBits,LeadingBits,TrailingBits>::clear() {
  WordType& ext = static_cast<ImplClass*>(this)->extWord();
  ext &= ~maskEdge;
}

template <typename ImplClass,typename WordType,std::size_t SizeBits,std::size_t PathBits,std::size_t LeadingBits,std::size_t TrailingBits>
void BinaryWordEdgeBase<ImplClass,WordType,SizeBits,PathBits,LeadingBits,TrailingBits>::push_back(std::size_t c) {
  if (full()) { throw std::length_error("[BinaryWordEdgeBase] push_back: edge full"); }
  WordType oldSize = size();
  setSize(oldSize + 1);
  WordType newPathBit = pathBitOffset(oldSize);
  WordType& ext = static_cast<ImplClass*>(this)->extWord();
  if (c > 0) {
    // Mask in newPathBit
    ext |= maskBits(1,newPathBit);
  } else {
    // Mask out newPathBit
    ext &= ~maskBits(1,newPathBit);
  }
}

template <typename ImplClass,typename WordType,std::size_t SizeBits,std::size_t PathBits,std::size_t LeadingBits,std::size_t TrailingBits>
void BinaryWordEdgeBase<ImplClass,WordType,SizeBits,PathBits,LeadingBits,TrailingBits>::pop_back() {
  if (empty()) { throw std::length_error("[BinaryWordEdgeBase] pop_back: edge empty"); }
  setSize(size() - 1);
}

template <typename ImplClass,typename WordType,std::size_t SizeBits,std::size_t PathBits,std::size_t LeadingBits,std::size_t TrailingBits>
std::size_t BinaryWordEdgeBase<ImplClass,WordType,SizeBits,PathBits,LeadingBits,TrailingBits>::at(WordType p) const { return pathBit(p); }

template <typename ImplClass,typename WordType,std::size_t SizeBits,std::size_t PathBits,std::size_t LeadingBits,std::size_t TrailingBits>
std::size_t BinaryWordEdgeBase<ImplClass,WordType,SizeBits,PathBits,LeadingBits,TrailingBits>::operator[](WordType p) const { return pathBit(p); }

template <typename ImplClass,typename WordType,std::size_t SizeBits,std::size_t PathBits,std::size_t LeadingBits,std::size_t TrailingBits>
bool
BinaryWordEdgeBase<ImplClass,WordType,SizeBits,PathBits,LeadingBits,TrailingBits>::operator==(const BinaryWordEdgeBase<ImplClass,WordType,SizeBits,PathBits,LeadingBits,TrailingBits>& other) const {
  return (relevantBits() == other.relevantBits());
}

template <typename ImplClass,typename WordType,std::size_t SizeBits,std::size_t PathBits,std::size_t LeadingBits,std::size_t TrailingBits>
bool
BinaryWordEdgeBase<ImplClass,WordType,SizeBits,PathBits,LeadingBits,TrailingBits>::operator!=(const BinaryWordEdgeBase<ImplClass,WordType,SizeBits,PathBits,LeadingBits,TrailingBits>& other) const {
  return !(*this == other);
}

template <typename ImplClass,typename WordType,std::size_t SizeBits,std::size_t PathBits,std::size_t LeadingBits,std::size_t TrailingBits>
WordType
BinaryWordEdgeBase<ImplClass,WordType,SizeBits,PathBits,LeadingBits,TrailingBits>::matching(const BinaryWordEdgeBase<ImplClass,WordType,SizeBits,PathBits,LeadingBits,TrailingBits>& other) const
{
  WordType longestMatch = std::min(size(),other.size());
  WordType cmp = (getPathBits(longestMatch) ^ other.getPathBits(longestMatch));
  // Find highest bit set in the xor, no bits set means everything matches
  if (cmp == 0) { return longestMatch; }
  WordType matchLen = PathBits;
  while (cmp > 0) {
    --matchLen;
    cmp >>= 1;
  }
  return matchLen;
}

template <typename ImplClass,typename WordType,std::size_t SizeBits,std::size_t PathBits,std::size_t LeadingBits,std::size_t TrailingBits>
void
BinaryWordEdgeBase<ImplClass,WordType,SizeBits,PathBits,LeadingBits,TrailingBits>::trim_back(uint32_t n) {
  WordType mySize = size();
  if (mySize < n) { throw std::length_error("[BinaryWordEdgeBase] trim_back: attempting to trim more bits than in edge"); }
  setSize(mySize - n);
}

template <typename ImplClass,typename WordType,std::size_t SizeBits,std::size_t PathBits,std::size_t LeadingBits,std::size_t TrailingBits>
void
BinaryWordEdgeBase<ImplClass,WordType,SizeBits,PathBits,LeadingBits,TrailingBits>::trim_front(uint32_t n)
{
  WordType mySize = size();
  if (mySize < n) { throw std::length_error("[BinaryWordEdgeBase] trim_front: attempting to trim more bits than in edge"); }
  WordType& ext = static_cast<ImplClass*>(this)->extWord();
  // get path bits, shift left, remask on path length
  WordType path = ((ext << n) & maskPathBits);
  // update path length
  setSize(mySize - n);
  // mask the new path bits in
  ext = ((ext & (maskSkip | maskSize | maskTrailing)) | path);
}


  
template <typename ImplClass,typename WordType,std::size_t SizeBits,std::size_t PathBits,std::size_t LeadingBits,std::size_t TrailingBits>
void
BinaryWordEdgeBase<ImplClass,WordType,SizeBits,PathBits,LeadingBits,TrailingBits>::printBits() const {
  WordType ext = static_cast<const ImplClass*>(this)->extWord();
  ext = (ext & maskPathBits);
}

template <typename ImplClass,typename WordType,std::size_t SizeBits,std::size_t PathBits,std::size_t LeadingBits,std::size_t TrailingBits>
bool
BinaryWordEdgeBase<ImplClass,WordType,SizeBits,PathBits,LeadingBits,TrailingBits>::coveredby(const BinaryWordEdgeBase<ImplClass,WordType,SizeBits,PathBits,LeadingBits,TrailingBits>& other) const
{
  WordType mySize = size();
  if (mySize > other.size()) { return false; }
  return (getPathBits(mySize) ^ other.getPathBits(mySize)) == 0;
}

template <typename ImplClass,typename WordType,std::size_t SizeBits,std::size_t PathBits,std::size_t LeadingBits,std::size_t TrailingBits>
void
BinaryWordEdgeBase<ImplClass,WordType,SizeBits,PathBits,LeadingBits,TrailingBits>::setSize(WordType s) {
  WordType& ext = static_cast<ImplClass*>(this)->extWord();
  ext = (((s & maskBits(SizeBits,0)) << (PathBits + TrailingBits)) | (ext & ~maskSize));
}

template <typename ImplClass,typename WordType,std::size_t SizeBits,std::size_t PathBits,std::size_t LeadingBits,std::size_t TrailingBits>
WordType
BinaryWordEdgeBase<ImplClass,WordType,SizeBits,PathBits,LeadingBits,TrailingBits>::pathBitOffset(uint32_t n) { return (TrailingBits + (PathBits - (n+1))); }

template <typename ImplClass,typename WordType,std::size_t SizeBits,std::size_t PathBits,std::size_t LeadingBits,std::size_t TrailingBits>
uint32_t
BinaryWordEdgeBase<ImplClass,WordType,SizeBits,PathBits,LeadingBits,TrailingBits>::pathBit(uint32_t n) const
{
  if (n >= size()) { throw std::length_error("[BinaryWordEdgeBase] path bit out of range"); }
  const WordType& ext = static_cast<const ImplClass*>(this)->extWord();
  WordType bitoff = pathBitOffset(n);
  return ((maskBits(1,bitoff) & ext) >> bitoff);
}

template <typename ImplClass,typename WordType,std::size_t SizeBits,std::size_t PathBits,std::size_t LeadingBits,std::size_t TrailingBits>
WordType
BinaryWordEdgeBase<ImplClass,WordType,SizeBits,PathBits,LeadingBits,TrailingBits>::relevantBits() const
{ 
  const WordType& ext = static_cast<const ImplClass*>(this)->extWord();
  // Mask in the size and bits specified in size
  WordType curSize = size();
  return ((ext & maskSize) | (ext & maskBits(curSize,TrailingBits + (PathBits - curSize))));
}

template <typename ImplClass,typename WordType,std::size_t SizeBits,std::size_t PathBits,std::size_t LeadingBits,std::size_t TrailingBits>
WordType
BinaryWordEdgeBase<ImplClass,WordType,SizeBits,PathBits,LeadingBits,TrailingBits>::allPathBits() const
{
  return getPathBits(size());
}

template <typename ImplClass,typename WordType,std::size_t SizeBits,std::size_t PathBits,std::size_t LeadingBits,std::size_t TrailingBits>
WordType
BinaryWordEdgeBase<ImplClass,WordType,SizeBits,PathBits,LeadingBits,TrailingBits>::getPathBits(uint32_t bitCount) const
{
  if (bitCount > size()) { throw std::length_error("[BinaryWordEdgeBase] getPathBits: bitCount > size()"); }
  const WordType& ext = static_cast<const ImplClass*>(this)->extWord();
  WordType pbits = (ext & maskPathBits);
  pbits >>= TrailingBits;
  pbits &= maskBits(bitCount,PathBits - bitCount);
  return pbits;
}


} // namespace RadixTree
} // namespace Mapper
} // namespace Akamai
#endif
