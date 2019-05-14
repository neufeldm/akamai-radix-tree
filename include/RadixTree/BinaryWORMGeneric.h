#ifndef AKAMAI_MAPPER_RADIX_TREE_BINARY_WORM_GENERIC_H_
#define AKAMAI_MAPPER_RADIX_TREE_BINARY_WORM_GENERIC_H_

#include <stdint.h>
#include <cstddef>
#include <string>

namespace Akamai {
namespace Mapper {
namespace RadixTree {

class BinaryWORMValue
{
public:
  BinaryWORMValue() = default;
  virtual ~BinaryWORMValue() = default;
  virtual std::string valueTypeID() const = 0;
  virtual std::size_t readSize(const uint8_t* valBuf) const = 0;
};

template <typename T>
class BinaryWORMValueTyped
  : public virtual BinaryWORMValue
{
public:
  using ValueType = T;
  BinaryWORMValueTyped() = default;
  virtual ~BinaryWORMValueTyped() = default;
  virtual std::size_t read(const uint8_t* valBuf, ValueType* valPtr) = 0;
  virtual std::size_t writeSize(const ValueType* valPtr) = 0;
  virtual std::size_t write(const ValueType* valPtr, uint8_t* valPtr) = 0;
};

class BinaryWORMNodeHeader
{
public:
  virtual bool hasValue() const = 0;
  virtual bool hasChild(std::size_t c) const = 0;
  virtual void setHasChild(std::size_t c) = 0;
  virtual std::size_t rightChildOffset() const = 0;
  virtual void setRightChildOffset(std::size_t o) = 0;
  virtual const uint8_t* getChild(std::size_t c) const = 0;
  virtual std::size_t headerSize() const = 0;
  virtual std::size_t writeHeader(uint8_t* b) const = 0;
  virtual std::size_t edgeStepCount() const = 0;
  virtual const uint8_t* valuePtr() const = 0;

private:
  bool offsetLittleEndian_{false};
  std::size_t offsetSize_{0};
};

} // namespace RadixTree
} // namespace Mapper
} // namespace Akamai

#endif