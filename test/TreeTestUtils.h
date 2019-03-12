#ifndef AKAMAI_MAPPER_RADIX_TREE_TREE_TEST_UTILS_H_
#define AKAMAI_MAPPER_RADIX_TREE_TREE_TEST_UTILS_H_

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
#include <string>
#include <random>
#include <algorithm>
#include <limits.h>
#include <unordered_map>

#include "RandomUtils.h"
#include "PathEdgeTestUtils.h"

/**
 * \brief A path with an associated value.
 * 
 * This is a convenience class used to track a desired key/value in test code.
 */
template <typename PATHTYPE,typename VALUETYPE>
struct TestPathValue
  : public PATHTYPE
{
  using PathType = PATHTYPE;
  using ValueType = VALUETYPE;
  static constexpr std::size_t Radix = PathType::Radix;
  static constexpr std::size_t MaxDepth = PathType::MaxDepth;

  static_assert(std::is_integral<ValueType>::value,"ValueType must be an integral");
  ValueType value{};

  TestPathValue(const PathType& ip, const ValueType& v) : PathType(ip), value(v) {}
  TestPathValue(std::initializer_list<std::size_t> steps,const ValueType& v) : PathType(steps),value(v) {}
  TestPathValue(const std::vector<std::size_t>& steps,const ValueType& v) : PathType(steps),value(v) {}
  TestPathValue() = default;
  virtual ~TestPathValue() = default;
  
  template <typename CursorType>
  inline void setCursorValue(CursorType&& c) const;
  
  template <typename CursorType>
  inline void moveCursorFromSetValue(CursorType&& c,const PathType& from) const;

  template <typename CursorType>
  inline void moveCursorToSetValue(CursorType&& c,const PathType& to) const;
};

class PathNumIter {
public:
  PathNumIter() = delete;
  inline PathNumIter(std::size_t radix,std::size_t size);
  inline PathNumIter(std::size_t radix,std::initializer_list<std::size_t> digits);
  inline PathNumIter(std::size_t radix,const std::vector<std::size_t>& digits);
  PathNumIter(std::size_t radix,std::size_t size,uint64_t num);

  void zero() { std::fill(digits_.begin(),digits_.end(),0); }
  void max() { std::fill(digits_.begin(),digits_.end(),radix_ - 1); }
  
  inline void set(const std::vector<std::size_t>& digits);
  inline void set(std::initializer_list<std::size_t> digits);
  inline void set(uint64_t val,std::size_t size);

  uint64_t number() const;
  const std::vector<std::size_t>& digits() const { return digits_; }
  
  inline bool increment();

private:
  std::size_t radix_{};
  std::vector<std::size_t> digits_{};
};

template <typename PathValueT>
class TreeSpotList {
public:
  using PathValueType = PathValueT;
  static constexpr std::size_t Radix = PathValueType::Radix;
  static constexpr std::size_t MaxDepth = PathValueType::MaxDepth;

  TreeSpotList() = default;
  inline TreeSpotList(const std::vector<PathValueType>& treeSpots);
  inline TreeSpotList(std::vector<PathValueType>&& treeSpots);


  void setPaths(const std::vector<PathValueType>& p) { treeSpots_ = p; resetSequence(); }

  // want to be able to build a spot list:
  // - all possible spots
  // - all possible spots at a particular depth
  // - set # of paths, random sequences at random depths (think a little about how to distribute...)
  //
  // want to be able to sort spot list:
  // - pre/post/in, left to right/right to left
  // - random shuffle


  template <typename CursorType>
  inline void addToTree(CursorType&& c,bool eachFromRoot = false) const;

  template <typename CursorType>
  inline std::string checkTree(CursorType&& c,bool eachFromRoot = false) const;

  template <typename NewCursorFunc>
  inline void addToTreeNewCursor(NewCursorFunc&& ncf) const;

  template <typename NewCursorFunc>
  inline std::string checkTreeNewCursor(NewCursorFunc&& ncf) const;

  void shuffle(RandomNumbers<std::size_t>& rn) { rn.shuffleContainer(treeSpotSequence_); }
  
  template <typename PathLessThan>
  inline void sort(const PathLessThan& lt);

  inline void resetSequence();

  void reverse() { std::reverse(treeSpotSequence_.begin(),treeSpotSequence_.end()); }

