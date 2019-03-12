#ifndef AKAMAI_MAPPER_RADIXTREE_BINARY_LEAP_LOOKUP_H_
#define AKAMAI_MAPPER_RADIXTREE_BINARY_LEAP_LOOKUP_H_

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

// This class implements a binary tree lookup that starts with an unordered_map
// to "leap" down the path, then uses another lookup object for the rest.
// Alongside the unordered_map it also contains a lookup object that covers
// the space that the map leaps over. As with the simple BinaryTreeLookup class
// the depth, tree implementations, and stored value are all template parameters.

template <typename ValueT,std::size_t MaxDepth,std::size_t LeapBits,
          template <typename,std::size_t> class LookupInLeapT,
          template <typename,std::size_t> class LookupFromLeapT>
class BinaryLeapLookup {
public:
  // Placing some limits on our leap parameters, some of these limits are
  // required and others are somewhat arbitrary.
  static_assert(MaxDepth >= 16,"BinaryLeapLookup: MaxDepth must be >= 16");
  static_assert((LeapBits >= 8) && (LeapBits <= 64),"BinaryLeapLookup: LeapBits must be >= 8 and <= 64");
  static_assert((MaxDepth - 8) > LeapBits,"BinaryLeapLookup: LeapBits must be < (MaxDepth - 8)");
  static_assert((LeapBits % 8) == 0,"BinaryLeapLookup: LeapBits must be multiple of 8");

  static constexpr std::size_t MaxDepthInLeap = (LeapBits - 1);
  static constexpr std::size_t MaxDepthFromLeap = (MaxDepth - LeapBits);
  static constexpr std::size_t LeapBytes = (LeapBits/8);

  using Value = ValueT;
  using LookupInLeap = LookupInLeapT<ValueT,MaxDepthInLeap>;
  using LookupFromLeap = LookupFromLeapT<ValueT,MaxDepthFromLeap>;
  using LeapKey = typename Akamai::Mapper::RadixTree::Utils::UIntRequired<LeapBits>::type;
  using LeapMap = std::unordered_map<LeapKey,LookupFromLeap>;

  void addValue(const uint8_t* addrBits,uint8_t prefixLength,const ValueT& v) {
    // if our depth < our leap size then do the addition in the in-leap tree
    // if our depth >= our leap size then run the add on the from-leap lookup
    if (prefixLength <= MaxDepthInLeap) { return lookupInLeap_.addValue(addrBits,prefixLength,v); }
    return leapMap_[getLeapKey(addrBits)].addValue(addrBits + LeapBytes,prefixLength - LeapBits,v);
  }
  
  bool removeValue(const uint8_t* addrBits,uint8_t prefixLength) {
    // Find the correct lookup object to delegate to - either in or after the leap.
    if (prefixLength <= MaxDepthInLeap) { return lookupInLeap_.removeValue(addrBits,prefixLength); }

    auto leapMapIter = leapMap_.find(getLeapKey(addrBits));
    if (leapMapIter == leapMap_.end()) { return false; }
    return leapMapIter->second.removeValue(addrBits + LeapBytes,prefixLength - LeapBits);
  }

  ValueDepth<ValueT> lookupValueDepth(const uint8_t* addrBits,std::size_t prefixLength) const {
    // Lookups inside the leap distance are easy.
    if (prefixLength <= MaxDepthInLeap) { return lookupInLeap_.lookupValueDepth(addrBits,prefixLength); }
    
    // Outside the leap distance we have to see if we can actually leap there, otherwise
    // we need to check the in-leap lookup for a covering value.
    auto leapMapIter = leapMap_.find(getLeapKey(addrBits));
    // if we have a possible lookup in our leap tree then we try it first
    if (leapMapIter != leapMap_.end()) {
      ValueDepth<ValueT> leapResult = leapMapIter->second.lookupValueDepth(addrBits + LeapBytes,prefixLength - LeapBits);
      if (leapResult.foundValue) {
        leapResult.depth += LeapBits;
        return leapResult;
      }
    }
    // if we get here then we didn't find anything in our leap try to find a covering
    // value in the in-leap lookup
    return lookupInLeap_.lookupValueDepth(addrBits,MaxDepthInLeap);
  }

  // This is just like lookupValueDepth but ignores the depth - basically a convenience function
  // if you don't care about the depth.
  ValueT lookupValue(const uint8_t* addrBits,std::size_t prefixLength) const {
    return lookupValueDepth(addrBits,prefixLength).value;
  }
  
private:
  static LeapKey getLeapKey(const uint8_t* addrBits) { 
    const uint64_t leapKey64 = Akamai::Mapper::RadixTree::BitPacking::atBits(LeapBits,addrBits,0);
    return static_cast<LeapKey>(leapKey64);
  }

  LeapMap leapMap_{};
  LookupInLeap lookupInLeap_{};
};

#endif