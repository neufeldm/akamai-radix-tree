#ifndef AKAMAI_MAPPER_RADIX_TREE_TEST_RANDOM_UTILS_H_
#define AKAMAI_MAPPER_RADIX_TREE_TEST_RANDOM_UTILS_H_

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
#include <random>
#include <vector>
#include <algorithm>
#include <stdexcept>

/**
 * \brief Wrapper around some random seeds generated using /dev/random.
 * 
 * If you need a series of random seeds you can instantiate a RandomSeeds
 * object and then rotate through all available seeds by using the "next" function.
 */
class RandomSeeds {
public:
  static std::size_t size() { return sizeof(rawSeeds_); }
  static uint64_t seed(std::size_t i) {
    if (i >= size()) { throw std::range_error("RandomSeeds: exceeded maximum seed count"); }
    return static_cast<uint64_t>(rawSeeds_[i]);
  }
  static uint64_t at(std::size_t i) { return seed(i); }
  uint64_t next() { uint64_t s = seed(curSeed_); curSeed_ = ((curSeed_+1) % size()); return s; }
private:
  // Array of seed values generated using /dev/random
  static constexpr unsigned long rawSeeds_[] =
                                     {4219639790UL, 227331179UL, 3476305967UL, 
                                      3247631670UL, 1121662137UL, 2921204145UL,
                                      3392985584UL, 4287976845UL, 3719391715UL,
                                      2919539972UL};
  std::size_t curSeed_{0};
};

template <typename IntType>
class RandomNumbers {
public:
  static_assert(std::is_unsigned<IntType>::value,"RandomNumbers: only generating unsigned numbers");
  RandomNumbers() = delete;
  RandomNumbers(IntType seed) : generator_(static_cast<uint64_t>(seed)) {}

  IntType next() { return static_cast<IntType>(generator_()); }
  std::vector<IntType> next(std::size_t count) {
    std::vector<IntType> numbers;
    numbers.reserve(count);
    for (std::size_t i = 0;i < count;++i) { numbers.push_back(generator_()); }
    return numbers;
  }

  std::vector<IntType> nextSequence(std::size_t count) {
    std::vector<IntType> r;
    for (std::size_t i=0;i<count;++i) { r.push_back(static_cast<IntType>(generator_())); }
    return r;
  }

  std::vector<IntType> nextUniform(std::size_t count,IntType mn,IntType mx) {
    uint64_t min(static_cast<uint64_t>(mn)), max(static_cast<uint64_t>(mx));
    if (min >= max) { throw std::range_error("RandomNumbers::nextUniform: min >= max"); }
    std::uniform_int_distribution<uint64_t> uniform(min,max);
    std::vector<IntType> numbers;
    numbers.reserve(count);
    for (std::size_t i=0;i<count;++i) { numbers.push_back(static_cast<IntType>(uniform(generator_))); }
    return numbers;
  }

  std::mt19937_64& generator() { return generator_; }
  const std::mt19937_64& generator() const { return generator_; }

  template <typename ContainerType>
  void shuffleContainer(ContainerType&& c) { std::shuffle(c.begin(),c.end(),generator_); }

private:
  std::mt19937_64 generator_{};
};


template <typename VectorIntType>
std::vector<VectorIntType>
generateRandomSequence(VectorIntType seed,std::size_t count) {
  std::vector<VectorIntType> r;
  r.reserve(count);
  std::mt19937_64 gen{static_cast<uint64_t>(seed)};
  for (std::size_t i=0;i<count;++i) { r.push_back(static_cast<VectorIntType>(gen())); }
  return r;
}

template <typename VectorIntType>
std::vector<VectorIntType>
generateUniformRandomSequence(VectorIntType seed,std::size_t count,VectorIntType min,VectorIntType max) {
  std::vector<VectorIntType> r;
  r.reserve(count);
  std::mt19937_64 gen{static_cast<uint64_t>(seed)};
  std::uniform_int_distribution<uint64_t> dis(static_cast<uint64_t>(min),static_cast<uint64_t>(max));
  for (std::size_t i=0;i<count;++i) { r.push_back(static_cast<VectorIntType>(dis(gen))); }
  return r;
}

template <typename ContainerType>
void shuffleContainer(uint64_t seed,ContainerType&& c) {
  std::shuffle(c.begin(),c.end(),std::mt19937_64{seed});
}


#endif