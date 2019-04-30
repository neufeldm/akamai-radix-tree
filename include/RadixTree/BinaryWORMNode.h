#ifndef AKAMAI_MAPPER_RADIX_TREE_BINARY_WORM_NODE_H_
#define AKAMAI_MAPPER_RADIX_TREE_BINARY_WORM_NODE_H_

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

#include <stdint.h>
#include <stdexcept>
#include <type_traits>

#include "BinaryWORMNodeHeaderBytes.h"
#include "BinaryWORMNodeHeader.h"

namespace Akamai {
namespace Mapper {
namespace RadixTree {

/**
 * \brief NodeValueRO compatible wrapper that keeps a copy of the underlying value.
 */
template <typename ValueT>
class BinaryWORMValueCopyRO
{
public:
  using ValueType = ValueT;

  BinaryWORMValueCopyRO() = default;
  BinaryWORMValueCopyRO(const ValueType& v) : atValue_(true), valueCopy_(v) {}
  BinaryWORMValueCopyRO(ValueType&& v) : atValue_(true), valueCopy_(std::move(v)) {}

  bool operator==(const BinaryWORMValueCopyRO& o) const { return ((atValue_ == o.atValue_) && (valueCopy_ == o.valueCopy_)); }
  bool operator!=(const BinaryWORMValueCopyRO& o) const { return !(*this == o); }
 
  bool atNode() const { return atValue_; }
  bool atValue() const { return atValue_; }
  const ValueType* getPtrRO() const { return (atValue_ ? &valueCopy_ : nullptr); }
  bool ptrIsCopy() const { return true; }; 

private:
  bool atValue_{false};
  ValueType valueCopy_{};
};

/**
 * \brief Specialize on void value - no actual value contained.
 * 
 * This is intended for trees that function as sets of addresses,
 * i.e. we don't care about an associated value but just presence.
 */
template <>
class BinaryWORMValueCopyRO<void>
{
public:
  using ValueType = void;

  BinaryWORMValueCopyRO() = default;
  bool operator==(const BinaryWORMValueCopyRO& o) const { return (atValue_ == o.atValue_); }
  bool operator!=(const BinaryWORMValueCopyRO& o) const { return !(*this == o); }