  const std::vector<PathValueType>& treeSpots() const { return treeSpots_; }
  const std::vector<std::size_t>& treeSpotSequence() const { return treeSpotSequence_; }

private:
  std::vector<PathValueType> treeSpots_{};
  std::vector<std::size_t> treeSpotSequence_{};
};

template <typename PathValueType>
inline std::vector<PathValueType>
allPathValuesAtLength(std::size_t l,typename PathValueType::ValueType& curValue);

template <typename PathValueType>
inline std::vector<PathValueType>
somePathValuesAtLength(RandomNumbers<uint64_t>& rn,double density,std::size_t l,typename PathValueType::ValueType& curValue);

template <typename PathValueType>
inline std::vector<PathValueType>
allPathValuesThroughLength(std::size_t l,typename PathValueType::ValueType& curValue);

template <typename PathValueType>
inline std::vector<PathValueType>
somePathValuesThroughLength(RandomNumbers<uint64_t>& rn,double density,std::size_t l,typename PathValueType::ValueType& curValue);

template <typename PathValueType>
inline std::vector<PathValueType>
randomPathValuesAtLength(RandomNumbers<uint64_t>& rn,std::size_t l,typename PathValueType::ValueType& curValue,std::size_t count);

template <typename PathValueType>
inline std::vector<PathValueType>
randomPathValuesThroughLength(RandomNumbers<uint64_t>& rn,std::size_t l,typename PathValueType::ValueType& curValue,std::size_t count);

template <typename PathValueType>
inline TreeSpotList<PathValueType> spotListFillTree(std::size_t d = PathValueType::MaxDepth);

template <typename PathValueType>
inline TreeSpotList<PathValueType>
spotListFillSomeOfTree(RandomNumbers<uint64_t>& rn,double density,std::size_t d = PathValueType::MaxDepth);

template <typename PathValueType>
inline TreeSpotList<PathValueType> spotListFillLayer(std::size_t d);

template <typename PathValueType>
inline TreeSpotList<PathValueType>
spotListFillSomeOfLayer(RandomNumbers<uint64_t>& rn,double density,std::size_t d);

template<typename PathType, typename CursorType, typename CallbackType>
inline void testPreOrderTraverse(PathType&& p, CursorType&& c, CallbackType&& cb);

/////////////////////
// IMPLEMENTATIONS //
/////////////////////

template <typename PATHTYPE,typename VALUETYPE>
template <typename CursorType>
void TestPathValue<PATHTYPE,VALUETYPE>::setCursorValue(CursorType&& c) const {
  PATHTYPE::setCursor(std::forward<CursorType>(c));
  c.addNode();
  c.nodeValue().set(value);
}

template <typename PATHTYPE,typename VALUETYPE>
template <typename CursorType>
void TestPathValue<PATHTYPE,VALUETYPE>::moveCursorFromSetValue(CursorType&& c,const PathType& from) const {
  from.moveCursorTo(std::forward<CursorType>(c),*this);
  c.addNode();
  c.nodeValue().set(value);
}

template <typename PATHTYPE,typename VALUETYPE>
template <typename CursorType>
void TestPathValue<PATHTYPE,VALUETYPE>::moveCursorToSetValue(CursorType&& c,const PathType& to) const {
  to.moveCursorFromSetValue(std::forward<CursorType>(c),*this);
}

PathNumIter::PathNumIter(std::size_t radix,std::size_t size)
  : radix_(radix)
  , digits_(size,0)
{
  if (radix < 2) { throw std::out_of_range("radix must be >= 2"); }
}

PathNumIter::PathNumIter(std::size_t radix,std::initializer_list<std::size_t> digits)
  : radix_(radix)
{
  std::vector<std::size_t> newDigits;
  if (radix_ < 2) { throw std::out_of_range("radix must be >= 2"); }
  newDigits.reserve(digits.size());
  for (std::size_t digit : digits) {
    if (digit >= radix_) { throw std::out_of_range("invalid digit for radix"); }
    newDigits.push_back(digit);
  }
  digits_ = std::move(newDigits);
}

PathNumIter::PathNumIter(std::size_t radix,const std::vector<std::size_t>& digits)
  : radix_(radix)
{
  if (radix_ < 2) { throw std::out_of_range("radix must be >= 2"); }
  for (std::size_t digit : digits) { if (digit >= radix_) { throw std::out_of_range("invalid digit for radix"); } }
  digits_ = digits;
}

PathNumIter::PathNumIter(std::size_t radix,std::size_t size,uint64_t num)
  : radix_(radix)
{
  set(num,size);
}

