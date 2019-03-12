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

#include <array>
#include <string>
#include <cstring>
#include <cstdio>
#include <stdint.h>

#include "IPAddressUtils.h"

namespace Akamai {
namespace Mapper {
namespace IPAddrUtils {

namespace {

// Hard-coded ::ffff:0:0:0:0 - the base for v4 mapped v6 addresses
std::array<uint8_t,16> v4MappedBase{{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00 }};

} // namespace

bool isV4Mapped(const uint8_t* bytes) { return (std::memcmp(bytes,&(v4MappedBase[0]),12) == 0); }

uint8_t* initAsV4(uint8_t* bytes) {
  std::memcpy(bytes,&(v4MappedBase[0]),16);
  return (bytes + 12);
}

std::string addrToString(const uint8_t* bytes,bool withPrefixLen, bool forceV6Format) {
  bool v4 = isV4Mapped(bytes);
  if (v4 && !forceV6Format) {
    std::string v4str;
    v4str.reserve(32);
    v4str = std::to_string(static_cast<unsigned>(bytes[12])) + '.' +
            std::to_string(static_cast<unsigned>(bytes[13])) + '.' +
            std::to_string(static_cast<unsigned>(bytes[14])) + '.' +
            std::to_string(static_cast<unsigned>(bytes[15]));
    if (withPrefixLen) { v4str += "/32"; }
    return v4str;
  }
 
  // IPv6 takes a little more work if we want it to look nice.
  // Since we're going to be printing out in 16 bit chunks between
  // the colons we'll go ahead and pack it that way to start.
  std::array<uint16_t,8> bytesAsWords;
  for (unsigned b=0;b<16;b+=2) {
    bytesAsWords[b/2] = ((static_cast<uint16_t>(bytes[b]) << 8) | static_cast<uint16_t>(bytes[b+1]));
  }
  // Walk through the 16 bit chunks, find the longest run of 0x0000 so we can
  // put a "::" in the right place (if any).
  int curStart{-1},curEnd{-1};
  int bestStart{-1},bestEnd{-1};
  for (int zw=0;zw<8;++zw) {
    if (bytesAsWords[zw] == 0) {
      if (curStart < 0) { curStart = zw; }
      curEnd = zw;
    } else if (curStart > -1) {
      if ((curEnd != curStart) &&
          ((curEnd - curStart) > (bestEnd - bestStart)))
      {
        bestStart = curStart;
        bestEnd = curEnd;
      }
      curStart = -1;
      curEnd = -1;
    }
  }
  // Check once more for the best - we might have ended our run on a 0x0000 block
  if ((curStart > -1) && (curStart != curEnd)) {
    if ((curEnd - curStart) > (bestEnd - bestStart)) {
      bestStart = curStart;
      bestEnd = curEnd;
    }
  }
  // XXX add option to disable 0 elision?
  std::string v6str;
  v6str.reserve(64);
  for (int cw=0;cw<8;++cw) {
    if ((cw >= bestStart) && (cw <= bestEnd)) {
      // Add start/finish colons for 0 block
      if ((cw == bestStart) || (cw == bestEnd)) { v6str += ':'; }
    } else {
      char blockbuf[16];
      std::snprintf(blockbuf,sizeof(blockbuf), "%x", bytesAsWords[cw]);
      v6str += blockbuf;
      if (cw < 7) {
        if ((bestStart < 0) || (cw != (bestStart -1))) { v6str += ':'; }
      }
    }
  }
  if (withPrefixLen) { v6str += "/128"; }
  return v6str;
}

std::string blockToString(const uint8_t* bytes,uint8_t prefixLen,bool forceV6Format) {
  if (isV4Mapped(bytes) && !forceV6Format && (prefixLen >= 96)) {
    return (addrToString(bytes,false,false) + '/' + std::to_string(prefixLen - 96));
  }
  return (addrToString(bytes,false,forceV6Format) + '/' + std::to_string(prefixLen));
}

namespace {

class OctetState {
public:
  OctetState() = delete;
  OctetState(std::array<uint8_t,4>& addr) : addr_(addr) {}
  bool addDigit(uint8_t d) {
    if (curDigitCount_ > 2) { return false; }
    curOctetValue_ = 10*curOctetValue_ + d;
    if (curOctetValue_ > 255) { return false; }
    ++curDigitCount_;
    return true;
  }
  bool octetFinish() {
    // if we've finished we're OK with "adding" an empty octet as a NOP.
    if (finishedOctetCount_ >= 4) { return (curDigitCount_ == 0); }
    if ((curDigitCount_ == 0) || (curDigitCount_ > 3)) { return false; }
    if (curOctetValue_ > 255) { return false; }
    addr_[finishedOctetCount_++] = static_cast<uint8_t>(curOctetValue_);
    curOctetValue_ = 0;
    curDigitCount_ = 0;
    return true;
  }
  bool done() const { return (finishedOctetCount_ == 4); }

private:
  std::array<uint8_t,4>& addr_;
  uint16_t curOctetValue_{0};
  unsigned finishedOctetCount_{0};
  unsigned curDigitCount_{0};
};

} // namespace


bool stringToAddrBytesV4(const char* s,std::size_t strLength,uint8_t* bytes,uint8_t& prefixLen) {
  if ((strLength < (sizeof("0.0.0.0") - 1)) || (strLength >= sizeof("255.255.255.255/32"))) { return false; }
  const char digits[] = "0123456789";
  std::array<uint8_t,4> a{{0}};
  OctetState os{a};
  const char* curChar = s;
  uint16_t curPrefix{0};
  uint8_t curPrefixDigits{0};
  bool inPrefix{false};
  std::size_t dotCount = 0;
  while ((static_cast<std::size_t>(curChar - s) < strLength) && (*curChar != '\0')) {
    if (*curChar == '.') {
      if (inPrefix) { return false; }
      if (++dotCount > 3) { return false; }
      if (!os.octetFinish()) { return false; }
    } else if (*curChar == '/') {
      if (inPrefix) { return false; }
      if (!os.octetFinish()) { return false; }
      if (!os.done()) { return false; }
      inPrefix = true;
    } else {
      const char* d = static_cast<const char*>(std::memchr(digits,*curChar,sizeof(digits)));
      if (d == nullptr) { return false; }
      if (inPrefix) {
        if (++curPrefixDigits > 2) { return false; }
        curPrefix = 10*curPrefix + static_cast<uint8_t>(d - digits);
        if (curPrefix > 32) { return false; }
      } else {
        if (!os.addDigit(static_cast<uint8_t>(d - digits))) { return false; }
      }
    }
    ++curChar;
  }
  if (!os.octetFinish()) { return false; }
  if (!os.done()) { return false; }
  prefixLen = (inPrefix ? curPrefix : 32);
  std::memcpy(initAsV4(bytes),&(a[0]),4);
  return true;
}

namespace {

struct IPv6Field {
  bool addDigit(uint8_t d) {
    if (digitsAdded >= 4) { return false; }
    fieldBits = ((fieldBits << 4) | (d & 0xF));
    ++digitsAdded;
    return true;
  }
  bool addByte(uint8_t b) {
    if ((digitsAdded != 0) && (digitsAdded != 2)) { return false; }
    fieldBits = ((fieldBits << 8) | b);
    digitsAdded += 2;
    return true;
  }
  bool notStarted() const { return ((digitsAdded == 0) && !explicitEmpty); }
  uint16_t fieldBits{0};
  std::size_t digitsAdded{0};
  bool explicitEmpty{false};
};

void assembleIPv6Bytes(const std::array<IPv6Field,8>& fields,std::size_t fieldCount,uint8_t* bytes) {
  std::size_t atInputField = 0;
  std::size_t atOutputField = 0;
  while ((atInputField < fieldCount) && (atOutputField < 8)) {
    const IPv6Field& curInputField = fields[atInputField];
    if (curInputField.explicitEmpty) {
      // If we're explicitly empty then go to the next input field and
      // jump forward to fill in the 0 gap we're adding.
      const std::size_t zeroFieldCount = ((8 - fieldCount) + 1);
      std::memset(bytes + 2*atOutputField,0,2*zeroFieldCount);
      ++atInputField;      
      atOutputField += zeroFieldCount;
    } else {
      // Simple fill-in of whatever bits we've got right now
      bytes[2*atOutputField] = (curInputField.fieldBits >> 8);
      bytes[2*atOutputField + 1] = (curInputField.fieldBits & 0xFF);
      ++atInputField;
      ++atOutputField;
    }
  }
}

} // namespace

bool stringToAddrBytesV6(const char* s,std::size_t strLength,uint8_t* bytes,uint8_t& prefixLen) {
  //std::memset(bytes,0,16);
  const char decimalDigits[] = "0123456789";
  const char hexDigitUpper[] = "0123456789ABCDEF";
  const char hexDigitLower[] = "0123456789abcdef";

  if ((strLength < (sizeof("::") - 1)) || (strLength >= sizeof("0000:0000:0000:0000:0000:ffff:255.255.255.255/32"))) { return false; }

  std::array<IPv6Field,8> ipv6Fields{};
  std::size_t curIPv6Field = 0;

  std::array<uint8_t,16> ipv4Bytes{};
  bool foundV4 = false;

  uint8_t prefixValue = 128;

  const char* curChar = s;
  uint16_t curPrefix{0};
  std::size_t curPrefixDigits{0};
  bool inPrefix{false};
  const char* lastColonAt = nullptr;
  bool parsedAsDone = false;
  std::size_t emptyFieldCount = 0;

  while ((static_cast<std::size_t>(curChar - s) < strLength) && (*curChar != '\0')) {
    // Too many characters in the string - bail out
    if (parsedAsDone) { return false; }
    if (*curChar == ':') {
      if (inPrefix) { return false; }
      // If we're working on the 8th (non-existent) field
      // then we've got too many fields defined. If we wind up
      // going to the next field then being at the 7th is still
      // to far along, but we'll check that if needed.
      if (curIPv6Field > 7) { return false; }
      lastColonAt = curChar;
      if (ipv6Fields[curIPv6Field].digitsAdded == 0) {
        if (emptyFieldCount > 0) {
          // Only time we're allowed to see more than one
          // empty field separator is if we've got 2 at
          // the very beginning.
          if (curIPv6Field != 1) { return false; }
          ++emptyFieldCount;
          // We won't increment the current field or mark it as
          // empty, just get to work on the next field.
        } else {
          ++emptyFieldCount;
          ipv6Fields[curIPv6Field++].explicitEmpty = true;
        }
      } else {
        // Don't go off the end of our fields
        if (curIPv6Field >= 7) { return false; }
        ++curIPv6Field;
      }
    } else if (*curChar == '/') {
      if (inPrefix) { return false; }
      inPrefix = true;
    } else if (*curChar == '.') {
      if (inPrefix) { return false; }
      // Might be a "v4 at the end" address type.
      // We need to have seen at least one colon.
      if (lastColonAt == nullptr) { return false; }
      if (curIPv6Field > 6) { return false; }
      // Now defer to the v4 conversion routine, starting at 1 past the colon.
      const char* v4Str = (lastColonAt + 1);
      std::size_t v4StrLength = (strLength - (v4Str - s));
      if (!stringToAddrBytesV4(v4Str,v4StrLength,ipv4Bytes.data(),prefixValue)) { return false; }
      foundV4 = true;
      parsedAsDone = true;
      curChar = (s + strLength);
      // reset the current field - we've added at least one ipv4 digit to it...
      ipv6Fields[curIPv6Field] = IPv6Field{};
    } else {
      if (inPrefix) {
        const char* d = static_cast<const char*>(std::memchr(decimalDigits,*curChar,sizeof(decimalDigits)));
        if (d == nullptr) { return false; }
        if (++curPrefixDigits > 3) { return false; }
        curPrefix = 10*curPrefix + static_cast<uint8_t>(d - decimalDigits);
        if (curPrefix > 128) { return false; }
        prefixValue = curPrefix;
        if (curPrefixDigits == 3) { parsedAsDone = true; }
      } else {
        const char* d = static_cast<const char*>(std::memchr(hexDigitLower,*curChar,sizeof(hexDigitLower)));
        const char* base = hexDigitLower;
        if (d == nullptr) {
          d = static_cast<const char*>(std::memchr(hexDigitUpper,*curChar,sizeof(hexDigitUpper)));
          if (d == nullptr) { return false; }
          base = hexDigitUpper;
        }
        if (!ipv6Fields[curIPv6Field].addDigit(static_cast<uint8_t>(d - base))) { return false; }
      }
    }
    ++curChar;
  }

  // If we found IPv4 bytes then we'll place them in the lower
  // two ipv6 fields if possible, check for correct IPv4 range
  // after we've assembled the final bytes.
  if (foundV4) {
    ipv6Fields[curIPv6Field].addByte(ipv4Bytes[12]);
    ipv6Fields[curIPv6Field].addByte(ipv4Bytes[13]);
    ++curIPv6Field;
    ipv6Fields[curIPv6Field].addByte(ipv4Bytes[14]);
    ipv6Fields[curIPv6Field].addByte(ipv4Bytes[15]);
  }

  // Generally we expect to finish the string on a field
  // that has some digits added to it. The one exception to
  // this is if the previous field is an explicitly empty
  // block (i.e. a final '::') in which case we're going
  // to finish off by padding zeros.
  if (ipv6Fields[curIPv6Field].notStarted()) {
    // Completely empty - bail out.
    if (curIPv6Field == 0) { return false; }
    // OK if we haven't started as long as we've just done a '::' block.
    // We don't need to increment the current IP field count in this case.
    if (!(ipv6Fields[curIPv6Field - 1].explicitEmpty)) { return false; }
  } else {
    ++curIPv6Field;
  }

  // If we started with an empty field then we need to have seen
  // two ':' in a row - shouldn't treat a single leading ':' as a '::'.
  if (ipv6Fields[0].explicitEmpty && (emptyFieldCount != 2)) { return false; }

  // Make sure we've got enough fields to fill out the address
  if ((emptyFieldCount == 0) && (curIPv6Field < 7)) { return false; }

  assembleIPv6Bytes(ipv6Fields,curIPv6Field,bytes);
  prefixLen = prefixValue;
  if (foundV4) { return isV4Mapped(bytes); }

  return true;
}

bool stringToAddrBytes(const char* s,std::size_t strLength,uint8_t* bytes,uint8_t& prefixLen)
{
  return (stringToAddrBytesV4(s,strLength,bytes,prefixLen) ||
          stringToAddrBytesV6(s,strLength,bytes,prefixLen));
}

bool stringToAddrBytes(const std::string& s,uint8_t* bytes,uint8_t& prefixLen) {
  std::size_t l = s.length();
  const char* p = s.c_str();
  return (stringToAddrBytesV4(p,l,bytes,prefixLen) || stringToAddrBytesV6(p,l,bytes,prefixLen)); 
}

bool stringToAddrBytesV4(const std::string& s,uint8_t* bytes,uint8_t& prefixLen) {
  std::size_t l = s.length();
  const char* p = s.c_str();
  return stringToAddrBytesV4(p,l,bytes,prefixLen); 
}

bool stringToAddrBytesV6(const std::string& s,uint8_t* bytes,uint8_t& prefixLen) {
  std::size_t l = s.length();
  const char* p = s.c_str();
  return stringToAddrBytesV6(p,l,bytes,prefixLen); 
}



} // namespace IPAddr
} // namespace Mapper
} // namespace Akarnai