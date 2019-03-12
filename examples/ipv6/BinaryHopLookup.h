#ifndef AKAMAI_MAPPER_RADIXTREE_BINARY_HOP_LOOKUP_H_
#define AKAMAI_MAPPER_RADIXTREE_BINARY_HOP_LOOKUP_H_

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

#include "BinaryTreeLookup.h"

// In this header file we implement a binary "hop tree". This is a hybrid radix tree
// that uses a top level multi-bit radix tree to "hop" as far as it can along the prefix
// in multi-bit increments, and then "steps" the rest of the way to the end of the prefix
// to set/get the applicable value. There are a number of examples of high-performance, 
// extremely memory-efficient hybrid tree implementations intended for use in fast IP routing.
// This is not one of those implementations, but it does help illustrate the flexibility of
// the Akamai radix tree library and is hopefully much easier to read.


// When taking the binary path provided and hoping down the top level tree we'll need
// to take a multi-bit view of the path. When stepping down the lower level tree we'll need
// to take the single-bit view of the path but ignore the bits we already hopped over.
// This class is a thin shim that provides just enough functionality to look like a path to
// our cursor navigation routines.
template <std::size_t BITS,std::size_t MAXDEPTH>
class BinaryPathWrapRO
{
public:
  static constexpr std::size_t Bits = BITS;
  static constexpr std::size_t MaxDepth = MAXDEPTH;

  BinaryPathWrapRO() = default;
  BinaryPathWrapRO(const uint8_t* pb,std::size_t len,std::size_t offset = 0) : pathBits_(pb), offset_(offset), length_(len) {
    if (offset > length_) { throw std::range_error("BinaryPathWrapRO: offset > length"); }
    if (length_ > MaxDepth) { throw std::range_error("BinaryPathWrapRO: length > MaxDepth"); }
  }

  std::size_t size() const { return length_ - offset_; }
  std::size_t at(std::size_t p) const {
    return static_cast<std::size_t>(Akamai::Mapper::RadixTree::BitPacking::atBits(Bits,pathBits_,p + offset_));
  }
  std::size_t operator[](std::size_t p) const { return at(p); }
  
private:
  const uint8_t* pathBits_{nullptr};
  std::size_t offset_{0};
  std::size_t length_{0};
};

// This class implements a binary key/value map using the hop tree scheme
// described above.
template <typename ValueT,std::size_t MD,std::size_t HopBits,
          template <std::size_t,typename,std::size_t> class HopTreeT,
          template <typename,std::size_t> class StepTreeT>
class BinaryHopLookup {
public:
  using Value = ValueT;
  static constexpr std::size_t MaxDepth = MD;

  void addValue(const uint8_t* addrBits,uint8_t prefixLength,const ValueT& v) {
    // Hop as far as we can - make a multibit path and walk down the toplevel tree.
    const std::size_t hopCount = prefixLength/HopBits;
    BinaryPathWrapRO<HopBits,HopDepth> hopPath(addrBits,hopCount,0);
    auto hopCursor = hopTree_.lookupCursorWO();
    Akamai::Mapper::RadixTree::cursorGoto(hopCursor,hopPath);
    // We need our lower-level tree to actually set a value.
    if (!hopCursor.atValue()) {
      hopCursor.addNode();
      hopCursor.nodeValue().set(StepTree{});
    }
  
    // Go as far as we need to in our lower-level tree and
    // set a value there.
    auto hopCursorValue = hopCursor.nodeValue();
    auto stepCursor = hopCursorValue.getPtrRW()->lookupCursorWO();
    const std::size_t bitsHopped = hopCount*HopBits;
    BinaryPathWrapRO<1,MaxDepth> stepPath(addrBits,prefixLength,bitsHopped);
    Akamai::Mapper::RadixTree::cursorGoto(stepCursor,stepPath);
    stepCursor.addNode();
    stepCursor.nodeValue().set(v);
  }
  
