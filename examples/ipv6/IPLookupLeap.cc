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
#include <cstddef>
#include <stdint.h>

#include "BinaryRadixTree.h"
#include "SimpleRadixTree.h"

#include "IPAddressBlock.h"
#include "BinaryLeapLookup.h"
#include "BinaryTreeLookup.h"
#include "BinaryHopLookup.h"
#include "IPLookupUtils.h"
#include "IPAddrBlockLookup.h"

template <typename ValueT,std::size_t Depth>
using SimpleBinaryTree = Akamai::Mapper::RadixTree::BinaryRadixTree32<ValueT,Depth>;
template <typename ValueT,std::size_t Depth>
using SimpleBinaryLookup = BinaryTreeLookup<ValueT,Depth,SimpleBinaryTree>;

template <std::size_t Radix,typename Step,std::size_t Depth>
using HopTree = Akamai::Mapper::RadixTree::SimpleRadixTree<Step,Radix,Depth,Depth/2>;

template <typename ValueT,std::size_t Depth>
using BinaryLookupHop4 = BinaryHopLookup<ValueT,Depth,4,HopTree,SimpleBinaryTree>;


template <typename ValueT,std::size_t LeapBits>
using IPv4LeapLookup = BinaryLeapLookup<ValueT,32,LeapBits,SimpleBinaryLookup,SimpleBinaryLookup>;

template <typename ValueT,std::size_t LeapBits>
using IPv6LeapLookup = BinaryLeapLookup<ValueT,128,LeapBits,SimpleBinaryLookup,BinaryLookupHop4>;

template <typename ValueT,std::size_t V4LeapBits,std::size_t V6LeapBits>
using LeapIPAddrBlockLookup = IPAddrBlockLookup<IPv4LeapLookup<ValueT,V4LeapBits>,IPv6LeapLookup<ValueT,V6LeapBits>>;

int main(int argc,char** argv) {
  auto printUsage = [argv]() {
    std::cerr << argv[0] << " <ip values file> [ip lookup values file] [ip lookup result output file]" << std::endl;
  };
  if ((argc < 2) || (argc > 4)) {
     printUsage();
     return -1;
  }

  int curArg = 1;
  std::string ipValuesFilename(argv[curArg++]);
  std::string lookup((curArg < argc) ? argv[curArg++] : "");
  std::string results((curArg < argc) ? argv[curArg++] : "");

  // Get the values to put into the lookup table
  std::ifstream values(ipValuesFilename);
  readValuesAndRunLookups<LeapIPAddrBlockLookup<std::string,16,32>>(values,lookup,results);
  
  return 0;
}