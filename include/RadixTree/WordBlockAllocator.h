#ifndef AKAMAI_MAPPER_RADIX_TREE_WORD_BLOCK_ALLOCATOR_H_
#define AKAMAI_MAPPER_RADIX_TREE_WORD_BLOCK_ALLOCATOR_H_

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

#include <stdexcept>
#include <vector>
#include <cstring>
#include <limits>

/**
 * \file WordBlockAllocator.h
 * Custom allocator types for binary word type nodes based on std::vector.
 */

namespace Akamai {
namespace Mapper {
namespace RadixTree {

/**
 * \brief Simple vector-based multi-word allocator.
 *
 * Does basic bounds checking but nothing to detect multiple block frees.
 */
template <typename WordType,std::size_t WordsPerChunk>
class WordBlockVectorAllocator
{
public:
  static_assert(std::is_integral<WordType>::value,"Word block must be based on an integer");
  static_assert(!std::numeric_limits<WordType>::is_signed,"Word block integer must be unsigned");

  typedef WordType RefType;
  static constexpr RefType nullRef = 0;

  WordBlockVectorAllocator(WordType chunkCount = 0) {
    if (chunkCount > 0) { words_.reserve(WordsPerChunk*chunkCount); }
  }

  RefType newRef() {
    RefType newChunk;
    if (freeChunks_.size() > 0) {
      newChunk = freeChunks_.back();
      freeChunks_.pop_back();
      // Equivalent to calling the "constructor" on the slab object -
      // we exploit the default initialization when expanding the primary
      // vector. Might want to formalize this at some point, i.e. provide
      // some user-defined constructor to execute.
      memset(getPtr(newChunk),0,sizeof(WordType)*WordsPerChunk);
    } else {
      words_.resize(words_.size() + WordsPerChunk);
      newChunk = (words_.size()/WordsPerChunk);
    }

    return newChunk;
  }

  void deleteRef(RefType ref) {
    if (ref == nullRef) { return; }
    std::size_t wordOffset = (ref-1)*WordsPerChunk;
    if (wordOffset >= words_.size()) { throw std::out_of_range("chunk reference out of range"); }
    freeChunks_.push_back(ref);
  }
  WordType* getPtr(RefType ref) const {
    if (ref == nullRef) return nullptr;
    std::size_t wordOffset = (ref-1)*WordsPerChunk;
    if (wordOffset >= words_.size()) { throw std::out_of_range("chunk reference out of range"); }
    // Mutable pointer, but doesn't alter the state of the allocator itself
    return (const_cast<WordType*>(&words_[0]) + wordOffset);
  }

  void clear() {
    words_.clear();
    freeChunks_.clear();
  }
  void reserve(WordType chunkCount) { words_.reserve(WordsPerChunk*chunkCount); }

  const std::vector<WordType>& chunkVector() const { return words_; }
  std::size_t unusedChunkCount() const { return freeChunks_.size(); }

private:
  std::vector<WordType> words_;
  std::vector<WordType> freeChunks_;
};


}
}
}

#endif
