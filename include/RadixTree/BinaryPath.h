#ifndef AKAMAI_MAPPER_RADIX_TREE_BINARY_PATH_H_
#define AKAMAI_MAPPER_RADIX_TREE_BINARY_PATH_H_

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

#include <exception>
#include <array>
#include <string>
#include <cstring>
#include <stdint.h>

namespace Akamai {
namespace Mapper {
namespace RadixTree {

/**
 * \brief Represents a compact bit-packed path in a binary tree.
 * 
 * Conveniently arranged so that a path of 128 bits is binary compatible with the representation
 * of an IPv6 address used in unix/posix machine word order.
 * 
 * @see SimplePath for documentation of methods.
 */
template <std::size_t MAXDEPTH>
class BinaryPath
{
public:
  static constexpr std::size_t Radix = 2;
  static constexpr std::size_t MaxDepth = MAXDEPTH;
  static constexpr std::size_t BytesRequired = ((((MaxDepth+7)/8) == 0) ? 1 : ((MaxDepth+7)/8));
  typedef std::array<uint8_t,BytesRequired> BitArrayType;

  BinaryPath() = default;
  inline BinaryPath(std::initializer_list<std::size_t> p);
  inline BinaryPath(const uint8_t* pathBytes,std::size_t pathLen);

  std::size_t size() const { return length_; }
  std::size_t suffixLength() const { return MaxDepth - length_; }
  bool full() const { return (length_ == MaxDepth); }
  bool empty() const { return (length_ == 0); }
  static constexpr std::size_t capacity() { return MaxDepth; }
  inline void clear();
  inline void resize(std::size_t newSize);
  const uint8_t* rawBytes() const { return bits_.data(); }
  uint8_t* rawBytes() { return bits_.data(); }
  static constexpr size_t byteCapacity() { return sizeof(bits_); }
  const BitArrayType& bytes() { return bits_; }
  inline void push_back(std::size_t c);
  inline void pop_back();
  inline std::size_t at(std::size_t p) const;
  std::size_t operator[](std::size_t p) const { return at(p); }
  inline bool operator==(const BinaryPath<MaxDepth>& other) const;
  bool operator!=(const BinaryPath<MaxDepth>& other) const { return !(*this == other); }
  inline std::size_t matching(const BinaryPath<MaxDepth>& other) const;
  inline void trim_back(std::size_t n);
  inline void trim_front(std::size_t n);
  inline bool coveredby(const BinaryPath<MaxDepth>& other) const;

  inline std::string toBinaryString() const;
  inline bool fromBinaryString(const std::string& s);
  inline std::string toHexString() const;
  inline bool fromHexString(const std::string& s);
  