void
PathNumIter::set(const std::vector<std::size_t>& digits) {
  if (digits.size() > digits_.size()) { throw std::out_of_range("too many digits for path"); }
  std::vector<std::size_t> newDigits;
  newDigits.reserve(digits_.size());
  if (digits.size() < digits_.size()) {
    std::size_t padcount = (digits_.size() - digits.size());
    for (std::size_t i=0;i<padcount;++i) { newDigits.push_back(0); }
  }
  for (std::size_t digit : digits) {
    if (digit >= radix_) { throw std::out_of_range("invalid digit for radix"); }
    newDigits.push_back(digit);
  }
  digits_ = std::move(newDigits);
}

void
PathNumIter::set(std::initializer_list<std::size_t> digits) {
  if (digits.size() > digits_.size()) { throw std::out_of_range("too many digits for path"); }
  std::vector<std::size_t> newDigits;
  newDigits.reserve(digits_.size());
  if (digits.size() < digits_.size()) {
    std::size_t padcount = (digits_.size() - digits.size());
    for (std::size_t i=0;i<padcount;++i) { newDigits.push_back(0); }
  }
  for (std::size_t digit : digits) {
    if (digit >= radix_) { throw std::out_of_range("invalid digit for radix"); }
    newDigits.push_back(digit);
  }
  digits_ = std::move(newDigits);
}  

void
PathNumIter::set(uint64_t val,std::size_t size) {
  std::vector<std::size_t> newDigits;
  newDigits.reserve(size);
  std::size_t curVal = val;
  std::size_t curRemainder = 0;
  do {
    curRemainder = (curVal % radix_);
    curVal = (curVal/radix_);
    newDigits.push_back(curRemainder);
    if (newDigits.size() > size) { throw std::out_of_range("integer too large for path"); }
  } while (curVal != 0);
  while (newDigits.size() < size) { newDigits.push_back(0); }
  std::reverse(newDigits.begin(),digits_.end());
  digits_ = std::move(newDigits);
}

uint64_t
PathNumIter::number() const {
  uint64_t result = 0;
  uint64_t curMultiplier = 1;
  static constexpr uint64_t max = std::numeric_limits<uint64_t>::max();
  for (std::size_t i=1;i<digits_.size();++i) {
    if (radix_ > max/curMultiplier) { throw std::out_of_range("path value too large for uint64_t"); }
    curMultiplier = radix_*curMultiplier;
  }
  for (std::size_t digit : digits_) {
    if (digit > max/curMultiplier) { throw std::out_of_range("path value too large for uint64_t"); }
    uint64_t digitValue = digit*curMultiplier;
    uint64_t newResult = result + digitValue;
    if (newResult < result) { throw std::out_of_range("path value too large for uint64_t"); }
    result = newResult;
    curMultiplier /= radix_;
  }
  return result;
}


bool
PathNumIter::increment() {
  if (digits_.empty()) { return false; }
  bool carry(false);
  std::size_t digitOffset = (digits_.size() - 1);
  if (++(digits_[digitOffset]) >= radix_) { digits_[digitOffset] = 0; carry = true; }
  --digitOffset;
  while (carry && (digitOffset != std::numeric_limits<std::size_t>::max())) {
    carry = false;
    if (++(digits_[digitOffset]) >= radix_) { digits_[digitOffset] = 0; carry = true; }
    --digitOffset;
  }
  // We're not done (haven't rolled over) as long as the carry flag isn't set
  if (carry) { max(); }
  return !carry;
}

template <typename PathValueType>
TreeSpotList<PathValueType>::TreeSpotList(const std::vector<PathValueType>& treeSpots)
  : treeSpots_(treeSpots)
{
  treeSpotSequence_.reserve(treeSpots_.size());
  for (std::size_t i=0;i<treeSpots_.size();++i) { treeSpotSequence_.push_back(i); }
}

template <typename PathValueType>  
TreeSpotList<PathValueType>::TreeSpotList(std::vector<PathValueType>&& treeSpots)
  : treeSpots_(std::move(treeSpots))
{
  treeSpotSequence_.reserve(treeSpots_.size());
  for (std::size_t i=0;i<treeSpots_.size();++i) { treeSpotSequence_.push_back(i); }
}


