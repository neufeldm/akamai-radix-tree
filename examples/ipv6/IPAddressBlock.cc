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

#include <string>
#include <cstring>
#include <stdint.h>
#include <stdexcept>
#include <array>

#include "IPAddressBlock.h"
#include "IPAddressUtils.h"

namespace Akamai {
namespace Mapper {
namespace IPAddrUtils {

IPAddressBlock::IPAddressBlock(const std::string& ipstr) {
  std::array<uint8_t,16> bytesFromStr;
  uint8_t prefixLenFromStr;
  if (!stringToAddrBytes(ipstr,bytesFromStr.data(),prefixLenFromStr)) {
    throw std::runtime_error(std::string("IPAddressBlock: invalid string: '" + ipstr + "'").c_str());
  }
  path_.resize(IPAddrUtils::isV4Mapped(bytesFromStr.data()) ? prefixLenFromStr + 96 : prefixLenFromStr);
  std::memcpy(path_.rawBytes(),bytesFromStr.data(),16);
}

IPAddressBlock::IPAddressBlock(const uint8_t* b,std::size_t l,bool v4Bytes) {
  if (v4Bytes) {
    path_.resize(l + 96);
    std::memcpy(initAsV4(path_.rawBytes()),b,4);
  } else {
    path_.resize(l);
    std::memcpy(path_.rawBytes(),b,16);
  }
}

bool
IPAddressBlock::fromString(const std::string& ipstr) {
  std::array<uint8_t,16> bytesFromStr;
  uint8_t prefixLenFromStr;
  if (!stringToAddrBytes(ipstr,bytesFromStr.data(),prefixLenFromStr)) { return false; }
  if (IPAddrUtils::isV4Mapped(bytesFromStr.data())) { prefixLenFromStr += 96; };
  path_.resize(prefixLenFromStr);
  std::memcpy(path_.rawBytes(),bytesFromStr.data(),16);
  return true;
}

std::string
IPAddressBlock::toString(bool noPrefixLen,bool forceV6Str) const {
  if (noPrefixLen) { return addrToString(path_.rawBytes(),false,forceV6Str); }
  return blockToString(path_.rawBytes(),static_cast<uint8_t>(path_.size()),forceV6Str);
}

bool
IPAddressBlock::fromBytes(const uint8_t* b,std::size_t l) {
  if (l > 128) { return false; }
  path_.resize(l);
  std::memcpy(path_.rawBytes(),b,16);
  return true;
}

const uint8_t*
IPAddressBlock::addrBytes() const { return path_.rawBytes(); }

std::size_t
IPAddressBlock::prefixLength() const { return path_.size(); }

bool
IPAddressBlock::isV4Mapped() const { return IPAddrUtils::isV4Mapped(addrBytes()); }

bool
IPAddressBlock::v4FromBytes(const uint8_t* v4b,std::size_t v4l) {
  if (v4l > 32) { return false; }
  path_.resize(v4l + 96);
  std::memcpy(initAsV4(path_.rawBytes()),v4b,4);
  return true;
}

const uint8_t*
IPAddressBlock::v4AddrBytes() const { return (isV4Mapped() ? (addrBytes() + 12) : nullptr); }

std::size_t IPAddressBlock::v4PrefixLength() const {
  if (!isV4Mapped()) { throw std::runtime_error("IPAddressBlock::v4PrefixLength(): address block not V4 mapped"); }
  return (path_.size() - 96);
}

} // namespace IPAddrUtils
} // namespace Mapper
} // namespace Akamai