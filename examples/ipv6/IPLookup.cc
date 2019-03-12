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
#include "IPAddrBlockLookup.h"
#include "IPLookupUtils.h"

// Simple application that reads in IP block/value pairs from a file, then
// performs lookups on those block/value pairs based on input from either
// stdin or a file and sends the lookup results to either stdout
// or a file.

using namespace Akamai::Mapper::IPAddrUtils;

// First some typedefs. The heavy templating in the radix tree library
// means that typedefs are pretty essential for readability.
template <typename ValueT,std::size_t D>
using IPv4Tree = Akamai::Mapper::RadixTree::BinaryRadixTree32<ValueT,D>;

template <typename ValueT,std::size_t D>
using IPv6Tree = Akamai::Mapper::RadixTree::BinaryRadixTree32<ValueT,D>;

template <typename ValueT,template <typename,std::size_t> class IPv4TreeT,template <typename,std::size_t> class IPv6TreeT>
using IPLookup = IPAddrBlockLookup<BinaryTreeLookup<ValueT,32,IPv4Tree>,BinaryTreeLookup<ValueT,128,IPv6Tree>>;

int main(int argc,const char** argv) {
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
  readValuesAndRunLookups<IPLookup<std::string,IPv4Tree,IPv6Tree>>(values,lookup,results);
  
  return 0;
}