template <typename PathValueType>
template <typename CursorType>
void TreeSpotList<PathValueType>::addToTree(CursorType&& c,bool eachFromRoot) const {
  static_assert(Radix == std::decay<CursorType>::type::Radix,"radix mismatch");

  if (eachFromRoot) {
    for (std::size_t curSpot : treeSpotSequence_) { treeSpots_[curSpot].setCursorValue(c); }
  } else {
    PathValueType prev{};
    for (std::size_t curSpot : treeSpotSequence_) {
      treeSpots_[curSpot].moveCursorFromSetValue(c,prev);
      prev = treeSpots_[curSpot];
    }
  }
}

template <typename PathValueType>
template <typename CursorType>
std::string
TreeSpotList<PathValueType>::checkTree(CursorType&& c,bool eachFromRoot) const {
  static_assert(Radix == std::decay<CursorType>::type::Radix,"radix mismatch");
  if (eachFromRoot) {
    for (std::size_t curSpot : treeSpotSequence_) {
      treeSpots_[curSpot].setCursor(c);
      if (!c.atValue()) { return "checkTree: missing value at " + pathToString(treeSpots_[curSpot]) + " expected '" +
                                 std::to_string(treeSpots_[curSpot].value) + "'"; }
      if (*(c.nodeValue().getPtrRO()) != treeSpots_[curSpot].value) {
        return "checkTree: incorrect value '" + std::to_string(*(c.nodeValue().getPtrRO())) + "' at " +
               pathToString(treeSpots_[curSpot]) + " expected '" + std::to_string(treeSpots_[curSpot].value) + "'";
      }
    }
  } else {
    PathValueType prev;
    for (std::size_t curSpot : treeSpotSequence_) {
      treeSpots_[curSpot].moveCursorFrom(c,prev);
      if (!c.atValue()) { return "checkTree: missing value at " + pathToString(treeSpots_[curSpot])  + " expected '" +
                          std::to_string(treeSpots_[curSpot].value) + "'"; }
      if (*(c.nodeValue().getPtrRO()) != treeSpots_[curSpot].value) {
         return "checkTree: incorrect value '" + std::to_string(*(c.nodeValue().getPtrRO())) + "' at " +
                pathToString(treeSpots_[curSpot]) + " expected '" + std::to_string(treeSpots_[curSpot].value) + "'";
      }
      prev = treeSpots_[curSpot];
    }
  }
  return "OK";
}

template <typename PathValueType>
template <typename NewCursorFunc>
void TreeSpotList<PathValueType>::addToTreeNewCursor(NewCursorFunc&& ncf) const {
  for (std::size_t curSpot : treeSpotSequence_) { treeSpots_[curSpot].setCursorValue(ncf()); }
}

template <typename PathValueType>
template <typename NewCursorFunc>
std::string TreeSpotList<PathValueType>::checkTreeNewCursor(NewCursorFunc&& ncf) const {
  for (std::size_t curSpot : treeSpotSequence_) {
    auto c = ncf();
    treeSpots_[curSpot].setCursor(c);
    if (!c.atValue()) { return "checkTreeNewCursor: missing value at " + pathToString(treeSpots_[curSpot]) +
                               " expected '" + std::to_string(treeSpots_[curSpot].value) + "'"; }
    if (*(c.nodeValue().getPtrRO()) != treeSpots_[curSpot].value) {
      return "checkTreeNewCursor: incorrect value '" + std::to_string(*(c.nodeValue().getPtrRO())) + "' at " +
             pathToString(treeSpots_[curSpot]) + " expected '" + std::to_string(treeSpots_[curSpot].value) + "'";
    }
  }
  return "OK";
}


template <typename PathValueType>
template <typename PathLessThan>
void
TreeSpotList<PathValueType>::sort(const PathLessThan& lt) {
  //typename std::add_pointer<typename std::remove_reference<PathLessThan>>::type ltPtr = &lt;
  const PathLessThan* ltPtr = &lt;
  auto sortFn = [ltPtr,this](std::size_t a,std::size_t b) -> bool { return (*ltPtr)(treeSpots_[a],treeSpots_[b]); };
  std::sort(treeSpotSequence_.begin(),treeSpotSequence_.end(),sortFn);
}

template <typename PathValueType>
void
TreeSpotList<PathValueType>::resetSequence() {
  treeSpotSequence_.clear();
  treeSpotSequence_.reserve(treeSpots_.size());
  for (std::size_t i=0;i<treeSpots_.size();++i) { treeSpotSequence_.push_back(i); }
}

