#ifndef AKAMAI_MAPPER_RADIX_TREE_UTILS_H_
#define AKAMAI_MAPPER_RADIX_TREE_UTILS_H_

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

#include <cstddef>
#include <stdint.h>

/**
 * \file RadixTreeUtils.h
 * Some metaprogramming utilities that periodically come in handy.
 */
namespace Akamai {
namespace Mapper {
namespace RadixTree {
namespace Utils {

/**
 * \brief Template recursion that gets number of bits required to represent a number.
 * Does a divide by 2 of number and log value + 1 for each superclass in the inheritance chain.
 * BitsRequired<number>::bitcount == number of bits required to represent number.
 */
template <std::size_t Num,std::size_t Bits=0>
struct BitsRequired : public BitsRequired<Num/2,Bits + 1> {};

/**
 * \brief Terminate the bits required computation, export the final value.
 */
template <std::size_t Bits>
struct BitsRequired<0,Bits> { static constexpr std::size_t value = Bits; };

/**
 * \brief Template recursion that gets the number of values representable by Bits.
 */
template <std::size_t Bits,std::size_t Num=1>
struct BitsValueCount : public BitsValueCount<Bits - 1,Num*2> {};

/**
 * \brief Terminate the BitsValueCount computation, export the final value.
 */
template <std::size_t Num>
struct BitsValueCount<0,Num> { static constexpr std::size_t value = Num; };

/**
 * \brief Template recursion that determines smallest uint type to hold "Bits" bits.
 * typename UIntRequired::UIntType == smallest standard unsigned integer type that will hold "Bits" bits.
 */
template <std::size_t Bits>
struct UIntRequired : public UIntRequired<Bits - 1> {
  static_assert(Bits <= 64,"No integer type >64 bits available!");
};
/**
 * \brief Our minimum size integer available is a byte.
 */
template <>
struct UIntRequired<0> { using type = uint8_t; };
/**
 * \brief 16 bits is the next size up after a byte.
 */
template <>
struct UIntRequired<9> { using type = uint16_t; };
/**
 * \brief 32 bits is the next size up after a uint16_t.
 */
template <>
struct UIntRequired<17> { using type = uint32_t; };
/**
 * \brief Finally we top out at 64 bits.
 */
template <>
struct UIntRequired<33> { using type = uint64_t; };
 /**
  * \brief We don't yet have any standard types bigger than 64 bit.
  */
template <>
struct UIntRequired<65> {};

template <std::size_t Number>
using SmallestUIntFor = typename UIntRequired<BitsRequired<Number>::value>::type;

} // namespace Utils
} // namespace RadixTree
} // namespace Mapper
} // namespace Akamai

#endif
