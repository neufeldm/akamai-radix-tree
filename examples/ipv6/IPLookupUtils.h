#ifndef AKAMAI_MAPPER_RADIXTREE_IPLOOKUP_UTILS_H_
#define AKAMAI_MAPPER_RADIXTREE_IPLOOKUP_UTILS_H_

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
#include <chrono>

#include "IPAddressBlock.h"

// Expects to see lines like this:
// <IP address block> <string value (no spaces)>
template <typename LookupTreeType>
void readIPBlockValues(std::istream& is,LookupTreeType& lt) {
  auto startTime = std::chrono::high_resolution_clock::now();
  std::string ipAddrBlockStr;
  typename LookupTreeType::Value value;
  uint64_t lineCount{0};
  while (is >> ipAddrBlockStr >> value) {
    if (ipAddrBlockStr.empty()) { return; }
    Akamai::Mapper::IPAddrUtils::IPAddressBlock ipAddrBlock(ipAddrBlockStr);
    lt.addValue(ipAddrBlock,value);
    ++lineCount;
  }
  auto finishTime = std::chrono::high_resolution_clock::now();
  std::cerr << "Spent " <<
                std::to_string(std::chrono::duration_cast<std::chrono::microseconds>(finishTime - startTime).count()) <<
                " microseconds reading/adding " << std::to_string(lineCount) << " values" << std::endl;
}

// Expects to see a series of IP address blocks to lookup, outputs the block with the value
// that mached the lookup as well as the value.
template <typename LookupTreeType>
uint64_t lookupIPBlockValues(std::istream& is,std::ostream& os,const LookupTreeType& lt) {
  uint64_t v4LookupCount{0};
  uint64_t v6LookupCount{0};
  uint64_t v4LookupNSec{0};
  uint64_t v6LookupNSec{0};
  std::string ipAddrBlockStr;
  while (std::getline(is,ipAddrBlockStr)) {
    if (ipAddrBlockStr.empty()) { break; }
    Akamai::Mapper::IPAddrUtils::IPAddressBlock ipAddrBlock(ipAddrBlockStr);
    uint64_t lookupNSec{0};
    auto startLookup = std::chrono::high_resolution_clock::now();
    auto valDepth = lt.lookupValueDepth(ipAddrBlock);
    auto finishLookup = std::chrono::high_resolution_clock::now();
    lookupNSec = (finishLookup - startLookup).count();
    if (ipAddrBlock.isV4Mapped()) {
      ipAddrBlock.path().resize(valDepth.depth + 96);
      ++v4LookupCount;
      v4LookupNSec += lookupNSec;
    } else {
      ipAddrBlock.path().resize(valDepth.depth);
      ++v6LookupCount;
      v6LookupNSec += lookupNSec;
    }
    os << ipAddrBlock.toString() << " " << valDepth.value << std::endl;
  }
  double nsPerV4Lookup = (v4LookupCount == 0) ? 0.0 : static_cast<double>(v4LookupNSec)/v4LookupCount;
  std::cerr << "Spent " << std::to_string(v4LookupNSec) << " ns on " <<
                std::to_string(v4LookupCount) << " IPv4 lookups " <<
                std::to_string(nsPerV4Lookup) << " ns per lookup" << std::endl;
  double nsPerV6Lookup = (v6LookupCount == 0) ? 0.0 : static_cast<double>(v6LookupNSec)/v6LookupCount;
  std::cerr << "Spent " << std::to_string(v6LookupNSec) << " ns on " <<
                std::to_string(v6LookupCount) << " IPv6 lookups " <<
                std::to_string(nsPerV6Lookup) << " ns per lookup" << std::endl;
  return v4LookupCount + v6LookupCount;
}

template <typename LookupTreeType>
void runLookups(const std::string& ifname,const std::string& ofname,const LookupTreeType& lt) {
  if (ifname.empty() && ofname.empty()) {
    lookupIPBlockValues(std::cin,std::cout,lt);
    return;
  }

  uint64_t lookupCount{0};
  auto startTime = std::chrono::high_resolution_clock::now();
  std::ifstream ipLookupValuesStream(ifname);
  if (ofname.empty()) {
    lookupCount = lookupIPBlockValues(ipLookupValuesStream,std::cout,lt);
  } else {
    std::ofstream ipLookupResultStream(ofname);
    lookupCount = lookupIPBlockValues(ipLookupValuesStream,ipLookupResultStream,lt);
  }
  auto finishTime = std::chrono::high_resolution_clock::now();
  std::cerr << "Spent " <<
              std::to_string(std::chrono::duration_cast<std::chrono::microseconds>(finishTime - startTime).count()) <<
              " microseconds reading/looking up/writing " << std::to_string(lookupCount) << " values" << std::endl;
}

template <typename LookupTreeType>
void readValuesAndRunLookups(std::ifstream& valuesStream,const std::string& lookupFilename,const std::string& resultsFilename)
{
  LookupTreeType lookupTree{};

  readIPBlockValues(valuesStream,lookupTree);
  runLookups(lookupFilename,resultsFilename,lookupTree);
}

#endif