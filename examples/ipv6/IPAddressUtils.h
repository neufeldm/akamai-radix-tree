#ifndef AKAMAI_MAPPER_IP_ADDRESS_UTILS_H_
#define AKAMAI_MAPPER_IP_ADDRESS_UTILS_H_

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

/**
 * \file Utilities for manipulating IPv4/6 addresses, performing string conversions.
 * 
 * This code stores both IPv4 and IPv6 addresses as 16 byte IPv6 addresses. IPv4
 * addresses are represented within the IPv4 mapped IPv6 space, ::ffff:0:0:0:0.
 */

namespace Akamai {
namespace Mapper {
namespace IPAddrUtils {

/**
 * \brief Take the 16 bytes pointed to by "bytes" as an IP address, convert to std::string.
 * 
 * Addresses are assumed to be host addresses, so a prefix length of 32 for IPv4 and 128 for IPv6.
 * IPv4 mapped addresses are printed using "dotted quad" notation by default.
 * 
 * \param bytes pointer to a 16-byte IP address
 * \param withPrefixLen whether to print the prefix length at the end
 * \param forceV6Format print IPv4 addresses in IPv6 format
 */
std::string addrToString(const uint8_t* bytes,bool withPrefixLen = true, bool forceV6Format = false);

/**
 * \brief Convert an IP address block/subnet to a string.
 * 
 * @see addrToString
 * Like addrToString except including an explicit prefix length.
 */
std::string blockToString(const uint8_t* bytes,uint8_t prefixLen,bool forceV6Format = false);

/**
 * \brief Attempt to convert a string into a 16 byte IP address.
 * 
 * First tries an IPv4 conversion, if that fails does an IPv6.
 * Writes 16 bytes to "bytes" if conversion is successful, returns false on failure.
 */
bool stringToAddrBytes(const std::string& s,uint8_t* bytes,uint8_t& prefixLen);

/**
 * \brief Attempt to convert a string into a V4 mapped IPv6 address.
 * 
 * Only attempts to parse an IPv4 dotted quad, writes 16 bytes to "bytes".
 */
bool stringToAddrBytesV4(const std::string& s,uint8_t* bytes,uint8_t& prefixLen);


/**
 * \brief Attempt to convert a string into an IPv6 address.
 * 
 * Only attempts to parse an IPv6 address string, writes 16 bytes to "bytes".
 */
bool stringToAddrBytesV6(const std::string& s,uint8_t* bytes,uint8_t& prefixLen);

/**
 * \brief Char pointer/length version of std::string& routine above.
 */
bool stringToAddrBytes(const char* s,std::size_t slen,uint8_t* bytes,uint8_t& prefixLen);

/**
 * \brief Char pointer/length version of std::string& routine above.
 */
bool stringToAddrBytesV4(const char* s,std::size_t slen,uint8_t* bytes,uint8_t& prefixLen);

/**
 * \brief Char pointer/length version of std::string& routine above.
 */
bool stringToAddrBytesV6(const char* s,std::size_t slen,uint8_t* bytes,uint8_t& prefixLen);

/**
 * \brief Write the 16-byte "::ffff:0:0:0:0" pattern to bytes, return a pointer to the V4 part.
 */
uint8_t* initAsV4(uint8_t* bytes);

/**
 * \brief Check to see if bytes contains the IPv4 mapped ::ffff/96 prefix.
 */
bool isV4Mapped(const uint8_t* bytes);

} // namespace IPAddrUtils
} // namespace Mapper
} // namespace Akamai

#endif