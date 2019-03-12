#ifndef AKAMAI_MAPPER_IPADDRESSUTILS_IPADDRBLOCK_H_
#define AKAMAI_MAPPER_IPADDRESSUTILS_IPADDRBLOCK_H_

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
#include <stdint.h>

#include "BinaryPath.h"

namespace Akamai {
namespace Mapper {
namespace IPAddrUtils {

/**
 * \brief Simple class that combines a 128 bit binary path with IPv4/6 related utilities.
 */
class IPAddressBlock {
public:
  /**
   * \brief Create a new IP address block object containing ::/0
   */
  IPAddressBlock() = default;

  /**
   * \brief Attempt to parse ipstr as IPv4 or IPv6 address block, initialize path on success.
   * 
   * Throws an exception if string conversion fails.
   */
  IPAddressBlock(const std::string& ipstr);

  /**
   * \brief Absorb "bytes" as a raw 16-byte IPv6 or 4-byte IPv4 address.
   * 
   * If v4Bytes is true assumes that bytes is pointing to a 4-byte IPv4 address,
   * initializes a v4 mapped v6 address and copies those 4 bytes to the end 4 bytes
   * of the new 16 byte path. In the IPv4 case the prefixLength is assumed to
   * be relative to the v4 bytes (so a maximum of 32) instead of the full 128 
   * IPv6 bits.
   */
  IPAddressBlock(const uint8_t* bytes,std::size_t prefixLength,bool v4Bytes = false);
  ~IPAddressBlock() = default;
  
  /**
   * \brief Convert from string (IPv4 or IPv6), return true on success. 
   */
  bool fromString(const std::string& ipstr);

  /**
   * \brief Generate string version of contained IP address.
   * 
   * Prints prefix length at end unless noPrefixLen is true, IPv4 mapped
   * addresses as dotted quad unless forceV6Str is true.
   */
  std::string toString(bool noPrefixLen = false, bool forceV6Str = false) const;

  /**
   * \brief Absorb raw bytes as an IP address.
   * 
   * Always copies 16 bytes, be careful if you've got junk in the bits
   * past prefixLength.
   */
  bool fromBytes(const uint8_t* bytes,std::size_t prefixLength);

  /**
   * \brief Return pointer to raw address bytes (16 bytes)
   */
  const uint8_t* addrBytes() const;

  /**
   * \brief Return prefix length of path, always relative to the full 128 IPv6 bits.
   * 
   * For IPv4 mapped addresses subtract 96 to get the normal subnet length.
   */
  std::size_t prefixLength() const;

  /**
   * \brief Whether the address is V4 mapped or not.
   */
  bool isV4Mapped() const;

  /**
   * \brief Build v4 mapped v6 address from 4 bytes at v4Bytes with prefix length for 32 bit address.
   * 
   * Initializes internal path with IPv4 mapped preamble, automatically adds 96 to v4PrefixLength.
   */
  bool v4FromBytes(const uint8_t* v4Bytes,std::size_t v4prefixLength);
  
  /**
   * \brief Pointer to raw IPv4 address bytes, nullptr if not IPv4 mapped.
   */
  const uint8_t* v4AddrBytes() const;
  /**
   * \brief V4 prefix length, throws exception if not v4 mapped.
   */
  std::size_t v4PrefixLength() const;

  using RadixTreePath = RadixTree::BinaryPath<128>;
  const RadixTreePath& path() const { return path_; }
  RadixTreePath& path() { return path_; }

private:
  RadixTree::BinaryPath<128> path_{};
};



} // namespace IPAddrUtils
} // namespace Mapper
} // namespace Akamai

#endif