template <typename PathValueType>
std::vector<PathValueType>
allPathValuesAtLength(std::size_t l,typename PathValueType::ValueType& curValue) {
  static constexpr std::size_t Radix = PathValueType::Radix;
  static constexpr std::size_t MaxDepth = PathValueType::MaxDepth;
  
  if (l > MaxDepth) { throw std::range_error("allPathValuesAtLength: length out of range"); }
  std::vector<PathValueType> pathValues;
  PathNumIter pathIter(Radix,l);
  do {
    pathValues.push_back(PathValueType(pathIter.digits(),curValue++));
  } while (pathIter.increment());
  return pathValues;
}

template <typename PathValueType>
std::vector<PathValueType>
somePathValuesAtLength(RandomNumbers<uint64_t>& rn,double density,std::size_t l,typename PathValueType::ValueType& curValue) {
  static constexpr std::size_t Radix = PathValueType::Radix;
  static constexpr std::size_t MaxDepth = PathValueType::MaxDepth;

  if (l > MaxDepth) { throw std::range_error("somePathValuesAtLength: length out of range"); }
  if ((density < 0.0) || (density > 1.0)) { throw std::range_error("somePathValuesAtLength: density should be between 0 and 1"); }
  std::vector<PathValueType> pathValues;
  PathNumIter pathIter(Radix,l);
  static constexpr uint64_t MaxUniform = 100000;
  std::uniform_int_distribution<uint64_t> uniform(0,MaxUniform);
  do {
    double chance = static_cast<double>(uniform(rn.generator()))/static_cast<double>(MaxUniform);
    if (chance <= density) {
      pathValues.push_back(PathValueType(pathIter.digits(),curValue++));
    }
  } while (pathIter.increment());
  return pathValues;
}

template <typename PathValueType>
std::vector<PathValueType>
allPathValuesThroughLength(std::size_t l,typename PathValueType::ValueType& curValue) {
  static constexpr std::size_t Radix = PathValueType::Radix;
  static constexpr std::size_t MaxDepth = PathValueType::MaxDepth;

  if (l > MaxDepth) { throw std::range_error("allPathValuesThroughLength: length out of range"); }
  std::vector<PathValueType> pathValues;
  for (std::size_t curLength=0;curLength <= l;++curLength) {
    PathNumIter pathIter(Radix,curLength);
    do {
      pathValues.push_back(PathValueType(pathIter.digits(),curValue++));
    } while (pathIter.increment());
  }
  return pathValues;
}

template <typename PathValueType>
std::vector<PathValueType>
somePathValuesThroughLength(RandomNumbers<uint64_t>& rn,double density,std::size_t l,typename PathValueType::ValueType& curValue) {
  static constexpr std::size_t Radix = PathValueType::Radix;
  static constexpr std::size_t MaxDepth = PathValueType::MaxDepth;

  if (l > MaxDepth) { throw std::range_error("somePathValuesThroughLength: length out of range"); }
  if ((density < 0.0) || (density > 1.0)) { throw std::range_error("allPathValuesThroughLength: density should be between 0 and 1"); }
  std::vector<PathValueType> pathValues;
  static constexpr uint64_t MaxUniform = 100000;
  std::uniform_int_distribution<uint64_t> uniform(0,MaxUniform);
  for (std::size_t curLength=0;curLength <= l;++curLength) {
    PathNumIter pathIter(Radix,curLength);
    do {
      double chance = static_cast<double>(uniform(rn.generator()))/static_cast<double>(MaxUniform);
      if (chance <= density) {
        pathValues.push_back(PathValueType(pathIter.digits(),curValue++));
      }
    } while (pathIter.increment());
  }
  return pathValues;
}

template <typename PathValueType>
std::vector<PathValueType>
randomPathValuesAtLength(RandomNumbers<uint64_t>& rn,std::size_t l,typename PathValueType::ValueType& curValue,std::size_t count) {
  static constexpr std::size_t Radix = PathValueType::Radix;
  static constexpr std::size_t MaxDepth = PathValueType::MaxDepth;
  using ValueType = typename PathValueType::ValueType;

  if (l > MaxDepth) { throw std::range_error("randomPathValuesAtLength: length out of range"); }
  PathNumIter pathIter(Radix,l);
  pathIter.max();
  std::uniform_int_distribution<uint64_t> uniform(0,pathIter.number());
  std::vector<PathValueType> pathValues;
  pathValues.reserve(count);
  // Since the random path numbers can repeat we should remember what values we've already used
  // so that we keep the same value at each path.
  std::unordered_map<uint64_t,ValueType> valueAtPath;
  for (std::size_t i=0;i<count;++i) {
    uint64_t pathInt = uniform(rn.generator());
    ValueType valueToUse = curValue;
    auto prevValueIter = valueAtPath.find(pathInt);
    if (prevValueIter == valueAtPath.end()) { valueAtPath[pathInt] = curValue++; }
    else { valueToUse = prevValueIter->second; }
    pathValues.push_back(PathValueType(PathNumIter(Radix,pathInt,l).digits(),valueToUse));
  }
  return pathValues;
}