  bool atNode() const { return atValue_; }
  bool atValue() const { return atValue_; }
  const ValueType* getPtrRO() const { return nullptr; }
  bool ptrIsCopy() const { return true; }

private:
  bool atValue_{false};
};


/**
 * \brief Read-only wrapper for WORM node, combines header with value reading.
 * See integer read/write value implementations below for examples, but briefly
 * the ReadValueT interface needs to provide the following:
 * \verbatim
   typename ReadValueT::ValueType - type of value in memory
   std::size_t readSize(const uint8_t* valPtr) - total size in bytes of value pointed to by valPtr
   std::size_t read(const uint8_t* valBuf,ValueType* valPtr) - read ValueType into *valPtr from valBuf, return bytes read
 * \endverbatim
 */
template <std::size_t OFFSETSIZE,bool LITTLEENDIAN,typename ReadValueT>
class BinaryWORMNodeRO
  : public BinaryWORMNodeHeaderRO<OFFSETSIZE,LITTLEENDIAN>
{
public:
  using ReadValueType = ReadValueT;
  using BaseType = BinaryWORMNodeHeaderRO<OFFSETSIZE,LITTLEENDIAN>;
  using ValueType = typename ReadValueType::ValueType;
  using HeaderBytes = BinaryWORMNodeHeaderBytes<OFFSETSIZE,LITTLEENDIAN>;
  static constexpr bool VoidValue = std::is_same<ValueType,void>::value;

  BinaryWORMNodeRO() = default;
  BinaryWORMNodeRO(const ReadValueType& rv) : readValue_(rv) {}
  BinaryWORMNodeRO(const ReadValueType& rv,const uint8_t* nodePtr)
    : BaseType(nodePtr)
    , readValue_(rv)
  {}
  BinaryWORMNodeRO(const uint8_t* nodePtr)
    : BaseType(nodePtr)
  {}

  const uint8_t* getChild(std::size_t c) const {
    if (!this->hasChild(c)) { return nullptr; }
    const uint8_t* p = this->ptr();
    bool singleChild = !(HeaderBytes::hasChild(p,0) && HeaderBytes::hasChild(p,1));
    if (singleChild || (c == 0)) {
      std::size_t nodeSize = HeaderBytes::headerSize(p);
      if (HeaderBytes::hasValue(p)) { nodeSize += readValue_.readSize(p + nodeSize); }
      return (p + nodeSize);
    }
    if (c == 1) {
      return (p + HeaderBytes::getRightChildOffset(p));
    }
    throw std::runtime_error("BinaryWORMNodeRO: invalid child");
    return nullptr;
  }

  std::size_t readValue(ValueType* v) {
    if (VoidValue || !this->hasValue()) { return 0; }
    return readValue_.read(this->valuePtr(),v);
  }
  std::size_t valueSize() { return (this->hasValue() ? readValue_.readSize(this->valuePtr()) : 0); }

private:
  ReadValueType readValue_{};
};

/**
 * \brief Write wrapper for WORM node, combines header with value writing.
 * See integer read/write value implementations below for examples, but briefly
 * the WriteValueT interface needs to provide the following:
 * \verbatim
   WriteValueT interface:
   (ReadValueT iterface)
   std::size_t writeSize(const ValueType* valPtr) - size in bytes that val would consume if written
   std::size_t write(const ValueType* valPtr,uint8_t* valBuf) - writes *valPtr to valBuf, returns bytes written 
 * \endverbatim
 */
template <std::size_t OFFSETSIZE,bool LITTLEENDIAN,typename WriteValueT>
class BinaryWORMNodeWO
  : public BinaryWORMNodeHeaderRW<OFFSETSIZE,LITTLEENDIAN>
{
public:
  using WriteValueType = WriteValueT;
  using BaseType = BinaryWORMNodeHeaderRW<OFFSETSIZE,LITTLEENDIAN>;
  using ValueType = typename WriteValueT::ValueType;
  static constexpr bool VoidValue = std::is_same<ValueType,void>::value;
  BinaryWORMNodeWO() = default;
  BinaryWORMNodeWO(const WriteValueType& wv) : writeValue_(wv) {}

  void setValue(const ValueType* v) {
    if (v == nullptr) { clearValue(); return; }
    valueBytes_.resize(writeValue_.writeSize(v));
    writeValue_.write(v,valueBytes_.data());
  }
  void clearValue() { valueBytes_.clear(); this->setHasValue(false); }

  std::size_t size() const { return (this->headerSize() + valueSize()); }
  std::size_t valueSize() const {
    if (VoidValue || !this->hasValue()) { return 0; }
    if (valueBytes_.empty()) {
      throw std::runtime_error("BinaryWORMNodeWO: attempt to get size of non-void empty value");
    }
    return valueBytes_.size();
  }

  std::size_t write(uint8_t* ptr) const {
    std::size_t bytesWritten = this->writeHeader(ptr);
    if (VoidValue || !this->hasValue()) { return bytesWritten; }
    if (valueBytes_.empty()) {
      throw std::runtime_error("BinaryWORMNodeWO: attempt to write non-void empty value");
    }
    std::memcpy(ptr + bytesWritten,valueBytes_.data(),valueBytes_.size());
    bytesWritten += valueBytes_.size();
    return bytesWritten;
  }

private:
  WriteValueType writeValue_{};
  std::vector<uint8_t> valueBytes_{};
};


template <std::size_t UINTBYTECOUNT,bool LITTLEENDIAN>
struct BinaryWORMReadWriteUInt {
  using UIntByteOps = BinaryWORMNodeUIntOps<UINTBYTECOUNT,LITTLEENDIAN>;
  static constexpr bool LittleEndian = LITTLEENDIAN;
  static constexpr bool BigEndian = !LittleEndian;
  static constexpr std::size_t UIntSize = UINTBYTECOUNT;

  // typename ReadValueT::ValueType - type of value in memory
  // typename WriteValueT::ValueType - type of value in memory
  using ValueType = typename UIntByteOps::UIntType;

  //
  // Static methods for the simple integer wrapping we're doing here,
  // but feel free to keep state around if your particular
  // value type needs it.
  //

  /**
   * \brief String identifying this value type - used by our generic WORM tree creation functions.
   */
  static std::string valueTypeID() {
    static std::string vt = std::string("AKAMAI-UINT-") + 
                            (LittleEndian ? "LITTLEENDIAN-" : "BIGENDIAN-") +
                            std::to_string(UIntSize);
    return vt;
  }

  // ReadValueT interface

  // std::size_t readSize(const uint8_t* valPtr) - total size in bytes of value pointed to by valPtr
  static std::size_t readSize(const uint8_t*) { return UIntSize; }

  // std::size_t read(const uint8_t* ptr,ValueType* val) - returns ValueType object based on data in valPtr
  static std::size_t read(const uint8_t* valBuf,ValueType* valPtr) {
    *valPtr = UIntByteOps::readUInt(valBuf); 
    return UIntSize;
  }

  // WriteValueT interface
    
  // std::size_t writeSize(const ValueType* val) - size in bytes that val would consume if written
  static std::size_t writeSize(const ValueType*) { return UIntSize; } 
  
  // std::size_t write(const ValueType* valPtr,uint8_t* valBuf) - writes *valPtr to valBuf, returns bytes written
  static std::size_t write(const ValueType* valPtr,uint8_t* valBuf) {
    UIntByteOps::writeUInt(valBuf,*valPtr);
    return UIntSize;
  }
};

/////////////////////
// IMPLEMENTATIONS //
/////////////////////

} // namespace RadixTree
} // namespace Mapper
} // namespace Akamai

#endif