  template <typename BitsType>
  inline static void shiftLeftBits(BitsType&& bits,std::size_t bitCount);

private:
  BitArrayType bits_{};
  std::size_t length_{0};
};

namespace BinaryPathUtils {
/**
 * \brief Return bits in Binary Path in binary string format
 * \param p an instance of BinaryPath
 * \param sep character separator
 * \param sepDigits specifies how many digits are placed between separator
 */  
template <typename PathType>
inline std::string pathToBinaryString(const PathType& p,char sep = '.',std::size_t sepDigits=4);

/**
 * \brief From the referenced string, add each bit of binary string to Binary Path
 * \param p an instance of BinaryPath
 * \param s reference to string s
 * \param sep character separator
 */
template <typename PathType>
inline bool pathFromBinaryString(PathType& p,const std::string& s,char sep = '.');


/**
 * \brief Return bit path in hex string format.
 * \param p reference to instance of BinaryPath
 * \param sep hex digit group separator
 * \param sepDigits number of digits in each hex digit group
 * \param lensep separator between path digits and path length
 */
template <typename PathType>
inline std::string pathToHexString(const PathType& p,char digsep = ':',std::size_t sepDigits=4,char lensep = '/');

/**
 * \brief Convert hex string representation of path into binary path
 * \param p reference to instance of BinaryPath
 * \param s hex string 
 * \param digsep hex digit group separator
 * \param lensep separator between path digits and path length
 */
template <typename PathType>
inline bool pathFromHexString(PathType& p,const std::string& s,char digsep = ':',char lensep = '/');

} // namespace BinaryPathUtils

/////////////////////////////////
// IMPLEMENTATION - BinaryPath //
/////////////////////////////////

template <std::size_t MaxDepth>
BinaryPath<MaxDepth>::BinaryPath(std::initializer_list<std::size_t> steps) {
  if (steps.size() > MaxDepth) { throw std::out_of_range("initializer path exeeds maximum depth"); }
  for (std::size_t step : steps) {
    if (step >= Radix) { throw std::out_of_range("step value exceeds radix"); }
    push_back(step);
  }
}

template <std::size_t MaxDepth>
BinaryPath<MaxDepth>::BinaryPath(const uint8_t* pathBytes,std::size_t pathLen) {
  if (pathLen > MaxDepth) { throw std::out_of_range("pathLen exceeds maximum depth"); }
  length_ = pathLen;
  std::memcpy(bits_.data(),pathBytes,BytesRequired);
}


template <std::size_t MaxDepth>
void BinaryPath<MaxDepth>::clear() {
  bits_.fill(0);
  length_ = 0;
}

template <std::size_t MaxDepth>
void BinaryPath<MaxDepth>::resize(std::size_t newSize) {
  if (newSize > MaxDepth) { throw std::length_error("PathBinary::resize: attempt to resize larger than max depth"); }
  if (newSize < length_) { return trim_back(length_ - newSize); }
  // Since we keep trailing bits set to 0 we can just push out the length
  // if the new desired length is bigger than what we've got now.
  length_ = newSize;
}

template <std::size_t MaxDepth>
void BinaryPath<MaxDepth>::push_back(std::size_t c) {
  if (full()) { throw std::length_error("PathBinary::push_back: path full"); }
  // We keep unassigned bits at the end cleared, so can just do an "or"
  if (c != 0) {
    bits_[length_/8] |= (static_cast<uint8_t>(0x1) << (7 - (length_ % 8)));
  }
  ++length_;
}

template <std::size_t MaxDepth>
void BinaryPath<MaxDepth>::pop_back() {
  if (empty()) { throw std::length_error("PathBinary::pop_back: path empty"); }
  --length_;
  // Always keep unassigned bits == 0
  bits_[length_/8] &= ~(static_cast<uint8_t>(0x1) << (7 - (length_ % 8)));
}
  
template <std::size_t MaxDepth>
std::size_t BinaryPath<MaxDepth>::at(std::size_t p) const {
  if (p >= length_) { throw std::length_error("PathBinary: attempt to access out of range element"); }
  return ((bits_[p/8] & (static_cast<uint8_t>(0x1) << (7 - (p % 8)))) != 0) ? 1 : 0;
}

template <std::size_t MaxDepth>
bool BinaryPath<MaxDepth>::operator==(const BinaryPath<MaxDepth>& other) const {
  if (length_ != other.length_) { return false; }
  if (length_ == 0) { return true; }
  return (bits_ == other.bits_);
}

template <std::size_t MaxDepth>
std::size_t BinaryPath<MaxDepth>::matching(const BinaryPath<MaxDepth>& other) const
{
  // Return number of bits matching from start of path
  std::size_t maxMatchLen = std::min(size(),other.size());
  std::size_t matchLen = 0;
  uint8_t cmp = 0;
  std::size_t atByte = 0;
  while ((cmp == 0) && (matchLen < maxMatchLen)) {
    cmp = (bits_[atByte] ^ other.bits_[atByte]);
    matchLen += 8;
    uint8_t tmp = cmp;
    while (tmp > 0) {
      --matchLen;
      tmp >>= 1;
    }
    ++atByte;
  }
  return std::min(matchLen,maxMatchLen);
}

template <std::size_t MaxDepth>
void BinaryPath<MaxDepth>::trim_back(std::size_t n) {
  if (n == 0) { return; }
  if (length_ < n) {
    throw std::length_error("PathBinary::trim_back: attempt to trim more bits than available");
  }
  if (length_ == n) { return clear(); }
  std::size_t newLength = (length_ - n);
  std::size_t bytesToKeep = (newLength / 8);
  std::size_t bitsToKeep = (newLength % 8);
  if (bitsToKeep > 0) {
    // handle the end of the bytesToKeep, increment it so that we don't
    // handle these bits along with the bytes later
    bits_[bytesToKeep++] &= (static_cast<uint8_t>(0xFF) << (8 - bitsToKeep));
  }
  std::size_t lastByte = (length_ + 7)/8;
  for (std::size_t i = bytesToKeep; i < lastByte; ++i) {
    bits_[i] = 0;
  }
  length_ = newLength;
}

template <std::size_t MaxDepth>
void BinaryPath<MaxDepth>::trim_front(std::size_t n)
{
  if (n == 0) { return; }
  if (length_ < n) {
    throw std::length_error("PathBinary::trim_front: attempt to trim more bits than available");
  }

  shiftLeftBits(bits_,n);
  if (length_ == n) { return clear(); }
  trim_back(n);
}

template <std::size_t MaxDepth>
bool BinaryPath<MaxDepth>::coveredby(const BinaryPath<MaxDepth>& other) const
{
  if (length_ > other.length_) { return false; }
  return (matching(other) == length_);
}

template <std::size_t MaxDepth>
std::string BinaryPath<MaxDepth>::toBinaryString() const
{
  return BinaryPathUtils::pathToBinaryString<BinaryPath<MaxDepth>>(*this,'.',4);
}

template <std::size_t MaxDepth>
bool BinaryPath<MaxDepth>::fromBinaryString(const std::string& s)
{
  return BinaryPathUtils::pathFromBinaryString<BinaryPath<MaxDepth>>(*this,s,'.');
}

template <std::size_t MaxDepth>
std::string BinaryPath<MaxDepth>::toHexString() const {
  return BinaryPathUtils::pathToHexString<BinaryPath<MaxDepth>>(*this,':',4,'/');
}

template <std::size_t MaxDepth>
bool BinaryPath<MaxDepth>::fromHexString(const std::string& s)
{
  return BinaryPathUtils::pathFromHexString<BinaryPath<MaxDepth>>(*this,s,':','/');
}
  
template <std::size_t MaxDepth>
template <typename BitsType>
void BinaryPath<MaxDepth>::shiftLeftBits(BitsType&& bits,std::size_t bitCount) {

  if (bitCount >= 8) {
    std::size_t bytesToShift = (bitCount/8);
    if (bytesToShift > bits.size()) { throw std::length_error("PathBinary: shiftLeft too many bits"); }
    void* dest = &(bits[0]);
    void* src = &(bits[bytesToShift]);
    memmove(dest,src,(bits.size() - bytesToShift));
  }
  std::size_t bitsToShift = (bitCount % 8);
  if (bitsToShift > 0) {
    for (std::size_t i=0; i<bits.size(); ++i) {
      bits[i] = (bits[i] << bitsToShift);
      if (i < (bits.size() - 1)) {
        bits[i] |= (bits[i+1] >> (8 - bitsToShift));
      }
    }
  }
}

namespace BinaryPathUtils {
template <typename PathType>
std::string pathToBinaryString(const PathType& p,char sep,std::size_t sepDigits) {
  std::string result;
  std::size_t length = p.size();
  for (std::size_t i=0;i<length;++i) {
    result += ((p.at(i) == 0) ? '0' : '1');
    if ((i != (length - 1)) && (((i + 1) % sepDigits) == 0)) { result += sep; }
  }
  return result;
}

template <typename PathType>
bool pathFromBinaryString(PathType& p,const std::string& s,char sep) {
  // Make sure we've only got 1-0-separator, can't have more than 1 separator in a row, can't exceed max digits
  std::size_t lastsep = 0;
  std::size_t bitcount = 0;
  PathType newpath;
  if (s.empty()) { return true; }
  for (std::size_t i = 0; i < s.size(); ++i) {
    if (bitcount >= p.capacity()) { return false; }
    if (s[i] == '1') {
      newpath.push_back(1);
      ++bitcount;
    }
    else if (s[i] == '0') {
      newpath.push_back(0);
      ++bitcount;
    }
    else if (s[i] == sep) {
      if (lastsep == (i-1)) { return false; }
      lastsep = i;
    } else { return false; }
  }
  p = std::move(newpath);
  return true;
}

template <typename PathType>
std::string pathToHexString(const PathType& p,char sep,std::size_t sepDigits,char lensep) {
  std::string result;
  std::size_t length = p.size();
  const std::array<char,16> nibbleToHex{'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'};
  uint8_t curNibble(0);
  for (std::size_t i = 0; i < p.size(); ++i) {
    uint32_t bitInNibble = (i % 4);
    curNibble |= (p.at(i) << (3 - (i % 4)));
    if (bitInNibble == 3) {
      result += nibbleToHex[curNibble];
      if ((i != (length - 1)) && ((i + 1) % (4*sepDigits)) == 0) { result += sep; }
      curNibble = 0;
    }
  }
  if ((length % 4) != 0) { result += nibbleToHex[curNibble]; }
  result += lensep;
  result += std::to_string(length);
  return result;
}

template <typename PathType>
bool pathFromHexString(PathType& p,const std::string& s,char digsep,char lensep) {
  // Make sure we've only got 0-9a-fA-F:separator:lensep, can't have more than one separator or lensep, length in decimal
  PathType newpath;
  std::size_t lastsep = 0;
  if (s.empty()) { return true; }
  std::size_t bitsRecorded(0);
  for (std::size_t i = 0; i < s.size(); ++i) {
    char c = tolower(s[i]);
    uint8_t curNibble = 0xff;
    if ((c >= '0') && (c <= '9')) { curNibble = static_cast<uint8_t>(c - '0'); }
    else if ((c >= 'a') && (c <= 'f')) { curNibble = static_cast<uint8_t>(c - 'a' + 10); }
    else if (c == digsep) {
      if (lastsep == (i-1)) { return false; }
      lastsep = i;
    } else if (c == lensep) {
      if (i >= (s.size() - 1)) { return false; }
      std::string lenstr = s.substr(i+1);
      std::size_t bitlen(0);
      try {
        bitlen = stoul(lenstr);
      } catch (...) {
        return false;
      }
      if (bitlen > bitsRecorded) { return false; }
      if (bitlen < (bitsRecorded - 3)) { return false; }
      newpath.resize(bitlen);
    }
    if (curNibble <= 0x0f) {
      std::size_t maxBitsToAdd = (p.capacity() - bitsRecorded);
      if (maxBitsToAdd > 4) { maxBitsToAdd = 4; }
      for (std::size_t b = 0;b < maxBitsToAdd; ++b) {
        if (((static_cast<uint8_t>(0x8) >> b) & curNibble) == 0) { newpath.push_back(0); }
        else { newpath.push_back(1); }
      }
      bitsRecorded += maxBitsToAdd;
    }
  }
  p = std::move(newpath);
  return true;
}

} // namespace BinaryPathUtils

} // namespace RadixTree
} // namespace Mapper
} // namespace Akamai

#endif
