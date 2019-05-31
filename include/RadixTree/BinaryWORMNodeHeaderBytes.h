#ifndef AKAMAI_MAPPER_RADIX_TREE_BINARY_WORM_NODE_HEADER_BYTES_H_
#define AKAMAI_MAPPER_RADIX_TREE_BINARY_WORM_NODE_HEADER_BYTES_H_

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

#include "RadixTreeUtils.h"

/**
 * \file BinaryWORMNodeHeaderBytes.h
 * 
 * This file contains low-level byte manipulation routines for our
 * binary WORM node header. These serve to define the underlying byte
 * format, the intention is that higher-level wrapper classes be
 * built on top of these for general use.
 */

namespace Akamai {
namespace Mapper {
namespace RadixTree {

/**
 * \brief Utilities for variable-byte little/big endian integer reading/writing.
 */
template <std::size_t UINTSIZEOF,bool LITTLEENDIAN>
struct BinaryWORMNodeUIntOps {
  static constexpr bool LittleEndian = LITTLEENDIAN;
  static constexpr bool BigEndian = !LITTLEENDIAN;
  static constexpr std::size_t UIntSize = UINTSIZEOF;
  static constexpr std::size_t UIntSizeBits = UINTSIZEOF*8;
  static_assert((UIntSize <= 8) && (UIntSize > 0),"UInt size must be > 0 and <= 8");
  using UIntType = typename Utils::UIntRequired<UIntSizeBits>::type;
  static constexpr UIntType UINT_MASK = ((static_cast<UIntType>(0) - 1) >> (8*sizeof(UIntType) - UIntSizeBits));
  static void writeUInt(uint8_t* uintPtr,UIntType ui) {
    for (std::size_t b = 0;b < UIntSize;++b) {
      uintPtr[LittleEndian ? b : (UIntSize - b - 1)] = static_cast<uint8_t>(ui & 0xFF);
      ui = (ui >> 8);
    }
  }
  static UIntType readUInt(const uint8_t* uintPtr) {
    UIntType curUInt{0};
    for (std::size_t i=0;i<UIntSize;++i) {
      std::size_t shiftLeft = 8*(LittleEndian ? i : (UIntSize - i - 1));
      curUInt |= (static_cast<UIntType>(uintPtr[i]) << shiftLeft);
    }
    return curUInt;
  }
};

/**
 * \brief Byte manipulation for our basic WORM node header.
 * 
 * The basic WORM node has an initial header that fits into
 *  a single byte. Here's what it contains:
 * \verabtim
   Bit 7: 1/0 depending if node has/doesn't have a value
   Bit 6: 1/0 depending if node has/doesn't have a "left" child (child 0)
   Bit 5: 1/0 depending if node has/doesn't have a "right" child (child 1)
   Bits 4,3: 2 bit integer, 0 - 3 as length of node edge
   Bit 2: edge step 0
   Bit 1: edge step 1
   Bit 0: edge step 2
 * \endverbatim
 * Immediately following the metadata byte:
 *  -# If node has both children then OFFSETSIZE bytes representing the
 *     offset of the right child (child 1). This offset is relative to the start of the node.
 *  -# If node has a value then the bytes representing the value
 *
 * Immediately after that:
 * - If the node has no children, then the next node in tree (most likely a right (second) child of a prior node)
 * - If the node has one child, then the header/value for that child node
 * - If the node has two children, then the header/value for the left (first) child
 */
template <std::size_t OFFSETSIZE,bool LITTLEENDIAN>
struct BinaryWORMNodeHeaderBytes {
  static constexpr std::size_t OffsetSize = OFFSETSIZE;
  static constexpr std::size_t MaxHeaderSize = (1 + OffsetSize);
  static_assert((OffsetSize <= 8) && (OffsetSize > 0),"Offset size must be > 0 bytes and <= 8");
  static constexpr bool LittleEndian = LITTLEENDIAN;
  static constexpr bool BigEndian = !LITTLEENDIAN;
  using OffsetType = typename Utils::UIntRequired<8*OffsetSize>::type;
  static constexpr std::size_t MaxEdgeSteps = 3;
  using EdgeWordType = uint8_t;
  static_assert(8*sizeof(EdgeWordType) >= MaxEdgeSteps,"EdgeWordType too small for MaxEdgeSteps");
  using UIntOps = BinaryWORMNodeUIntOps<OffsetSize,LittleEndian>;
  static constexpr std::size_t Radix = 2;
  static constexpr char HeaderTypeID[] = "AKAMAI-WORM-SINGLEBYTE";

