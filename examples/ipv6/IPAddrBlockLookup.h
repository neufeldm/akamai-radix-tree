#ifndef AKAMAI_MAPPER_IPADDRBLOCK_LOOKUP_H_
#define AKAMAI_MAPPER_IPADDRBLOCK_LOOKUP_H_

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

#include <iostream>
#include <fstream>
#include <cstddef>
#include <stdint.h>
#include <unordered_map>
#include <chrono>

#include "BinaryRadixTree.h"
#include "IPAddressBlock.h"
#include "BinaryTreeLookup.h"

// This file contains a simple shim class that takes two binary tree lookup objects,
// (see BinaryTreeLookup.h, BinaryLeapLookup.h and BinaryHopLookup.h) one for IPv4
// values and another for IPv6. It takes instances of our IP address block utility wrapper
// and dispatches lookup of those bytes to either the IPv4 or IPv6 lookup object
// as required.

template <typename IPv4Lookup,typename IPv6Lookup>
class IPAddrBlockLookup {
public:
  static_assert(std::is_same<typename IPv4Lookup::Value,typename IPv6Lookup::Value>::value,"IPv4/v6 lookup type mismatch");
  using Value = typename IPv4Lookup::Value;

  void addValue(const Akamai::Mapper::IPAddrUtils::IPAddressBlock& ipBlock,const Value& v) {
    if (ipBlock.isV4Mapped()) {
      return v4Lookup_.addValue(ipBlock.v4AddrBytes(),ipBlock.v4PrefixLength(),v);
    } else {
      return v6Lookup_.addValue(ipBlock.addrBytes(),ipBlock.prefixLength(),v);
    }
  }
  
  bool removeValue(const Akamai::Mapper::IPAddrUtils::IPAddressBlock& ipBlock) {
    if (ipBlock.isV4Mapped()) {
      return v4Lookup_.removeValue(ipBlock.v4AddrBytes(),ipBlock.v4PrefixLength());
    }
    return v6Lookup_.removeValue(ipBlock.addrBytes(),ipBlock.prefixLength());
  }

  ValueDepth<Value>
  lookupValueDepth(const Akamai::Mapper::IPAddrUtils::IPAddressBlock& ipBlock) const {
    if (ipBlock.isV4Mapped()) {
      return v4Lookup_.lookupValueDepth(ipBlock.v4AddrBytes(),ipBlock.v4PrefixLength());
    }
    return v6Lookup_.lookupValueDepth(ipBlock.addrBytes(),ipBlock.prefixLength());
  }

  // This is just like lookupValueDepth but ignores the depth - basically a convenience function
  // if you don't care about the depth.
  Value lookupValue(const Akamai::Mapper::IPAddrUtils::IPAddressBlock& ipBlock) const {
    return lookupValueDepth(ipBlock).first;
  }

private:
  IPv4Lookup v4Lookup_{};
  IPv6Lookup v6Lookup_{};
};

#endif