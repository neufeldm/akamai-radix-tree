#ifndef AKAMAI_MAPPER_RADIX_TREE_BINARY_WORM_TREE_H_
#define AKAMAI_MAPPER_RADIX_TREE_BINARY_WORM_TREE_H_

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
#include <array>

#include "BinaryWORMCursorRO.h"
#include "SimpleStack.h"

namespace Akamai {
namespace Mapper {
namespace RadixTree {

/**
 * \brief Class that manages a Binary WORM buffer, providing access to cursors.
 * 
 * This class is designed to work in conjunction with the BinaryWORMTreeBuilder class.
 * The requirements of the buffer manager in this class dovetail with the requirements
 * of the buffer manager for the tree builder, e.g. std::vector<uint8_t>.
 */
template <typename BufferT,typename PathT,typename BinaryWORMNodeHeaderT>
class BinaryWORMTree {
public:
  using PathType = PathT;
  using CursorROType = BinaryWORMCursorRO<PathT,BinaryWORMNodeHeaderT,SimpleFixedDepthStack>;
  using CursorType = CursorROType;
  using LookupCursorROType = BinaryWORMLookupCursorRO<PathT,BinaryWORMNodeHeaderT>;
  using ValueTypeRO = typename CursorROType::ValueType;
  using ValueType = ValueTypeRO;
  using Buffer = BufferT;
  
  BinaryWORMTree() = default;
  BinaryWORMTree(const Buffer& b) : buffer_(b) {}
  BinaryWORMTree(Buffer&& b) : buffer_(std::move(b)) {}
  virtual ~BinaryWORMTree() = default;

  void setBuffer(const Buffer& b) { buffer_ = b; }
  void setBuffer(Buffer&& b) { buffer_ = std::move(b); }
  const Buffer& buffer() const { return buffer_; }
  Buffer extractBuffer() { return std::move(buffer_); }

  CursorType cursor() const { return cursorRO(); }
  CursorROType cursorRO() const { return CursorROType{buffer().data()}; }
  CursorROType walkCursorRO() const { return CursorROType{buffer().data()}; }
  LookupCursorROType lookupCursorRO() const { return LookupCursorROType{buffer().data()}; }

private:
  Buffer buffer_{};
};

/**
 * \brief Convenience typedef - a std::vector<uint8_t> is directly usable as a buffer manager.
 */
template <typename PathT,typename BinaryWORMNodeHeaderT>
using BinaryWORMTreeVector = BinaryWORMTree<std::vector<uint8_t>,PathT,BinaryWORMNodeHeaderT>;

/**
 * \brief Wraps a uint8_t* pointer for use by a WORM tree, no ownership implied.
 */
class UnownedBufferRO {
public:
  const uint8_t* data() const { return buffer_; }
  std::size_t size() const { return size_; }
  UnownedBufferRO() = default;
  UnownedBufferRO(const uint8_t* b,std::size_t s) : buffer_(b), size_(s) {};
private:
  const uint8_t* buffer_{nullptr};
  std::size_t size_{0};
};

/**
 * \brief Takes an object that owns a buffer, wraps it in a shared pointer.
 */
template <typename T>
struct SharedBufferOwnerRO {
  std::shared_ptr<T> buffer{};
  SharedBufferOwnerRO() = default;
  SharedBufferOwnerRO(const std::shared_ptr<T>& b) : buffer(b) {}
  template <typename... ConstructorArgs>
  SharedBufferOwnerRO(ConstructorArgs... cArgs) : buffer(std::make_shared<T>(std::forward<ConstructorArgs>(cArgs)...)) {}
  const uint8_t* data() const { return buffer->data(); }
  std::size_t size() const { return buffer->size(); }
};

/**
 * \brief Single-owner wrapper for a malloc buffer.
 */
class MallocBufferManagerRO {
public:
  MallocBufferManagerRO() = default;
  MallocBufferManagerRO(uint8_t* b,std::size_t s) : buffer_(b), size_(s) {}
  MallocBufferManagerRO(const MallocBufferManagerRO& o) = delete;
  MallocBufferManagerRO(MallocBufferManagerRO&& o) : buffer_(o.buffer_), size_(o.size_) { free(o.buffer_); o.buffer_ = nullptr; o.size_ = 0; }
  MallocBufferManagerRO& operator=(const MallocBufferManagerRO& o) = delete;
  MallocBufferManagerRO& operator=(MallocBufferManagerRO&& o) { free(buffer_); buffer_ = nullptr; size_ = 0; std::swap(buffer_,o.buffer_); std::swap(size_,o.size_); return *this; }
  virtual ~MallocBufferManagerRO() { free(buffer_); buffer_ = nullptr; size_ = 0; }
  uint8_t* data() { return buffer_; }
  const uint8_t* data() const { return buffer_; }
  void resize(std::size_t newSize) {
    void* newBuffer{nullptr};
    if (newSize == 0) { free(buffer_); }
    else { newBuffer = realloc(buffer_,newSize); }
    if ((newBuffer == nullptr) && (newSize != 0)) {
      throw std::bad_alloc();
    } else {
      buffer_ = reinterpret_cast<uint8_t*>(newBuffer);
      size_ = newSize;
    }
  }
  std::size_t size() const { return size_; }
  void insertBuffer(uint8_t* b,std::size_t s) { free(buffer_); buffer_ = b; size_ = s; }
  uint8_t* extractBuffer() { uint8_t* b{nullptr}; std::swap(b,buffer_); return b; }

private:
  uint8_t* buffer_{nullptr};
  std::size_t size_{0};
};


/////////////////////
// IMPLEMENTATIONS //
/////////////////////

} // namespace RadixTree
} // namespace Mapper
} // namespace Akamai

#endif