template <typename PathValueType>
std::vector<PathValueType>
randomPathValuesThroughLength(RandomNumbers<uint64_t>& rn,std::size_t l,typename PathValueType::ValueType& curValue,std::size_t count) {
  static constexpr std::size_t Radix = PathValueType::Radix;
  static constexpr std::size_t MaxDepth = PathValueType::MaxDepth;
  using ValueType = typename PathValueType::ValueType;

  if (l > MaxDepth) { throw std::range_error("randomPathValuesAtLength: length out of range"); }
  PathNumIter pathIter(Radix,l);
  pathIter.max();
  std::uniform_int_distribution<uint64_t> uniformPath(0,pathIter.number());
  std::uniform_int_distribution<uint64_t> uniformDepth(0,l);
  std::vector<PathValueType> pathValues;
  pathValues.reserve(count);
  // Since the random path numbers can repeat we should remember what values we've already used
  // so that we keep the same value at each path.
  std::unordered_map<uint64_t,ValueType> valueAtPath;
  for (std::size_t i=0;i<count;++i) {
    uint64_t pathInt = uniformPath(rn.generator());
    ValueType valueToUse = curValue;
    auto prevValueIter = valueAtPath.find(pathInt);
    if (prevValueIter == valueAtPath.end()) { valueAtPath[pathInt] = curValue++; }
    else { valueToUse = prevValueIter->second; }
    std::size_t pathLength = static_cast<std::size_t>(uniformDepth(rn.generator()));
    pathValues.push_back(PathValueType(PathNumIter(Radix,pathInt,pathLength).digits(),valueToUse));
  }
  return pathValues;
}


template <typename PathValueType>
TreeSpotList<PathValueType>
spotListFillTree(std::size_t d) {
  using ValueType = typename PathValueType::ValueType;
  ValueType val = 0;
  std::vector<PathValueType> alltree =
    allPathValuesThroughLength<PathValueType>(d,val);
  return TreeSpotList<PathValueType>(std::move(alltree));
}

template <typename PathValueType>
TreeSpotList<PathValueType>
spotListFillSomeOfTree(RandomNumbers<uint64_t>& rn,double density,std::size_t d) {
  using ValueType = typename PathValueType::ValueType;
  ValueType val = 0;
  std::vector<PathValueType> someOfTree =
    somePathValuesThroughLength<PathValueType>(rn,density,d,val);
  return TreeSpotList<PathValueType>(std::move(someOfTree));
}

template <typename PathValueType>
TreeSpotList<PathValueType>
spotListFillLayer(std::size_t d) {
  using ValueType = typename PathValueType::ValueType;
  ValueType val = 0;
  std::vector<PathValueType> alltree =
    allPathValuesAtLength<PathValueType>(d,val);
  return TreeSpotList<PathValueType>(std::move(alltree));
}

template <typename PathValueType>
TreeSpotList<PathValueType> spotListFillSomeOfLayer(RandomNumbers<uint64_t>& rn,double density,std::size_t d) {
  using ValueType = typename PathValueType::ValueType;
  ValueType val = 0;
  std::vector<PathValueType> someOfTree =
    somePathValuesAtLength<PathValueType>(rn,density,d,val);
  return TreeSpotList<PathValueType>(std::move(someOfTree));
}

template<typename PathType, typename CursorType, typename CallbackType>
void testPreOrderTraverse(PathType&& p, CursorType&& c, CallbackType&& cb) {
  static constexpr size_t radix = std::remove_reference<CursorType>::type::Radix;
  if (c.atNode()) { cb(std::forward<PathType>(p),std::forward<CursorType>(c)); }
  for (size_t child = 0; child < radix; ++child) {
    if(c.canGoChildNode(child)) {
      p.push_back(child);
      c.goChild(child);
      testPreOrderTraverse(std::forward<PathType>(p), std::forward<CursorType>(c), std::forward<CallbackType>(cb));
      c.goParent();
      p.pop_back();
    }
  }
}


#endif