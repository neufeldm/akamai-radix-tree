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
#include <cstddef>
#include <stdint.h>
#include <array>
#include <string>

#include "IPAddressUtils.h"

using namespace Akamai::Mapper::IPAddrUtils;

namespace {

std::string bytesToHexStr(const uint8_t* b) {
  const char hexchars[] = "0123456789abcdef";
  std::string h;
  for (std::size_t i = 0; i < 16;++i) {
    h += hexchars[b[i] >> 4];
    h += hexchars[b[i] & 0x0F];
    if ((i < 15) && (((i + 1) % 2) == 0)) { h += ':'; }
  }
  return h;
}

} // namespace

int main(int /*argc*/,char** /*argv*/) {
  std::string ipaddrstr;
  while (std::cin >>ipaddrstr) {
    if (ipaddrstr.empty()) { break; }
    std::array<uint8_t,16> ipaddrbytes{};
    uint8_t ipaddrprefixlen{0};
    if (stringToAddrBytes(ipaddrstr,ipaddrbytes.data(),ipaddrprefixlen)) {
      std::cout << ipaddrstr << " " <<
                   bytesToHexStr(ipaddrbytes.data()) << " "
                   << blockToString(ipaddrbytes.data(),ipaddrprefixlen) << std::endl;
    } else {
      std::cout << ipaddrstr << " X X" << std::endl;
    }
  }

  return 0;
}