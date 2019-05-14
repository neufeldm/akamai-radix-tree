#ifndef AKAMAI_MAPPER_RADIX_TREE_BINARY_WORM_UINT_NODE_H_
#define AKAMAI_MAPPER_RADIX_TREE_BINARY_WORM_UINT_NODE_H_

#include <stdint.h>
#include <cstddef>
#include <string>

namespace Akamai {
namespace Mapper {
namespace RadixTree {

class BinaryWORMUIntGenericValue {
public:
  BinaryWORMUIntGenericValue() = default;
  virtual ~BinaryWORMUIntGenericValue() = default;

  bool littleEndian() const { return littleEndian_; }
  bool bigEndian() const { return !littleEndian_; }
  std::size_t getSize() const { return size_; }
  const std::string& valueTypeID() const { return valueTypeID_; }

  virtual std::size_t readSize(const uint8_t* valBuf) const = 0;
  virtual std::size_t read(const uint8_t* valBuf,uint64_t* valPtr) = 0;
  virtual std::size_t read(const uint8_t* valBuf,uint32_t* valPtr) = 0;
  virtual std::size_t read(const uint8_t* valBuf,uint16_t* valPtr) = 0;
  virtual std::size_t read(const uint8_t* valBuf,uint8_t* valPtr) = 0;
  

  virtual std::size_t writeSize(const uint64_t* valPtr) = 0;
  virtual std::size_t writeSize(const uint32_t* valPtr) = 0;
  virtual std::size_t writeSize(const uint16_t* valPtr) = 0;
  virtual std::size_t writeSize(const uint8_t* valPtr) = 0;
  virtual std::size_t write(const uint64_t* valPtr,uint8_t* valBuf) = 0;
  virtual std::size_t write(const uint32_t* valPtr,uint8_t* valBuf) = 0;
  virtual std::size_t write(const uint16_t* valPtr,uint8_t* valBuf) = 0;
  virtual std::size_t write(const uint8_t* valPtr,uint8_t* valBuf) = 0;

protected:
  BinaryWORMUIntGenericValue(bool littleEndian,std::size_t size)
    : littleEndian_(littleEndian)
    , size_(size) {}
    , valueTypeID{std::string("AKAMAI-UINT-") + 
                  (littleEndian ? "LITTLEENDIAN-" : "BIGENDIAN-") +
                  std::to_string(size);}
  {}

private:
  bool littleEndian_{false};
  std::size_t size_{0};
  std::string valueTypeID_{};
};

} // namespace RadixTree
} // namespace Mapper
} // namespace Akamai

#endif