  static std::string headerTypeID() { static std::string hid = HeaderTypeID; return hid; }

  static void clear(uint8_t* b) { *b = 0x0; }

  // Static header accessors don't do bounds/validity checking
  static constexpr uint8_t MASK_HAS_CHILD(std::size_t c) { return ((c == 0) ? 0x40 : 0x20); };
  static bool hasChild(const uint8_t* b,std::size_t c) { return ((MASK_HAS_CHILD(c) & *b) != 0); }
  static void setHasChild(uint8_t* b,std::size_t c,bool hc) { *b = (hc ? (MASK_HAS_CHILD(c) | *b) : (~MASK_HAS_CHILD(c) & *b)); }
  static OffsetType getRightChildOffset(const uint8_t* b) { return UIntOps::readUInt(b+1); }
  static void setRightChildOffset(uint8_t* b,OffsetType o) { UIntOps::writeUInt(b+1,o); }

  static constexpr uint8_t MASK_HAS_VALUE = 0x80;
  static bool hasValue(const uint8_t* b) { return ((*b & MASK_HAS_VALUE) != 0); }
  static void setHasValue(uint8_t* b, bool hv) { *b = (hv ? (*b | MASK_HAS_VALUE) : (*b & ~MASK_HAS_VALUE)); }

  static constexpr uint8_t MASK_STEPCOUNT = 0x18;
  static constexpr std::size_t SHIFT_STEPCOUNT = 3;
  static std::size_t edgeStepCount(const uint8_t* b) { return ((*b & MASK_STEPCOUNT) >> SHIFT_STEPCOUNT); }
  static void setEdgeStepCount(uint8_t* b,std::size_t sc) {
    *b = ((*b & ~MASK_STEPCOUNT) | ((static_cast<uint8_t>(sc) << SHIFT_STEPCOUNT) & MASK_STEPCOUNT));
  }
  static constexpr uint8_t ZERO = 0x0;
  static constexpr uint8_t ONE = 0x1;
  static constexpr std::size_t EDGE_STEPCOUNT = 3;
  static constexpr uint8_t EDGE_MASK = ((ONE << EDGE_STEPCOUNT) - 1);
  static constexpr uint8_t MASK_ALL_EDGE_IN = (MASK_STEPCOUNT | EDGE_MASK);
  static constexpr uint8_t MASK_ALL_EDGE_OUT = ~MASK_ALL_EDGE_IN;
  static std::size_t EDGE_SHIFT(std::size_t es) { return (EDGE_STEPCOUNT - es - 1); }
  static std::size_t edgeStepAt(const uint8_t* b,std::size_t es) { return ((*b >> EDGE_SHIFT(es)) & 0x1); }
  static void setEdgeStepAt(uint8_t* b,std::size_t es,std::size_t sv) {
    *b = ((~(ONE << EDGE_SHIFT(es)) & *b) | (((sv == 0) ? ZERO : ONE) << EDGE_SHIFT(es)));
  }
  // return the edge bits (if any) in the top N bits of a uint8_t
  static uint8_t getEdgeBitsAsWord(const uint8_t* b) {
    return ((*b & EDGE_MASK) << (8 - EDGE_STEPCOUNT));
  }
  static std::size_t headerSize(const uint8_t* b) { return headerSize(hasChild(b,1) && hasChild(b,0)); }
  static std::size_t headerSize(bool hasBothChildren) { return (1 + (hasBothChildren ? OffsetSize : 0)); }
};

} // namespace RadixTree
} // namespace Mapper
} // namespace Akamai

#endif