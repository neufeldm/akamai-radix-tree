#ifndef AKAMAI_MAPPER_RADIXTREE_BIT_PACKING_H_
#define AKAMAI_MAPPER_RADIXTREE_BIT_PACKING_H_

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

#include <algorithm>
#include <stdexcept>
#include <stdint.h>
#include <cstddef>

namespace Akamai {
namespace Mapper {
namespace RadixTree {

namespace BitPacking {

/**
 * \brief Returns value of bit at index bit offset, no bounds checking performed.
 */
inline std::size_t atBit(const uint8_t* bits,std::size_t index);

/**
 * \brief Extracts bitsPerNumber at bit offset (index*bitsPerNumber) as big-endian integer.
 */
inline uint64_t atBits(std::size_t bitsPerNumber,const uint8_t* bits,std::size_t index);

/////////////////////
// IMPLEMENTATIONS //
/////////////////////

std::size_t atBit(const uint8_t* bits,std::size_t index) {
  return ((bits[index/8] & (static_cast<uint8_t>(0x1) << (7 - (index % 8)))) != 0) ? 1 : 0;
}

uint64_t atBits(std::size_t bitsPerNumber,const uint8_t* bits,std::size_t index) {
  if (bitsPerNumber > 64) { throw std::range_error("bitsPerNumber too large (max. 64)"); }
  if (bitsPerNumber == 1) { return static_cast<uint64_t>(atBit(bits,index)); }
  if (bitsPerNumber == 8) { return bits[index]; }

  const std::size_t startBit = bitsPerNumber*index;
  const std::size_t finishBit = startBit + bitsPerNumber - 1;
  uint64_t result = 0;
  std::size_t atBit = startBit;
  do {
    const std::size_t atByte = atBit/8;
    const std::size_t atBitInByte = (atBit % 8);
    const std::size_t bitsAtTopToSkip = atBitInByte;
    const std::size_t bitsToGet = std::min(bitsPerNumber,8 - atBitInByte);
    const std::size_t bitsAtBottomToSkip = (8 - (atBitInByte + bitsToGet));
    const uint64_t newBits = static_cast<uint64_t>((bits[atByte] & (0xFF >> bitsAtTopToSkip)) >> bitsAtBottomToSkip);
    result = ((result << bitsToGet) | newBits);
    atBit += bitsToGet;
  } while (atBit <= finishBit);

  return result;
}

} // namespace BitPacking

} // namespace RadixTree
} // namespace Mapper
} // namespace Akamai

#endif