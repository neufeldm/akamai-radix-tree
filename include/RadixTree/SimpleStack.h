#ifndef AKAMAI_MAPPER_RADIX_TREE_SIMPLE_STACK_H_
#define AKAMAI_MAPPER_RADIX_TREE_SIMPLE_STACK_H_

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
#include <memory>

namespace Akamai {
namespace Mapper {
namespace RadixTree {

/**
 * \brief A simple fixed-depth stack.
 *  
 * Provides a vector-like interface so you can easily swap the two out depending on what you'd prefer,
 * e.g. a larger fixed object without dynamic memory allocs (this one)
 * or vice versa (std::vector).
 */
template <typename T,std::size_t MaxDepth>
class SimpleFixedDepthStack
{
public:
  typedef T value_type;
  
  SimpleFixedDepthStack() = default;

  void push_back(const T& x) {
    if (depth_ == MaxDepth) { throw std::length_error("MaxDepthStack::push_back(): overflow"); }
    elements_[depth_++] = x;
  }
  void pop_back() {
    if (depth_ == 0) { throw std::length_error("MaxDepthStack::pop_back(): underflow"); }
    elements_[--depth_] = T();
  }
  T& back() {
    if (depth_ == 0) { throw std::length_error("MaxDepthStack::back(): empty stack"); }
    return elements_[depth_ - 1];
  }
  const T& back() const {
    if (depth_ == 0) { throw std::length_error("MaxDepthStack::back() const: empty stack"); }
    return elements_[depth_ - 1];
  }
  
  T& front() {
    if (depth_ == 0) { throw std::length_error("MaxDepthStack::front(): empty stack"); }
    return elements_[0];
  }
  const T& front() const {
    if (depth_ == 0) { throw std::length_error("MaxDepthStack::front() const: empty stack"); }
    return elements_[0];
  }

  void resize(std::size_t newsize) {
    if (newsize > depth_) { throw std::length_error("MaxDepthStack::resize(): attempt to resize > current depth"); }
    while (depth_ > newsize) { elements_[--depth_] = T(); }
  }

  std::size_t size() const { return depth_; }

  bool empty() const { return (depth_ == 0); }

  T& at(std::size_t i) {
    if (i >= depth_) { throw std::length_error("MaxDepthStack::at(): nonexistent element"); }
    return elements_[i];
  }

  const T& at(std::size_t i) const {
    if (i >= depth_) { throw std::length_error("const MaxDepthStack::at(): nonexistent element"); }
    return elements_[i];
  }

  T& operator[](std::size_t i) { return elements_[i]; }

  const T& operator[](std::size_t i) const { return elements_[i]; }


private:
  std::array<T,MaxDepth> elements_{};
  std::size_t depth_{0};
};

}
}
}

#endif