  bool removeValue(const uint8_t* addrBits,uint8_t prefixLength) {
    // Hop as far as we can - make a multibit path and walk down the toplevel tree.
    const std::size_t hopCount = prefixLength/HopBits;
    BinaryPathWrapRO<HopBits,HopDepth> hopPath(addrBits,hopCount,0);
    auto hopCursor = hopTree_.cursor();
    Akamai::Mapper::RadixTree::cursorGoto(hopCursor,hopPath);
    if (!hopCursor.atValue()) { return false; }
    
    // Now use our lower-level tree to do the actual removal (if applicable).
    auto hopCursorValue = hopCursor.nodeValue();
    auto stepCursor = hopCursorValue.getPtrRW()->cursor();
    const std::size_t bitsHopped = hopCount*HopBits;
    BinaryPathWrapRO<1,MaxDepth> stepPath(addrBits,prefixLength,bitsHopped);
    if (!Akamai::Mapper::RadixTree::cursorRemoveValueAt(stepCursor,stepPath)) { return false; }
    
    // If our step value deletion results in us being at the root of the step tree without
    // a value, then we need to remove our hop tree value and whatever parent nodes we can.
    // The cursorRemoveValueAt function above handles this cleanup in the step tree.
    if (!stepCursor.canGoParent() && !stepCursor.atValue()) {
      hopCursorValue.clear();
      while (!hopCursor.atNode() || hopCursor.removeNode()) { hopCursor.goParent(); }
    }
    return true;
  }

  ValueDepth<ValueT> lookupValueDepth(const uint8_t* addrBits,std::size_t prefixLength) const {
    // Hop down the top-level tree as long as there's a potential value we might find.
    BinaryPathWrapRO<HopBits,HopDepth> hopPath(addrBits,prefixLength/HopBits,0);
    auto hopCursor = hopTree_.lookupCursorRO();
    std::size_t hopCount = Akamai::Mapper::RadixTree::cursorGotoCovering(hopCursor,hopPath);
    // No covering value in the toplevel tree means we're done.
    if (!hopCursor.atValue()) { return ValueDepth<ValueT>(); }

    // Now we need to look for our covering value in the step tree.
    auto hopCursorValue = hopCursor.nodeValueRO();
    auto stepCursor = hopCursorValue.getPtrRO()->lookupCursorRO();
    const std::size_t bitsHopped = hopCount*HopBits;
    BinaryPathWrapRO<1,MaxDepth> stepPath(addrBits,prefixLength,bitsHopped);
    std::size_t stepCount = Akamai::Mapper::RadixTree::cursorGotoCovering(stepCursor,stepPath);
    if (stepCursor.atValue()) {
      return ValueDepth<ValueT>{*(stepCursor.nodeValueRO().getPtrRO()),bitsHopped+stepCount};
    }

    // No covering value in the step tree means that we need to walk back up the hop tree
    // until we find a step tree with an actual root value (if there is one).
    std::size_t atDepth = bitsHopped;
    while (hopCursor.goParent()) {
      atDepth -= HopBits;
      if (hopCursor.atValue()) {
        auto curHopCursorValue = hopCursor.nodeValueRO();
        auto stepRootCursor = curHopCursorValue.getPtrRO()->lookupCursorRO();
        if (stepRootCursor.atValue()) {
          return ValueDepth<ValueT>{*(stepRootCursor.nodeValueRO().getPtrRO()),atDepth};
        }
      }
    }
    return ValueDepth<ValueT>();
  }

  // This is just like lookupValueDepth but ignores the depth - basically a convenience function
  // if you don't care about the depth.
  ValueT lookupValue(const uint8_t* addrBits,std::size_t prefixLength) const {
    return lookupValueDepth(addrBits,prefixLength).value;
  }

  static_assert((HopBits > 1) && (HopBits <= 64),"HopBits must be > 1 and <= 64");
  static_assert((MaxDepth % HopBits) == 0,"HopBits must divide MaxDepth evenly");

  // More convenient to specify the number of bits we'd like to hop in each top level tree,
  // but we need the actual radix to build the tree. Use one of the provided metaprogramming
  // structs to compute this.
  static constexpr std::size_t HopRadix = Akamai::Mapper::RadixTree::Utils::BitsValueCount<HopBits>::value;

  static constexpr std::size_t HopDepth = (MaxDepth/HopBits);
  static constexpr std::size_t StepDepth = (HopBits - 1);

  using StepTree = StepTreeT<ValueT,StepDepth>;
  using HopTree = HopTreeT<HopRadix,StepTree,HopDepth>;

private:
  HopTree hopTree_{};
};

#endif