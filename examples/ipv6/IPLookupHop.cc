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

#include "SimpleRadixTree.h"
#include "BinaryRadixTree.h"

#include "BinaryHopLookup.h"
#include "IPAddrBlockLookup.h"
#include "IPAddressBlock.h"

#include "IPLookupUtils.h"

using namespace Akamai::Mapper::IPAddrUtils;

// First some typedefs for convenience. Due to the heavy templating
// in the radix tree library these sorts of typedefs are critical for
// readability and usability.

// Our hop trees are always going to be of depth 32 for IPv4 and 128 for IPv6.
// Note that because the IPv4 space is included within IPv6 space we could just
// use a single IPv6 tree for both our IPv4 and IPv6 lookups. Such a joint lookup
// structure is conceptually clean. That said, because IPv4 mapped space is 96 bits
// deep within IPv6 space we incur extra tree traversals getting down to the IPv4
// root before we can even start the actual lookup.
template <typename ValueT,std::size_t HopBits,
          template <std::size_t,typename,std::size_t> class HopTreeT,
          template <typename,std::size_t> class StepTreeT>
using IPv6HopLookup = BinaryHopLookup<ValueT,128,HopBits,HopTreeT,StepTreeT>;

template <typename ValueT,std::size_t HopBits,
          template <std::size_t,typename,std::size_t> class HopTreeT,
          template <typename,std::size_t> class StepTreeT>
using IPv4HopLookup = BinaryHopLookup<ValueT,32,HopBits,HopTreeT,StepTreeT>;

// Our step tree is always going to be a simple node/child binary tree implementation
// using 32 bit integers for our metadata. If you only require storing integers or bools,
// then the "word" trees might be a better option.
template <typename ValueT,std::size_t Depth>
using StepTree = Akamai::Mapper::RadixTree::BinaryRadixTree32<ValueT,Depth>;

// Just going to use a simple radix tree for hopping. This implementation uses
// an array to store child pointers, so sparse high-degree nodes use more memory
// than needed. There is also a node implementation that uses a hash table to store
// children which might scale better for high-degree sparse trees.
template <std::size_t Radix,typename Step,std::size_t Depth>
using HopTree = Akamai::Mapper::RadixTree::SimpleRadixTree<Step,Radix,Depth,Depth/2>;

// Finally, some typedefs to glue everything together into functional IPv4/IPv6 hop lookups.
template <typename ValueT,std::size_t HopBits>
using IPv6Hop = IPv6HopLookup<ValueT,HopBits,HopTree,StepTree>;

template <typename ValueT,std::size_t HopBits>
using IPv4Hop = IPv4HopLookup<ValueT,HopBits,HopTree,StepTree>;

template <typename ValueT,std::size_t V4HopBits,std::size_t V6HopBits>
using HopIPAddrBlockLookup = IPAddrBlockLookup<IPv4Hop<std::string,V4HopBits>,IPv6Hop<std::string,V6HopBits>>;

// We're always using strings as lookup values - add typedefs
template <std::size_t V4HopBits,std::size_t V6HopBits>
using HopStrLookup = HopIPAddrBlockLookup<std::string,V4HopBits,V6HopBits>;


int main(int argc,const char** argv) {
  auto printUsage = [argv]() {
    std::cerr << argv[0] << " <hop tree type v4/6> <hop tree type v4/6> <ip values file> [ip lookup values file] [ip lookup result output file]" << std::endl;
    std::cerr << "Hop tree types: v4_4,v4_8,v4_16; v6_4,v6_8,v6_16" << std::endl;
  };
  if ((argc < 4) || (argc > 6)) {
     printUsage();
     return -1;
  }
  enum HopType {NONE=0,deg4,deg8,deg16};
  std::unordered_map<std::string,HopType>
    hopTypesV4{{"v4_4",HopType::deg4},
                {"v4_8",HopType::deg8},
                {"v4_16",HopType::deg16}};
  std::unordered_map<std::string,HopType>
    hopTypesV6{{"v6_4",HopType::deg4},
                {"v6_8",HopType::deg8},
                {"v6_16",HopType::deg16}};
  int curArg = 1;
  HopType v4HopType = HopType::NONE;
  HopType v6HopType = HopType::NONE;
  if (hopTypesV4[argv[curArg]] != HopType::NONE) { v4HopType = hopTypesV4[argv[curArg]]; }
  else if (hopTypesV6[argv[curArg]] != HopType::NONE) { v6HopType = hopTypesV6[argv[curArg]]; }
  ++curArg;
  if (hopTypesV4[argv[curArg]] != HopType::NONE) { v4HopType = hopTypesV4[argv[curArg]]; }
  else if (hopTypesV6[argv[curArg]] != HopType::NONE) { v6HopType = hopTypesV6[argv[curArg]]; }
  ++curArg;
  if ((v4HopType == HopType::NONE) || (v6HopType == HopType::NONE)) {
    std::cerr << "Unable to parse one or both hop tree types: '" << argv[curArg - 1] << "' '" << argv[curArg - 2] << "'" << std::endl;
    printUsage();
  }
  std::string ipValuesFilename(argv[curArg++]);
  std::string lookup((curArg < argc) ? argv[curArg++] : "");
  std::string results((curArg < argc) ? argv[curArg++] : "");

  // Get the values to put into the lookup table
  std::ifstream values(ipValuesFilename);

  // Handle any combination of hop tree configurations we might get.
  // This is ugly and tedious, but straightforward. There's likely some compile-time
  // sequence generation shenanigans we could pull here, but it doesn't
  // quite seem worth the trouble right now.
  #define MATCHHOP(V4E,V6E) if ((v4HopType == (V4E)) && (v6HopType == (V6E)))
 
  MATCHHOP(HopType::deg4,HopType::deg4) { readValuesAndRunLookups<HopStrLookup<4,4>>(values,lookup,results); }
  else MATCHHOP(HopType::deg4,HopType::deg8) { readValuesAndRunLookups<HopStrLookup<4,8>>(values,lookup,results); }
  else MATCHHOP(HopType::deg4,HopType::deg16) { readValuesAndRunLookups<HopStrLookup<4,16>>(values,lookup,results); }
  else MATCHHOP(HopType::deg8,HopType::deg4) { readValuesAndRunLookups<HopStrLookup<8,4>>(values,lookup,results); }
  else MATCHHOP(HopType::deg8,HopType::deg8) { readValuesAndRunLookups<HopStrLookup<8,8>>(values,lookup,results); }
  else MATCHHOP(HopType::deg8,HopType::deg16) { readValuesAndRunLookups<HopStrLookup<8,16>>(values,lookup,results); }
  else MATCHHOP(HopType::deg16,HopType::deg4) { readValuesAndRunLookups<HopStrLookup<16,4>>(values,lookup,results); }
  else MATCHHOP(HopType::deg16,HopType::deg8) { readValuesAndRunLookups<HopStrLookup<16,8>>(values,lookup,results); }
  else MATCHHOP(HopType::deg16,HopType::deg16) { readValuesAndRunLookups<HopStrLookup<16,16>>(values,lookup,results); }
  else {
    // unknown combo
    printUsage();
    std::cerr << "Unsupported combination of V4/V6 hop tree parameters." << std::endl;
    return -1;
  }

  #undef MATCHHOP

  return 0;
}